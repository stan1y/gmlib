#include "GMUtil.h"
#include "GMUI.h"
#include "GMUIBox.h"
#include "GMUILabel.h"
#include "GMUIButton.h"
#include "GMUITextInput.h"
#include "GMUICombo.h"

/** User Idle Counter **/

static uint32_t g_usr_idle_cnt = 0;
uint32_t UI_GetUserIdle()
{
  return g_usr_idle_cnt;
}

namespace ui {

static manager* g_manager = NULL;
typedef locked_vector<control*> dead_list;
static dead_list g_graveyard;

static message * g_message = NULL;
static mutex g_message_mx;

/** Manager **/

void manager::initialize(rect & available_rect, uint32_t flags)
{
  if (g_manager == NULL) {
    g_manager = new manager(available_rect, flags);
  }
  else {
    g_manager->set_pos(available_rect);
    g_manager->set_flags(flags);
  }
}

manager * manager::instance()
{
  if (g_manager == NULL) {
    SDLEx_LogError("manager::instance() - not initialized");
    throw std::exception("manager is not initialized");
  }
  return g_manager;
}

const SDL_Event* manager::current_event()
{
  if (g_manager) return g_manager->_cur_event;
  return NULL;
}

manager::manager(rect & available_rect, uint32_t flags):
  control(),
  screen::component(NULL),
  _flags(flags),
  _theme(GM_GetConfig()->default_ui),
  _cur_event(NULL),
  _focused_cnt(NULL),
  _hovered_cnt(NULL)
{
  set_pos(available_rect);
}

void manager::destroy(control* child)
{
  child->set_visible(false);
  g_graveyard.push_back(child);
}

void manager::set_focused_control(control * target)
{
  if (target == _focused_cnt)
    return;
  if (target == NULL)
    target = this;
  if (_focused_cnt != NULL) {
    SDL_Log("manager: focus lost {%s}", _focused_cnt->identifier().c_str());
    _focused_cnt->focus_lost(_focused_cnt);
  }
  _focused_cnt = target;
  if (_focused_cnt != NULL) {
    SDL_Log("manager: focus gain {%s}", _focused_cnt->identifier().c_str());
    _focused_cnt->focus(_focused_cnt);
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
    SDL_Log("manager: hover lost {%s} - %s left %s", 
      prev->identifier().c_str(),
      _pointer.tostr().c_str(),
      prev->get_absolute_pos().tostr().c_str());
    prev->hover_lost(prev);
  }
  // setup new
  if (_hovered_cnt != NULL) {
    SDL_Log("manager: hover gain {%s} - at %s", 
      _hovered_cnt->identifier().c_str(),
      _hovered_cnt->get_absolute_pos().tostr().c_str()
    );
    _hovered_cnt->hovered(_hovered_cnt);
  }
}

/* UI Manager Component interface */

void manager::render(SDL_Renderer* r, screen * src)
{
  // call UI protocol's render
  control::render(r, get_absolute_pos());
  // render pointer of theme supports
  if (_theme.ptr.tx_normal.is_valid()) {
    point pnt;
    SDL_GetMouseState(&pnt.x, &pnt.y);
    _theme.draw_pointer(r, rect(0, 0, 16, 16) + pnt);
  }
}

void manager::update(screen * scr)
{
  // count idle miliseconds
  g_usr_idle_cnt += GM_GetFrameTicks();

  SDL_GetMouseState(&_pointer.x, &_pointer.y);

  // process graveyard
  g_graveyard.lock();
  {
    dead_list::iterator it = g_graveyard.begin();
    for(; it != g_graveyard.end(); ++it) {
      control* child = *it;
      child->parent()->remove_child(child);
      delete child;
    }
    g_graveyard.clear();
  }
  g_graveyard.unlock();

  // call UI protocol's update
  control::update();
}

void manager::on_event(SDL_Event* ev, screen * src)
{
  _cur_event_mx.lock();
  _cur_event = ev;

  // reset user idle timer
  if (ev->type == SDL_MOUSEMOTION || ev->type == SDL_MOUSEWHEEL ||
      ev->type == SDL_MOUSEBUTTONDOWN || ev->type == SDL_MOUSEBUTTONUP ||
      ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP)
  {
    g_usr_idle_cnt = 0;
  }

  // update hovered control if pointer was moved
  if (ev->type == SDL_MOUSEMOTION) {
    control * found = find_child_at(ev->motion.x, ev->motion.y);
    // reset found to NULL if actual control is hidden or proxy
    if (found != NULL && !found->visible() )
      found = NULL;
    set_hovered_control(found);
  }

  // update focused control before callbacks
  if (ev->type == SDL_MOUSEBUTTONUP) {
    set_focused_control(_hovered_cnt);
  }

  // check event has a target or point at manager
  // check target is acceptable
  control * target = g_manager;
  if (_hovered_cnt != NULL && _hovered_cnt->visible() && !_hovered_cnt->proxy()) 
    target = _hovered_cnt;
  
  // find and call a handler for event
  switch(ev->type)
  {
  case SDL_MOUSEWHEEL:
    target->mouse_wheel(target);
    break;
  case SDL_MOUSEMOTION:
    target->mouse_move(target);
    break;
  case SDL_MOUSEBUTTONUP:
    target->mouse_up(target);
    break;
  case SDL_MOUSEBUTTONDOWN:
    target->mouse_down(target);
    break;
  case SDL_KEYUP:
    if (_focused_cnt)
      _focused_cnt->kbd_up(_focused_cnt);
    break;
  case SDL_KEYDOWN:
    if (_focused_cnt)
      _focused_cnt->kbd_down(_focused_cnt);
    break;
  };

  _cur_event = NULL;
  _cur_event_mx.unlock();

  // update hovered again becase callback might
  // had changed state of other controls (hidden/deleted)
  {
    control * found = find_child_at(ev->motion.x, ev->motion.y);
    // reset found to NULL if actual control is hidden or proxy
    if (found != NULL && !found->visible() )
      found = NULL;
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
  size_t idx = find_child_index(c);
  if (idx == MAXSIZE_T) {
    return;
  }
  size_t last = _children.size() - 1;
  control * tmp = _children[last];
  _children[last] = c;
  _children[idx] = tmp;
}

void manager::push_back(control * c)
{
  size_t idx = find_child_index(c);
  if (idx == MAXSIZE_T) {
    return;
  }
  control * tmp = _children[0];
  _children[0] = c;
  _children[idx] = tmp;
}

control* manager::build(const data & d)
{
  if (!d.is_object()) {
    SDLEx_LogError("manager::build - given data is not a json object");
    throw std::exception("given data is not a json object");
  }

  std::string class_id = d["class"].as<std::string>();
  rect pos;
  // check "rect" propecty
  if (d.has_key("rect")) {
    pos = d["rect"].as<rect>();
  }
  else {
    // check "size" property
    if (d.has_key("size")) {
      std::pair<int,int> size = d["size"].as<std::pair<int,int>>();
      pos.w = size.first;
      pos.h = size.second;
    }
    else {
      SDLEx_LogError("manager::build - failed to parse control");
      throw std::exception("failed to parse control data");
    }

    // check position property
    if (d.has_key("position")) {
      if ( d["position"].is_string()) {
        std::string p = d["position"].as<std::string>();
        if (p == "center") {
          pos.x = (manager::instance()->pos().w - pos.w) / 2;
          pos.y = (manager::instance()->pos().h - pos.h) / 2;
        }
      }
      if (d["position"].is_array()) {
        point pp = d["position"].as<point>();
        pos.x = pp.x; pos.y = pp.y;
      }
    }
  }

  // create instance of the class
  control * inst = create_control(class_id, pos);
  if (inst == NULL) {
    SDLEx_LogError("manager::build() - failed to create an instance of [%s]", class_id);
    throw std::exception("failed to create an instance of the class");
  }

  // let instance load it's props from data
  inst->load(d);

  // process children
  if (d.has_key("children")) {
    data children = d["children"];
    if (children.is_array()) {
      size_t len = children.length();
      for(size_t i = 0; i < len; ++i) {
        inst->add_child(build(children[i]));
      }
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
message::message(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms):
  _timeout_ms(timeout_ms),
  control(texture::get_string_rect(text, f))
{
  reset(text, f, c, timeout_ms);
}

void message::reset(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms)
{
  _timer.stop();
  set_visible(false);
  _tx.load_text_solid(text, f, c);
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
  //deplect 1 alpha every 10 ms
  uint32_t depleted = ticked / 10;
  if (depleted >= 255) {
    _timer.stop();
    SDL_Log("message: self-destruct");
    //self-destruct when alpha depleted
    manager::instance()->destroy(this);
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
  const theme & th = UI_GetTheme();
  alert_ex(text, 
    th.font_text_bold.ptr(), 
    th.color_highlight, 
    timeout_ms);
}

void message::alert_ex(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms)
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
  _tx.render(r, dst);
}

} //namespace ui
