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

    virtual std::string get_type_name() const { return "scrollbar"; }

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
    scrollbar_type _type;
    cursor_drag_state _drag;
    box * _container; /* a parent of box class */

    // colors 
    color _color_idle;
    color _color_highlight;
    color _color_back;

    // scrollbar event handlers
    void on_mouse_up(control * target);
    void on_mouse_down(control * target);
    void on_mouse_move(control * target);
    void on_mouse_wheel(control * target);

    
  };

  /* Public constructor of a new box container of given type */
  box(const rect & pos,
    const box_type & t = box::vbox,
    const h_align & ha = h_align::center,
    const v_align & va = v_align::top,
    const padding & pad = padding(),
    const int & gap = 2);

  virtual std::string get_type_name() const { return "box"; }
  std::string get_box_type_name() const;

  virtual ~box() 
  {
    if (_scroll != NULL) {
      delete _scroll;
    }
  }

  /* skin of this box */
  virtual const theme::container_skin * get_skin() { return theme::container_skin::dummy(); }

  /* if this box displays scroll bar */
  bool is_scrollbar_visible() 
  { 
    return (sbar() != scrollbar_type::scrollbar_hidden);
  }

  /* get and set scrollbar state for this box */
  scrollbar_type sbar() { return (_scroll == NULL ? scrollbar_type::scrollbar_hidden : _scroll->type()); }
  void set_sbar(scrollbar_type t, uint32_t ssize);

  /* get and set box alignment setup */
  const h_align & get_halign() const { return _ha; }
  const v_align & get_valign() const { return _va; }

  void set_halign(const h_align & ha) { _ha = ha; }
  void set_valign(const v_align & va) { _va = va; }

  // render this box contents
  virtual void draw(SDL_Renderer* r, const rect & dst);

  // render debug frame
  void draw_debug_frame(SDL_Renderer * r, const rect & dst);

  // add and auto-postion child on this box
  virtual void add_child(control* child);

  // remove child and reposition children
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

protected:

  // update selection on the box
  void switch_selection(control * target);
  
  /* box need to handle events from it's children */
  void on_child_click(control * target);
  void on_child_hover_changed(control * target);
  void on_child_wheel(control * target);
  void on_box_wheel(control * target);

  // box settings
  box_type _type;
  // child allignment in a box
  // h_align effective for vbox
  h_align _ha;
  // v_align effective for hbox
  v_align _va;

  // body texture (children's render target)
  texture _body;
  bool _dirty;

  // children available area bounds
  padding _pad;
  // actual area occupied by children
  rect _children_rect;
  // step in px between children in positioning
  // both for vertical and horizontal layout
  int _gap;
  
  // scroll control
  scrollbar * _scroll;

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

  panel(const rect & pos, 
        const panel_style & ps = panel_style::dialog, 
        const box_type & t = box::vbox,
        const h_align & ha = h_align::center, 
        const v_align & ba = v_align::top, 
        const padding & pad = padding(2),
        const int & gap = 2);
  virtual ~panel();

  virtual std::string get_type_name() const { return "panel"; }

  color get_background_color() { return _color_back; }
  void set_background_color(const color & c) { _color_back = c; }

  virtual void draw(SDL_Renderer* r, const rect & dst);
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
        fprintf(stderr, "panel::get_skin - unknown panel style %d", _ps);
        throw std::runtime_error("Uknown panel style");
        break;
    };
    
  }

  /* get and set panel style */
  const panel_style get_panel_style() const { return _ps; }
  void set_panel_style(const panel_style & ps)
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