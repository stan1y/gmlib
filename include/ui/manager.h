#ifndef _GMUI_H_
#define _GMUI_H_

#include "engine.h"
#include "evhndlr.h"
#include "texture.h"



namespace ui {

/* Forward declare control and control_list */
class control;
typedef container<control*> control_list;

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
  event_handler focused;
  event_handler focus_lost;
  event_handler mouse_move;
  event_handler mouse_wheel;
  event_handler mouse_up;
  event_handler mouse_down;
  event_handler kbd_up;
  event_handler kbd_down;

  /* Create new control */
  control(const rect & pos);
  control(const rect & pos, const std::string id);
  control(const rect & pos, const std::string id, std::string & theme_name);
  virtual ~control();
  
  /* Returns string name of this control type */
  virtual std::string get_type_name() const { return "control"; }

  /* Returns string description of this control instance */
  virtual std::string tostr() const;

  /** UI Control protocol */
  const std::string & identifier() const { return _id; }
  void set_identifier(const std::string & id) { _id = id; }

  /* render control at absolute rect */
  virtual void draw(SDL_Renderer* r, const rect & dst);
  /* per-frame update control state */
  virtual void update();
  /* load properties from data */
  virtual void load(const json &);

  /* returns an iterator corresponding to a direct child control of this control */
  control_list::const_iterator find_child(const control* child);
  control_list::iterator find_child(control* child);

  /* returns index of the child control */
  size_t find_child_index(const control* child);
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
  virtual control* parent() const { return _parent; }

  /* Get the absolute position of a visibile part (for rendering & collisions) of the control. 
     
     Usually rect includes the whole area of the control and control is rendered 
     directly into the frame at determined position. The position is specified 
     by the parent control as "dst" argument to control::render method. In case 
     parent performes offscreen rendering the argument to the control::render 
     would be relative position on the offscreen texture.
     Visible part of this offset texture is returned by the get_scrolled_rect() method
  */
  virtual rect get_absolute_pos() const;

  /* UI Control offscreen rendering support for
     children controls. If this control is a host 
     of children rendered to the offset texture then 
     this rect effectively is the visible part of the
     offset texture. Children with enabled scroll lock
     are not affected by the 
  */
  virtual rect get_scrolled_rect() const { return _scrolled_rect; }
  
  /* Get/Set control relative position on parent */
  const rect& pos() const { return _pos; }
  void set_pos(const rect& r) { _pos = r; }

  /* Get/set visibility of the control */
  bool visible() const { return _visible; }
  void set_visible(bool v) { _visible = v; }

  /* Get/set flag to harvest this control on the next cycle */
  bool destroyed() const { return _destroyed; }
  void set_destroyed(bool d) { _destroyed = d; }

  /* Get/set event transparency flag for the control */
  bool proxy() const { return _proxy; } 
  void set_proxy(bool d) { _proxy = d; }

  /* Get/set position lock flag for the control */
  bool locked() const { return _locked; }
  void set_locked(bool s) { _locked = s; }

  /* Get/set disabled state flag for the control */
  bool disabled() const { return _disabled; }
  void set_disabled(bool s) { _disabled = s; }

  /** Children Protocol */

  /* returns const list of children of this control */
  const control_list& children() const { return _children; }
  /* returns pointer to child at index without lock on children */
  control * get_child_at_index(size_t idx);
  
  /* Add remove child by pointer. sub-classes can override these.
     See ui::box for offset children rendering example.
  */
  virtual void add_child(control* child);
  /* Remove child by pointer */
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
  bool _destroyed;
  bool _disabled;

private:
  /* control unique id */
  std::string _id; 
};

/*
  UI Manager.
  UI Manager acts as a root control of the visible UI and hosts
  all other visibile top-level controls as childern.
  These children are usually panels with their own children
  controls representing required UI.
  
  Manager instance's most used methods have shortcuts in the ui:: namespace:
  * ui::destroy(control *)
  * T * ui::build<T>(const json &)
  * void ui::pop_front(control *)
  * void ui::push_back(control *
  * T get_hovered_control<?>()
  
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

  /* Initialize UI subsystem */
  static void initialize(rect & available_rect,
                         const std::string & theme,
                         const std::string & font = "default.ttf",
                         const int font_size = 12,
                         color color_idle = color::black(),
                         color color_highlight = color::black(),
                         color color_back = color::white());
                         

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
  control* build(const json &);

  /** Puts direct child at the end of the rendering order */
  void pop_front(control * c);

  /** Puts direct child at the begining of the rendering order */
  void push_back(control * c);

  /** UI Theme API */
  const texture & get_theme() { return _theme; }
  const ttf_font & get_font() { return _font; }
  const color get_color_idle() { return _theme_color_idle; }
  void set_color_idle(color idle) { _theme_color_idle = idle; }
  const color get_color_highlight() { return _theme_color_highlight; }
  void set_color_highlight(color highlight) { _theme_color_highlight = highlight; }
  const color get_color_back() { return _theme_color_back; }
  void set_color_back(color back) { _theme_color_back = back; }

  /* screen::component protocol overrides */
  virtual void on_update(screen *);
  virtual void on_event(SDL_Event* ev);
  virtual void render(SDL_Renderer* r);

  /* Customize string representation for the manager */
  virtual std::string tostr() const;

private:
  manager(rect &  available_rect,
    const std::string & theme_file,
    const std::string & theme_font,
    const int theme_font_size,
    const color & color_idle,
    const color & color_highlight,
    const color & color_back);
  
  void set_hovered_control(control * target);
  void set_focused_control(control * target);

  // forbid manager::load calls
  //virtual void load(data&) {};

  // global pointer position
  point _pointer;

  // current hovered top-level control
  // (target of the events) */
  control * _hovered_cnt;

  // current focused control
  control * _focused_cnt;

  // current processed event
  SDL_Event* _cur_event;
  sdl_mutex _cur_event_mx;

  // Theme settings
  texture _theme;
  ttf_font _font;
  color _theme_color_idle;
  color _theme_color_highlight;
  color _theme_color_back;
  
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

// T* ui::build<T>(const json &)
// template shortcut to manager's build method
template<class T> T * build(const json & d)
{
  return dynamic_cast<T *>( manager::instance()->build(d) );
}

// T* ui::build<T>(const std::string &)
// template shortcut to manager's build method
template<class T> T * build_file(const std::string & file)
{
  json d;
  SDL_Log("ui::build_file - loading %s", file.c_str());
  std::ifstream (media_path(file)) >> d;
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

static const texture * current_theme()
{
  return &ui::manager::instance()->get_theme();
}

/*
 * Core classes 
 */

// this structure defines a padding around a controls
// children area
typedef struct s_padding {
  int top;
  int left;
  int bottom;
  int right;

  s_padding(int m = 0) {
    top = m; left = m;
    bottom = m; right = m;
  }
  s_padding(int t, int l, int b, int r) {
    top = t; left = l;
    bottom = b; right = r;
  }
} padding;

// horizontal alignment selection
typedef enum h_align_t {
  left,
  right,
  center,
  expand
} h_align;

// vertical alignment selection
typedef enum v_align_t {
  top,
  bottom,
  middle,
  fill
} v_align;

inline std::string halign_to_str(const h_align & ha)
{
  switch(ha) {
  case h_align::left:
    return "left";
  case h_align::right:
    return "right";
  case h_align::center:
    return "center";
  case h_align::expand:
    return "expand";
  default:
    throw std::runtime_error("Can not convert unknown h_align value");
  };
}

inline h_align halign_from_str(const std::string & s)
{
  if (s == "left")
    return h_align::left;
  else if (s == "right")
    return h_align::right;
  else if (s == "center")
    return h_align::center;
  else if (s == "expand")
    return h_align::expand;
  else
    throw std::runtime_error("Can not convert string to h_align");
}

inline std::string valign_to_str(const v_align & va)
{
  switch(va) {
  case v_align::top:
    return "top";
  case v_align::bottom:
    return "bottom";
  case v_align::middle:
    return "middle";
  case v_align::fill:
    return "fill";
  default:
    throw std::runtime_error("Can not convert unknown v_align value");
  };
}

inline v_align valign_from_str(const std::string & s)
{
  if (s == "top")
    return v_align::top;
  else if (s == "bottom")
    return v_align::bottom;
  else if (s == "middle")
    return v_align::middle;
  else if (s == "fill")
    return v_align::fill;
  else
    throw std::runtime_error("Can not convert string to v_align");
}

/*
  UI Message control.
  Fade-out top level alert with text
  */
class message: public control {
public:
  
  virtual ~message();
  virtual void render(SDL_Renderer * r, const rect & dst);
  virtual void update();
  void show();

  virtual std::string get_type_name() const { return "message"; }

  /* show global alert */
  static void alert_ex(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms);
  static void alert(const std::string & text, uint32_t timeout_ms);

protected:
  /* use static methods to create new instances of global message alert */
  message(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms);

private:
  void reset(const std::string & text, const ttf_font & f, const color & c, uint32_t timeout_ms);
  uint32_t _timeout_ms;
  timer _timer;
  std::string _text;
  texture _tx;
};

namespace text {
  /* load texture data by rendering text with font */
  static texture * print_solid(const std::string& text, ttf_font const * font, const color & clr);
  static texture * print_blended(const std::string& text, ttf_font const * font, const color & clr);
  
  /* calculate rect needed to hold text rendered with font & color */
  static rect get_rect(const std::string& text, ttf_font const * font);
}

} //namespace ui


/* 
 * Utils
 */

/* Get miliseconds since last user input (kbd, mouse, touch) */
uint32_t UI_GetUserIdle();

#endif //_GMUI_H_
