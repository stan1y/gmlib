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
typedef ::event_handler<control> event_handler;

/* Forward UI manager class */
class manager;

/****************************************************************
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

  /** UI Control protocol */
  const std::string & identifier() const { return _id; }

  /* render control at absolute rect */
  virtual void render(SDL_Renderer* r, const rect & dst);
  /* per-frame update control state */
  virtual void update();
  /* load properties from data */
  virtual void load(data &);

  bool visible() const { return _visible; } 
  void set_visible(bool v) { _visible = v; }

  bool proxy() const { return _proxy; } 
  void set_proxy(bool d) { _proxy = d; }

  /* returns an iterator corresponding to a direct child control of this control */
  control_list::iterator find_child(control* child);

  /* returns pointer to a child of some level of this control */
  control * recursive_find_child(const std::string & id);

  /* returns child control at the given screen coordinates. 
     otherwise:
     returns NULL if this control is top levelor parent control 
     returns this if not (assumes that find_child_at call is made when x, y are in contor's rect)
  */
  control* find_child_at(uint32_t x, uint32_t y);

  /* Get/Set parent control */
  virtual void set_parent(control* parent);
  virtual control* parent() { return _parent; }

  /* Get the absolute position of a visibile part (for rendering & collisions) of the control. 
     
     Usually rect includes the whole area of the control and control is rendered 
     directly into the frame at determined position. The position is specified 
     by the parent control as "dst" argument to control::render method. In case 
     parent performes offscreen rendering the argument to the control::render 
     would be relative position on the offscreen texture. 
  */
  virtual rect get_absolute_pos();

  /* UI Control offscreen rendering support for
     children controls
  */
  virtual rect get_scrolled_rect() { return _scrolled_rect; }

  /* Get/Set control relative position on parent */
  const rect& pos() const { return _pos; }
  void set_pos(rect& r) { _pos = r; }

  /** Children Protocol */

  /* returns a box of children of this control */
  const control_list& children() const { return _children; }
  virtual void add_child(control* child);
  virtual void remove_child(control* child);

protected:
  control();
  control* _parent;
  control_list _children;
  rect _scrolled_rect;
  rect _pos;
  
  /* control state */
  bool _visible;
  bool _proxy;

private:
  /* control unique id */
  std::string _id; 
};

/**************************************************************
  UI Manager. 
  */
class manager: public control, public screen::component {
public:
  /* Manager flags */
  static const int ui_debug = 1;

  /* Initialize UI subsystem*/
  static void initialize(rect & available_rect, uint32_t flags = 0);

  /* Pointer to a currently processed event (during event handlers execution) */
  static const SDL_Event* current_event();
  
  /* Pointer to a shared global UI manager instance */
  static manager* instance();

  /* Schedule clean destruction of the control */
  void destroy(control* child);
  
  /* Desktop absolute position of the mouse pointer */
  const point& get_pointer() const { return _pointer; }

  /* Pointer to top-level hovered control */
  control* get_hovered_control() { return _hovered_cnt; }

  /* Get/Set pointer to a focus-based events receiver (a manager instance by default) */
  control* get_focused_control() { return _focused_cnt; }

  /** Construct new control assuming data is an object for
     a top-level parent control of the whole data object */
  static control* build(data &);

  /** UI Theme reference */
  const theme & get_theme() { return _theme; }

  /* Get/Set UI flags */
  const uint32_t & get_flags() { return _flags; }
  void set_flags(uint32_t f) { _flags = f; }

  /* Control overrides */
  virtual void update(screen * src);
  virtual void on_event(SDL_Event* ev, screen * src);
  virtual void render(SDL_Renderer* r, screen * src);

private:
  manager(rect &  available_rect, uint32_t flags);
  void set_hovered_control(control * target);
  void set_focused_control(control * target);

  // forbid manager::load calls
  virtual void load(data&) {};

  // ui flags
  uint32_t _flags;

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

// ui::destroy(control *)
// quick-link to manager's destroy method
inline void destroy(control * cnt) { ui::manager::instance()->destroy(cnt); }

/******************** Core Controls ***************************/
/**
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

/******************** Utils ***********************************/

/** Return abs path to resources folder for theme */
std::string UI_GetThemeRoot(const std::string & theme_path);
inline ui::manager * UI_GetManager() { return ui::manager::instance(); }
inline const ui::theme & UI_GetTheme() { return ui::manager::instance()->get_theme(); }

/* Get miliseconds since last user input (kbd, mouse, touch) */
uint32_t UI_GetUserIdle();

#endif _GMUI_H_