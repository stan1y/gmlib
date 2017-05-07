#include "manager.h"
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

static manager* g_manager = NULL;
typedef container<control*> dead_list;
static dead_list g_graveyard;

static message * g_message = NULL;
static std::mutex g_message_mx;

/** Manager **/
void manager::initialize(rect & available_rect,
 									 const std::string & theme_tileset,
 									 const std::string & theme_font,
	                 const int theme_font_size,
 									 const color color_idle,
 									 const color color_highlight,
 									 const color color_back)
{
  if (g_manager == NULL) {
    g_manager = new manager(available_rect,
			theme_tileset, theme_font, theme_font_size, 
			color_idle, color_highlight, color_back);
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
                 const std::string & theme_file,
								 const std::string & font_file,
								 const int font_size,
	               const color & color_idle, 
	               const color & color_highlight,
								 const color & color_back):
  control(),
  screen::component(NULL),
  _hovered_cnt(NULL),
  _focused_cnt(NULL),
  _cur_event(NULL),
  _theme(theme_file),
  _font(font_file, font_size),
	_theme_color_idle(color_idle),
	_theme_color_highlight(color_highlight),
	_theme_color_back(color_back)
{
  set_pos(available_rect);
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
  if (type_id == "panel") {
    return new panel(pos);
  }
  if (type_id == "box") {
    return new box(pos);
  }
  if (type_id == "label") {
    return new label(pos);
  }
  if (type_id == "button") {
    return new button(pos);
  }
  if (type_id == "input") {
    return new text_input(pos);
  }
  if (type_id == "combo") {
    return new combo(pos);
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
message::message(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms):
  control(f.get_text_rect(text)),
  _timeout_ms(timeout_ms)
{
  reset(text, f, c, timeout_ms);
}

void message::reset(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms)
{
  _timer.stop();
  set_visible(false);
  _text = text;
  SDL_Surface *s = f.print_solid(_text, c);
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
    ui::manager::instance()->get_font(), 
    ui::manager::instance()->get_color_highlight(),
    timeout_ms);
}

void message::alert_ex(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms)
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
