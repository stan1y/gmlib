#include "manager.h"
#include "dlg.h"
#include "util.h"

#include "box.h"
#include "label.h"
#include "button.h"
#include "text.h"
#include "combo.h"

/** User Idle Counter **/

static uint32_t g_usr_idle_cnt = 0;
uint32_t UI_GetUserIdle()
{
  return g_usr_idle_cnt;
}

namespace ui {


/** Fonts cache */

typedef std::map<std::string, ttf_font*> fonts_cache;
static fonts_cache g_fonts_cache;

ttf_font* manager::load_font(const std::string & font_file, const size_t & ptsize)
{
  std::string font_id = (std::stringstream() << font_file << ":" << ptsize).str();
  fonts_cache::iterator i = g_fonts_cache.find(font_id);
  if (i == g_fonts_cache.end()) {
    ttf_font * font = new ttf_font(font_file, ptsize);
    g_fonts_cache.insert(std::make_pair(font_id, font));
    return font;
  }
  return i->second;
}

static manager* g_manager = NULL;
typedef container<control*> dead_list;
static dead_list g_graveyard;

static message * g_message = NULL;
static std::mutex g_message_mx;

/** Manager **/
void manager::initialize(rect & available_rect, const std::string & theme_file)
{
  if (g_manager == NULL) {
    g_manager = new manager(available_rect, theme_file);
  }
  else {
    g_manager->set_pos(available_rect);
  }
}

manager * manager::instance()
{
  if (g_manager == NULL) {
    SDL_Log("manager::instance() - not initialized");
    throw std::runtime_error("manager is not initialized");
  }
  return g_manager;
}

const SDL_Event* manager::current_event()
{
  if (g_manager) return g_manager->_cur_event;
  return NULL;
}

manager::manager(rect & available_rect, 
                 const std::string & theme_file):
  control(),
  screen::component(NULL),
  _hovered_cnt(NULL),
  _focused_cnt(NULL),
  _cur_event(NULL)
{
  set_pos(available_rect);
  // read theme settings
  std::ifstream(media_path(theme_file)) >> _theme_data;
  _theme_sprites.load(_theme_data["res"]);
}

void manager::destroy(control* child)
{
  mutex_lock guard(g_graveyard.mutex());
  g_graveyard.push_back(child);
  child->set_visible(false);
  child->set_destroyed(true);
}

std::string manager::tostr() const
{
  std::stringstream ss;
  ss << "{manager " \
     << " rect: " << _pos.tostr() \
     << " children: " << _children.size()
     << "}";
  return ss.str();
}

json manager::get_theme_prop(const std::string & type_name, const std::string & prop_name)
{
  if (_theme_data.find(type_name) != _theme_data.end() &&
      _theme_data.at(type_name).find(prop_name) != _theme_data.at(type_name).end())
  {
    return _theme_data.at(type_name).at(prop_name);
  }
  else if (_theme_data.find(prop_name) != _theme_data.end())
  {
    return _theme_data.at(prop_name);
  }
  else
  {
    SDL_Log("ui::manager::get_theme_prop - failed to find any property '%s'",
            prop_name.c_str());
    throw std::runtime_error("failed to find theme property");
  }
}

color manager::get_back_color(const std::string & type_name)
{
  return color::from_json(get_theme_prop(type_name, "back_color"));
}

color manager::get_highlight_color(const std::string & type_name)
{
  return color::from_json(get_theme_prop(type_name, "highlight_color"));
}

color manager::get_idle_color(const std::string & type_name)
{
  return color::from_json(get_theme_prop(type_name, "idle_color"));
}

const ttf_font * manager::get_font(const std::string & type_name)
{
  json font = get_theme_prop(type_name, "font");
  if (font.is_array()) {
    return load_font(font.at(0), font.at(1));
  }
  SDL_Log("ui::manager::get_font - failed to load font for %s",
          type_name.c_str());
  throw std::runtime_error("failed to load font");
}

void manager::set_focused_control(control * target)
{
  if (target == _focused_cnt)
    return;
  if (target == NULL)
    target = this;
  if (_focused_cnt != NULL) {
    _focused_cnt->focus_lost(_focused_cnt);
  }
  _focused_cnt = target;
  if (_focused_cnt != NULL) {
    _focused_cnt->focused(_focused_cnt);
  }
}

void manager::set_hovered_control(control * target)
{
  if (_hovered_cnt == target)
    return;

  control * prev = _hovered_cnt;
  _hovered_cnt = target;
  if (prev != NULL) {
    prev->hover_lost(prev);
  }
  // setup new
  if (_hovered_cnt != NULL) {
    _hovered_cnt->hovered(_hovered_cnt);
  }
}

/* UI Manager Component interface */

void manager::render(SDL_Renderer* r)
{
  // call UI protocol's render
  control::draw(r, get_absolute_pos());
}

void manager::on_update(screen *)
{
  // count idle miliseconds
  g_usr_idle_cnt += GM_GetFrameTicks();

  SDL_GetMouseState(&_pointer.x, &_pointer.y);

  // process graveyard
  {
    mutex_lock guard(g_graveyard.mutex());
    dead_list::iterator it = g_graveyard.begin();
    for(; it != g_graveyard.end(); ++it) {
      control* child = *it;
      if (child->parent() == nullptr) {
        SDL_Log("manager::update - g_graveyard has zombie %s", child->tostr().c_str());
        throw std::runtime_error("zombie control found in graveyard");
      }
      else {
        // unlink child from parent's tree
        child->parent()->remove_child(child);
        child->set_parent(nullptr);
      }
      SDL_Log("destroying %s", child->tostr().c_str());
      delete child;
    }
    g_graveyard.clear();
  }

  // call UI protocol's update
  control::update();
}

void manager::on_event(SDL_Event* ev)
{
  mutex_lock lock(_cur_event_mx);
  _cur_event = ev;

  // reset user idle timer
  if (ev->type == SDL_MOUSEMOTION || ev->type == SDL_MOUSEWHEEL ||
      ev->type == SDL_MOUSEBUTTONDOWN || ev->type == SDL_MOUSEBUTTONUP ||
      ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP)
  {
    g_usr_idle_cnt = 0;
  }

  // check event has a target or point at manager
  // check target is acceptable
  control * target = g_manager;
  if (_hovered_cnt != NULL && _hovered_cnt->visible() && !_hovered_cnt->proxy()) 
    target = _hovered_cnt;

  // find and call a handler for event
  switch(ev->type)
  {  
  case SDL_MOUSEMOTION:
    {
      // update hovered control if pointer was moved before callback
      // reset found to NULL if actual control is hidden or proxy
      control * found = find_child_at(ev->motion.x, ev->motion.y);
      if (found != NULL && !found->visible()) {
        found = NULL;
      }
      set_hovered_control(found);
    }
    target->mouse_move(target);
    break;
  case SDL_MOUSEBUTTONUP:
    {
      // update focused control before callbacks
      set_focused_control(_hovered_cnt);
    }
    target->mouse_up(target);
    break;
  case SDL_MOUSEBUTTONDOWN:
    target->mouse_down(target);
    break;
  case SDL_KEYUP:
    if (_focused_cnt) _focused_cnt->kbd_up(_focused_cnt);
    break;
  case SDL_KEYDOWN:
    if (_focused_cnt) _focused_cnt->kbd_down(_focused_cnt);
    break;
  case SDL_MOUSEWHEEL:
    target->mouse_wheel(target);
    break;
  };

  _cur_event = NULL;
  _cur_event_mx.unlock();

  // update hovered again becase callback might
  // had changed state of other controls (hidden/deleted)
  {
    point pointer = manager::instance()->get_pointer();
    control * found = find_child_at(pointer);
    // reset found to NULL if actual control is hidden or proxy
    if (found != NULL && !found->visible() ) found = NULL;
    set_hovered_control(found);
  }
}

/*
 * Construct controls from JSON description
 */

control* create_control(const std::string type_id, rect & pos)
{
  if (type_id == "box")
  {
    return new box(pos);
  }
  if (type_id == "hbox")
  {
    return new box(pos, box::hbox);
  }
  if (type_id == "vbox")
  {
    return new box(pos, box::vbox);
  }
  if (type_id == "panel" || type_id == "pnl")
  {
    return new panel(pos);
  }
  if (type_id.find("label") != std::string::npos)
  {
    // styled label
    label* lbl = new label(pos);
    lbl->set_style(type_id);
    return lbl;
  }
  if (type_id.find("btn") != std::string::npos)
  {
    // styled button
    button* btn = new button(pos);
    btn->set_style(type_id);
    return btn;
  }
  if (type_id.find("input") != std::string::npos)
  {
    // styled input
    text_input* inp = new text_input(pos);
    inp->set_style(type_id);
    return inp;
  }
  if (type_id.find("combo") != std::string::npos)
  {
    // styled combo
    combo *cmb = new combo(pos);
    cmb->set_style(type_id);
    return cmb;
  }
  return NULL;
}

control * control::find_child(const std::string & id)
{
  // check self
  if (_id == id) return this;
  // check children
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control * child = *it;
    if (child->identifier() == id)
      return child;
    child = child->find_child(id);
    if (child)
      return child;
  }
  
  return NULL;
}

void manager::pop_front(control * c)
{
  lock_container(_children);
  size_t idx = find_child_index(c);
  size_t last = _children.size() - 1;
  control * tmp = _children[last];
  _children[last] = c;
  _children[idx] = tmp;
}

void manager::push_back(control * c)
{
  lock_container(_children);
  size_t idx = find_child_index(c);
  control * tmp = _children[0];
  _children[0] = c;
  _children[idx] = tmp;
}

control* manager::build(const json & d)
{
  rect pos(0, 0, 0, 0);

  std::string class_id = d["class"];
  if (d.find("rect") != d.end()) {
    pos = rect(
          d["rect"].at(0),
          d["rect"].at(1),
          d["rect"].at(2),
          d["rect"].at(3));
  }
  else {
    if (d.find("size") != d.end())
      pos = rect(
            0,
            0,
            d["size"].at(0),
            d["size"].at(1));

    if (d.find("position") != d.end()) {
      if (d["position"].is_array())
        pos += point(d["position"].at(0),
            d["position"].at(1));
      if (d["position"].is_string()) {
        std::string spos = d["position"];
        if (spos == "center") {
          pos.x = (manager::instance()->pos().w - pos.w) / 2;
          pos.y = (manager::instance()->pos().h - pos.h) / 2;
        }
      }

    }
  }

  control * inst = create_control(class_id, pos);
  if (inst == NULL) {
    SDL_Log("manager::build() - failed to create an instance of [%s]", class_id.c_str());
    throw std::runtime_error("failed to create an instance of the class");
  }

  // let instance load it's props from data
  inst->load(d);

  // process children
  if (d.find("children") != d.end()) {
    json::const_iterator child = d["children"].begin();
    for(; child != d["children"].end(); ++child) {
      inst->add_child(build(*child));
    }
  }

  return inst;
}

/**
  UI Message implementation
  */
message::message(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms):
  control(f->get_text_rect(text)),
  _timeout_ms(timeout_ms)
{
  reset(text, f, c, timeout_ms);
}

void message::reset(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms)
{
  _timer.stop();
  set_visible(false);
  _text = text;
  SDL_Surface *s = f->print_solid(_text, c);
  _tx.set_surface(s);
  SDL_FreeSurface(s);
  rect display = GM_GetDisplayRect();

  _pos.w = _tx.width();
  _pos.h = _tx.height();
  // middle of the screen with Y offset
  _pos.x = (display.w - _pos.w) / 2;
  _pos.y = 25 + _pos.h;
  show();
}

void message::update()
{
  if (!_timer.is_started())
    return;

  g_message_mx.lock();
  uint32_t ticked = _timer.get_ticks();
  uint32_t step = 255 / _timeout_ms;
  //deplete 1 alpha each step
  uint32_t depleted = ticked / step;
  if (depleted >= 255) {
    _timer.stop();

    //self-destruct when alpha depleted
    ui::destroy(this);
    g_message = NULL;
  }
  
  uint8_t a = 255 - uint32_to_uint8(depleted);
  _tx.set_alpha(a);
  g_message_mx.unlock();
}

void message::show()
{
  set_visible(true);
  _timer.start();
}

void message::alert(const std::string & text, uint32_t timeout_ms)
{
  alert_ex(text, 
    ui::manager::instance()->get_font("alert"),
    ui::manager::instance()->get_highlight_color("alert"),
    timeout_ms);
}

void message::alert_ex(const std::string & text, const ttf_font * f, const color & c, uint32_t timeout_ms)
{
  g_message_mx.lock();
  if (g_message == NULL)
    g_message = new message(text, f, c, timeout_ms);
  else
    g_message->reset(text, f, c, timeout_ms);
  g_message_mx.unlock();
}

message::~message()
{
  
}

void message::render(SDL_Renderer * r, const rect & dst)
{
  g_message_mx.lock();
  _tx.render(r, dst);
  g_message_mx.unlock();
}

} //namespace ui
