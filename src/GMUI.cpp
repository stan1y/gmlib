#include "GMUI.h"
#include "GMUtil.h"
#include "GMData.h"
#include "GMUIBox.h"
#include "GMUILabel.h"
#include "GMUIButton.h"
#include "GMUITextInput.h"
#include "GMUICombo.h"
#include "GMUIPushButton.h"

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

void manager::initialize(rect & available_rect, const std::string & theme_name)
{
  if (g_manager == NULL) {
    g_manager = new manager(available_rect, theme_name);
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

manager::manager(rect & available_rect, const std::string & theme_name):
  control(),
  screen::component(NULL),
  _hovered_cnt(NULL),
  _focused_cnt(NULL),
  _cur_event(NULL),
  _theme(theme_name)
{
  set_pos(available_rect);
}

void manager::destroy(control* child)
{
  child->set_visible(false);
  child->set_destroyed(true);

  {
    mutex_lock guard(g_graveyard.mutex());
    g_graveyard.push_back(child);
  }
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
#ifdef UI_DEBUG_HOVER
    SDL_Log("manager::set_focused_control - focus lost id: %s", _focused_cnt->identifier().c_str());
#endif
    _focused_cnt->focus_lost(_focused_cnt);
  }
  _focused_cnt = target;
  if (_focused_cnt != NULL) {
#ifdef UI_DEBUG_HOVER
    SDL_Log("manager::set_focused_control - focus gain id: %s", _focused_cnt->identifier().c_str());
#endif
    _focused_cnt->focused(_focused_cnt);
  }
}

void manager::set_hovered_control(control * target)
{
  if (_hovered_cnt == target)
    return;
  // notify prev
  control * prev = _hovered_cnt;
  _hovered_cnt = target;
  if (prev != NULL) {
#ifdef UI_DEBUG_HOVER
      SDL_Log("manager::set_hovered_control - hover lost id: %s, %s left %s", 
      prev->identifier().c_str(),
      _pointer.tostr().c_str(),
      prev->get_absolute_pos().tostr().c_str());
#endif
    prev->hover_lost(prev);
  }
  // setup new
  if (_hovered_cnt != NULL) {
#ifdef UI_DEBUG_HOVER
    SDL_Log("manager::set_hovered_control - hover gain id: %s at %s", 
      _hovered_cnt->identifier().c_str(),
      _hovered_cnt->get_absolute_pos().tostr().c_str()
    );
#endif
    _hovered_cnt->hovered(_hovered_cnt);
  }
}

/* UI Manager Component interface */

void manager::render(SDL_Renderer* r)
{
  // call UI protocol's render
  control::draw(r, get_absolute_pos());

  // render pointer of theme supports
  if (_theme.ptr.tx_normal.is_valid()) {
    point pnt;
    SDL_GetMouseState(&pnt.x, &pnt.y);
    _theme.draw_pointer(r, rect(0, 0, 16, 16) + pnt);
  }
}

void manager::update()
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
  if (type_id == "btn") {
    return new btn(pos);
  }
  if (type_id == "sbtn") {
    return new sbtn(pos);
  }
  if (type_id == "lbtn") {
    return new lbtn(pos);
  }
  /*if (type_id == "push_btn") {
    return new push_btn(pos, );
  }*/
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

control* manager::build(data const * d)
{
  if (!d->is_object() || !d->has_key("class")) {
    SDL_Log("manager::build - given data is not a json object");
    throw std::runtime_error("given data is not a json object");
  }

  std::string class_id = d->get<std::string>("class");

  rect pos;
  // check "rect" propecty
  if (d->has_key("rect")) {
    pos = d->get<rect>("rect");
  }
  else {
    // check "size" property
    if (d->has_key("size")) {
      std::pair<int, int> size = d->get< std::pair<int, int> >("size");
      pos.w = size.first;
      pos.h = size.second;
    }
    else {
      pos = rect(0, 0, 0, 0);
    }

    // check position property
    if (d->has_key("position")) {
      const data::json * position = d->get("position");
      if ( json_is_string(position) ) {
        std::string p = position->as<std::string>();
        if (p == "center") {
          pos.x = (manager::instance()->pos().w - pos.w) / 2;
          pos.y = (manager::instance()->pos().h - pos.h) / 2;
        }
      }
      if ( json_is_array(position) ) {
        pos += position->as<point>();
      }
    }
  }

  // create instance of the class
  control * inst = create_control(class_id, pos);
  if (inst == NULL) {
    SDL_Log("manager::build() - failed to create an instance of [%s]", class_id.c_str());
    throw std::runtime_error("failed to create an instance of the class");
  }

  // let instance load it's props from data
  const data & obj_data = *d;
  inst->load(obj_data);

  // process children
  if (d->has_key("children") && json_is_array(d->get("children"))) {
    data children(d->get("children"));
    data::array_iterator child_it = children.array_begin();
    for(; child_it != children.array_end(); ++child_it) {
      data child_data(*child_it);
      inst->add_child(build(&child_data));
    }
  }

  return inst;
}

void manager::set_pointer(theme::pointer::pointer_type t)
{
  _theme.ptr_type = t;
}

theme::pointer::pointer_type manager::get_pointer_type()
{
  return _theme.ptr_type;
}

/**
  UI Message implementation
  */
message::message(const std::string & text, ttf_font const * f, const color & c, uint32_t timeout_ms):
  control(f->get_text_rect(text)),
  _timeout_ms(timeout_ms)
{
  reset(text, f, c, timeout_ms);
}

void message::reset(const std::string & text,  ttf_font const * f, const color & c, uint32_t timeout_ms)
{
  _timer.stop();
  set_visible(false);
  _text = text;
  _tx.set_surface(f->print_solid(_text, c));
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
#ifdef GM_DEBUG_UI
    SDL_Log("message::update() - self-destruct msg \"%s\"", _text.c_str());
#endif
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
#ifdef GM_DEBUG_UI
  SDL_Log("message::show() - show message with text \"%s\" at %s", _text.c_str(), _pos.tostr().c_str());
#endif
}

void message::alert(const std::string & text, uint32_t timeout_ms)
{
  alert_ex(text, 
    current_theme().font_text, 
    current_theme().color_highlight,
    timeout_ms);
}

void message::alert_ex(const std::string & text, ttf_font const * f, const color & c, uint32_t timeout_ms)
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
