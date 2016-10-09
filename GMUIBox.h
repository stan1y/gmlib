#ifndef _GMUI_BOX_H_
#define _GMUI_BOX_H_

#include "GMUI.h"

namespace ui {

/**
  UI Box: Auto-layout controls container.
  Implements VBOX & HBOX layouting with
  automatic scrollbar support.
*/
class box: public control {
public:
  typedef enum {
    none = 0,
    vbox = 1,
    hbox = 2
  } box_type;

  typedef enum {
    no_style   = 0,
    center     = 1,
    fill       = 2,
    pack_start = 3, /* left-right for vbox*/
    pack_end   = 4  /* up-down for hbox */
  } box_style;


  /**
    Box scrollbar
  */
  typedef enum {
    scrollbar_hidden = 0,
    scrollbar_right  = 1,
    scrollbar_bottom = 2,
  } scrollbar_type;

  class scrollbar : public control {
  public:
    static const int scroll_speed = 20;
    typedef enum {
      stop = 0,
      start = 1,
      moving = 2
    } cursor_drag_state;

    scrollbar(box * container, rect pos,
      /* horizontal or vertical */
      scrollbar_type type);

    void set_type(scrollbar_type t) { _type = t; }
    scrollbar_type type() const { return _type; }
    
    // get/set state of the scrollbar cursor
    const cursor_drag_state get_state() { return _drag; }
    void set_state(cursor_drag_state s);

    // returns position of the scrollbar cursor rect on the box
    // with relation to the scrolled area and total size of children_rect
    rect get_cursor_rect(const rect & children_rect) const;

    virtual void render(SDL_Renderer * r, const rect & dst);
    virtual void update();

    const theme::label_skin * get_skin()
    {
      return dynamic_cast<const theme::label_skin*> (current_theme().get_skin("scrollbar"));
    }

  private:
    color _color_idle;
    color _color_back;
    color _color_highlight;

    // scrollbar event handlers
    void on_mouse_up(control * target);
    void on_mouse_down(control * target);
    void on_mouse_move(control * target);
    void on_mouse_wheel(control * target);

    scrollbar_type _type;
    cursor_drag_state _drag;
    box * _container; /* a parent of box class */
  };

  /* Public constructor of a new box container of given type */
  box(rect pos, box_type t = box::vbox, 
    box_style s = box::no_style, int margin = 0);

  virtual ~box() 
  {
    if (_scroll != NULL) {
      delete _scroll;
    }
  }

  bool is_scrollbar_visible() 
  { 
    return (sbar() != scrollbar_type::scrollbar_hidden);
  }

  scrollbar_type sbar() { return (_scroll == NULL ? scrollbar_type::scrollbar_hidden : _scroll->type()); }
  void set_sbar(scrollbar_type t, uint32_t ssize);

  /** Control protocol overrides */
  virtual void render(SDL_Renderer* r, const rect & dst);
  virtual void add_child(control* child);
  virtual void remove_child(control* child);

  /* Returns a size of the offscreen texture used to rendered all children */
  virtual rect get_children_rect() { return _children_rect; }

  /* Setup box settings from data */
  virtual void load(const data &);

  /* returns scrolled position for children of this control to draw */
  virtual rect get_scrolled_rect() { return _scrolled_rect; }
  
  // scroll box width given delta pixels
  void do_scroll(int dx, int dy);

  // scroll box to a point on it's absolute body
  void scroll_to_point(const point & pnt);

  // auto-positioning of children controls
  // children can override to implement more layouting options
  virtual void update_children();

  // remove all children of this box
  void clear_children();

  const box_style get_box_style() const { return _style; }
  void set_box_style(const box_style & s);

protected:
  /* Public constructor of a new box container of given type */
  box(const std::string & type_name,
    rect pos, box_type t = box::vbox, 
    box_style s = box::no_style, int margin = 0);

  // update selection on the box
  void switch_selection(control * target);

  
  /* box need to handle events from it's children */
  void on_child_click(control * target);
  void on_child_hover_changed(control * target);
  void on_child_wheel(control * target);
  void on_box_wheel(control * target);

  // box settings
  int _margin;
  box_type _type;
  box_style _style;
  scrollbar * _scroll;

  // body texture (children's render target)
  texture _body;
  bool _dirty;
  rect _children_rect;

  // box selection
  control * _selected_ctl;
};

/**
  UI Panel: Themed box
*/
class panel: public box {
public:
  typedef enum {
    dialog   = 1,
    toolbox = 2,
    group   = 3,
    window  = 4
  } panel_style;

  panel(rect pos, panel_style ps = panel_style::dialog, box_type t = box::vbox, box_style s = box::no_style, int margin = 0);
  virtual ~panel();

  color get_background_color() { return _color_back; }
  void set_background_color(const color & c) { _color_back = c; }

  virtual void render(SDL_Renderer* r, const rect & dst);
  virtual void load(const data &);

  const theme::container_skin * get_skin() {
    switch (_ps) {
      case panel_style::dialog:
        return dynamic_cast<const theme::container_skin*>(current_theme().get_skin("dialog")); 
      case panel_style::group:
        return dynamic_cast<const theme::container_skin*>(current_theme().get_skin("group")); 
      case panel_style::toolbox:
        return dynamic_cast<const theme::container_skin*>(current_theme().get_skin("toolbox")); 
      case panel_style::window:
        return dynamic_cast<const theme::container_skin*>(current_theme().get_skin("window"));
      default:
        SDLEx_LogError("panel::get_skin - unknown panel style %d", _ps);
        throw std::exception("Uknown panel style");
        break;
    };
    
  }

  const panel::panel_style panel::get_panel_style() const { return _ps; }
  void panel::set_panel_style(const panel_style & ps)
  { 
    _ps = ps; 
    set_background_color(get_skin()->color_back);
  }

private:
  panel_style _ps;
  color _color_back;
};

}; //namespace ui

#endif //_GMUI_BOX_H_