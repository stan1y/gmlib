#ifndef _GMUI_H_
#define _GMUI_H_

#include "GMLib.h"
#include "GMEventHandler.h"
#include "GMTexture.h"
#include "GMData.h"
#include "GMUITheme.h"

namespace ui {

/* Forward declare control and control_list */
class control;
typedef std::vector<control*> control_list;

/* Define control-based event_handler */
typedef ::event_handler<control*> event_handler;

/* Forward UI manager class, subclass of the control */
class manager;

/*
 * Core UI Control
 * Basic events and protocols
 */
class control {
public:

  /** UI Control core events */
  event_handler selection_change;
  event_handler hovered;
  event_handler hover_lost;
  event_handler focus;
  event_handler focus_lost;
  event_handler mouse_move;
  event_handler mouse_wheel;
  event_handler mouse_up;
  event_handler mouse_down;
  event_handler kbd_up;
  event_handler kbd_down;

  /* Generate new unique id */
  static std::string newid();

  /* Create new control */
  control(rect pos);
  control(rect pos, const std::string id);
  virtual ~control();
  
  /* Returns string name of this control type */
  std::string get_type_name();

  /* Returns string description of this control instance */
  virtual std::string tostr();

  /** UI Control protocol */
  const std::string & identifier() const { return _id; }
  void set_identifier(const std::string & id) { _id = id; }

  /* render control at absolute rect */
  virtual void render(SDL_Renderer* r, const rect & dst);
  /* per-frame update control state */
  virtual void update();
  /* load properties from data */
  virtual void load(const data &);

  /* returns an iterator corresponding to a direct child control of this control */
  control_list::iterator find_child(control* child);
  /* returns index of the child control */
  size_t find_child_index(control* child);
  /* returns order this control is drawn on parent */
  size_t zlevel();

  /* returns pointer to a child of some level of this control */
  control * find_child(const std::string & id);
  template<class T> T * find_child(const std::string & id)
  {
    control * child = find_child(id);
    if (!child)
      return nullptr;
    return dynamic_cast<T*> (child);
  }

  /* returns child control at the given screen coordinates. 
     otherwise:
     returns NULL if this control is top level or parent control 
     returns this if not (assumes that find_child_at call is made when x, y are in contor's rect)
  */
  control* find_child_at(uint32_t x, uint32_t y);
  control* find_child_at(const point & at);

  /* Get/Set parent control */
  virtual void set_parent(control* parent);
  virtual control* parent() { return _parent; }

  /* Get the absolute position of a visibile part (for rendering & collisions) of the control. 
     
     Usually rect includes the whole area of the control and control is rendered 
     directly into the frame at determined position. The position is specified 
     by the parent control as "dst" argument to control::render method. In case 
     parent performes offscreen rendering the argument to the control::render 
     would be relative position on the offscreen texture.
     Visible part of this offset texture is returned by the get_scrolled_rect() method
  */
  virtual rect get_absolute_pos();

  /* UI Control offscreen rendering support for
     children controls. If this control is a host 
     of children rendered to the offset texture then 
     this rect effectively is the visible part of the
     offset texture. Children with enabled scroll lock
     are not affected by the 
  */
  virtual rect get_scrolled_rect() { return _scrolled_rect; }
  
  /* Get/Set control relative position on parent */
  const rect& pos() const { return _pos; }
  void set_pos(rect& r) { _pos = r; }

  /* Get/set visibility of the control */
  bool visible() const { return _visible; } 
  void set_visible(bool v) { _visible = v; }

  /* Get/set event transparency flag for the control */
  bool proxy() const { return _proxy; } 
  void set_proxy(bool d) { _proxy = d; }

  /* Get/set position lock flag for the control */
  bool is_locked() { return _locked; }
  void set_locked(bool s) { _locked = s; }

  /** Children Protocol */

  /* returns const list of children of this control */
  const control_list& children() const { return _children; }
  
  virtual void add_child(control* child);
  virtual void remove_child(control* child);

protected:
  control();
  // parent control
  control* _parent;
  // own position relative to the parent's position
  rect _pos;

  // own children
  void insert_child(size_t idx, control* child);
  control_list _children;
  rect _scrolled_rect;

  /* control state */
  bool _visible;
  bool _proxy;
  bool _locked;

private:
  /* control unique id */
  std::string _id; 
};

class ui_2d_screen: public screen {
public:
  virtual void activate()
  {
  }

};

/*
  UI Manager.
  UI Manager acts as a root control of the visible UI and hosts
  all other visibile top-level controls as childern.
  These children are usually panels with their own children
  controls representing required UI.
  
  Manager instance's most used methods have shortcuts in the ui:: namespace:
  * ui::destroy(control *)
  * T * ui::build<T>(const data &)
  * void ui::pop_front(control *)
  * void ui::push_back(control *
  * T get_hovered_control<?>()
  * set_pointer(theme::pointer::pointer_type)
  * theme::pointer::pointer_type get_pointer_type()

  Example:
    ui::manager::initialze(rect(0, 0, 640, 480));
    data d("ui/example.ui.json");
    ui::panel * my_panel = ui::manager::instance()->build<ui::panel>(d);

  Example:
    ui::manager::initialze(rect(0, 0, 640, 480));
    ui::panel * my_panel = ui::build<ui::panel>("ui/example.ui.json");
  */
class manager: public control, public screen::component {
public:
  /* Manager flags */
  static const int ui_debug = 1;

  /* Initialize UI subsystem*/
  static void initialize(rect & available_rect, bool debug = false);

  /* Pointer to a currently processed event (during event handlers execution) */
  static const SDL_Event* current_event();
  
  /* Pointer to a shared global UI manager instance */
  static manager* instance();

  /* Schedule clean destruction of the control */
  void destroy(control* child);
  
  /* Desktop absolute position of the mouse pointer */
  const point& get_pointer() const { return _pointer; }

  /* Set pointer type */
  void set_pointer(theme::pointer::pointer_type t);

  /* Get current pointer type */
  theme::pointer::pointer_type get_pointer_type();

  /* Pointer to top-level hovered control */
  control* get_hovered_control() { return _hovered_cnt; }

  /* Get/Set pointer to a focus-based events receiver (a manager instance by default) */
  control* get_focused_control() { return _focused_cnt; }

  /** Construct new control assuming data is an object for
     a top-level parent control of the whole data object */
  control* build(const data &);

  /** Puts direct child at the end of the rendering order */
  void pop_front(control * c);

  /** Puts direct child at the begining of the rendering order */
  void push_back(control * c);

  /** UI Theme reference */
  const theme & get_theme() { return _theme; }

  /* Control overrides */
  virtual void update(screen * src);
  virtual void on_event(SDL_Event* ev, screen * src);
  virtual void render(SDL_Renderer* r, screen * src);

  /* Customize string representation for the manager */
  virtual std::string tostr();

  /* Get/set debug mode for UI manager */
  bool is_debug() { return _debug_mode; }
  void set_debug(bool s) { _debug_mode = s; }

private:
  manager(rect &  available_rect, bool debug = false);
  void set_hovered_control(control * target);
  void set_focused_control(control * target);

  // forbid manager::load calls
  virtual void load(data&) {};

  // render UI with debug info
  // log UI events to the console
  bool _debug_mode;

  // global pointer position
  point _pointer;

  // current hovered top-level control
  // (target of the events) */
  control * _hovered_cnt;

  // current focused control
  control * _focused_cnt;

  // current processed event
  SDL_Event* _cur_event;
  mutex _cur_event_mx;

  // theme setup
  theme _theme;
};


/* 
 * UI Namespace global functions 
 */


// void ui::destroy(control *)
// shortcut to manager's destroy method
static void destroy(control * cnt) 
{ 
  ui::manager::instance()->destroy(cnt); 
}

// T* ui::build<T>(const data &)
// template shortcut to manager's build method
template<class T> T * build(const data & d) 
{
  return dynamic_cast<T *>( manager::instance()->build(d) );
}

// ui::pop_front(control *)
// shortcut to the same manager's method
static void pop_front(control * c)
{
  manager::instance()->pop_front(c);
}

// ui::push_back(control *)
// shortcut to the same manager's method
static void push_back(control * c)
{
  manager::instance()->push_back(c);
}

// return currently hovered control or nullptr
static control * get_hovered_control()
{
  return manager::instance()->get_hovered_control();
}

// return typed-instance of currently hovered control
template<class T>
T * get_hovered_control()
{
  control * c = get_hovered_control();
  if (c == nullptr)
    return c;
  return dynamic_cast<T*> (c);
}

/* Set pointer type */
static void set_pointer(theme::pointer::pointer_type t)
{
  return manager::instance()->set_pointer(t);
}

/* Get current pointer type */
static theme::pointer::pointer_type get_pointer_type()
{
  return manager::instance()->get_pointer_type();
}


/*
 * Core Controls 
 */


/*
  UI Message control.
  Fade-out top level alert with text
  */
class message: public control {
public:
  message(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms);
  virtual ~message();
  virtual void render(SDL_Renderer * r, const rect & dst);
  virtual void update();
  void show();

  /* show global alert */
  static void alert_ex(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms);
  static void alert(const std::string & text, uint32_t timeout_ms);

private:
  void reset(const std::string & text, TTF_Font * f, const color & c, uint32_t timeout_ms);
  uint32_t _timeout_ms;
  timer _timer;
  texture _tx;
};

} //namespace ui


/* 
 * Utils
 */


/** Return abs path to resources folder for theme */
std::string UI_GetThemeRoot(const std::string & theme_path);
inline ui::manager * UI_GetManager() { return ui::manager::instance(); }
inline const ui::theme & UI_GetTheme() { return ui::manager::instance()->get_theme(); }

/* Get miliseconds since last user input (kbd, mouse, touch) */
uint32_t UI_GetUserIdle();

/* Get debug mode state for the UI manager */
inline bool UI_Debug() { return ui::manager::instance()->is_debug(); }

#endif _GMUI_H_