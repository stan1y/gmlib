#ifndef _GMUI_BOX_H_
#define _GMUI_BOX_H_

#include "manager.h"
#include "frame.h"
#include "button.h"

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
    scrollbar_hidden     = 0x000,
    scrollbar_vertical   = 0x001,
    scrollbar_horizontal = 0x010,
    scrollbar_both       = scrollbar_vertical | scrollbar_horizontal,
      
  } scroll_type;
  
  static const int scroll_speed = 20;
  
  typedef enum {
    stop   = 0,
    start  = 1,
    vdrag  = 2,
    hdrag  = 3,
  } scroll_drag_state;
  
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
    if (_vscroll != nullptr) {
      remove_child(_vscroll);
      ui::destroy(_vscroll);
    }
    if (_hscroll != nullptr) {
      remove_child(_hscroll);
      ui::destroy(_hscroll);
    }
  }

  /* if this box displays scroll bar */
  bool is_scrollbar_hidden() 
  { 
    return (_scroll_type == scroll_type::scrollbar_hidden);
  }
  
  // scrollbar event handlers
  void on_scroll_mouse_up(control * target);
  void on_scroll_mouse_down(control * target);
  void on_scroll_mouse_move(control * target);
  void on_scroll_mouse_wheel(control * target);
  
  void set_scroll_type(scroll_type t, int size);
  scroll_type get_scroll_type() const { return _scroll_type; }
  int get_scroll_size() const { return _scroll_size; }
  
  // get/set state of the scrollbar cursor
  const scroll_drag_state get_scroll_state() { return _scroll_state; }
  void set_scroll_state(scroll_drag_state s);

  // returns position of the scrollbar cursor rect on the box
  // with relation to the scrolled area and total size of children_rect
  rect get_vscroll_rect() const;
  rect get_hscroll_rect() const;

  /* get and set box alignment setup */
  const h_align & get_halign() const { return _ha; }
  const v_align & get_valign() const { return _va; }

  void set_halign(const h_align & ha) { _ha = ha; }
  void set_valign(const v_align & va) { _va = va; }

  /* box contents padding */
  void set_padding(uint32_t pad) 
  { 
    _pad.top = pad; 
    _pad.left = pad;
    _pad.bottom = pad;
    _pad.right = pad;
  }
  void set_padding(padding & pad) { _pad = pad; }
  const padding & get_padding() { return _pad; }

  virtual void update();
  
  // render this box contents
  virtual void draw(SDL_Renderer* r, const rect & dst);

  // add and auto-postion child on this box
  virtual void add_child(control* child);

  // remove child and reposition children
  virtual void remove_child(control* child);

  /* Returns a size of the offscreen texture used to rendered all children */
  virtual rect get_children_rect() { return _children_rect; }

  /* Setup box settings from data */
  virtual void load(const json &);

  /* returns scrolled position for children of this control to draw */
  virtual rect get_scrolled_rect() const { return _scrolled_rect; }
  
  // scroll box width given delta pixels
  void do_scroll(int dx, int dy);

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

  /* Box settings */
  box_type _type;
  
  // h_align effective for vbox
  h_align _ha;
  // v_align effective for hbox
  v_align _va;

  // offscreen render target texture for children
  texture _body;
  bool _dirty;

  // children available area bounds
  padding _pad;
  // actual area occupied by children
  rect _children_rect;
  // step in px between children in positioning
  // both for vertical and horizontal layout
  int _gap;
  
  // scroll controls
  button * _vscroll;
  button * _hscroll;
  scroll_type _scroll_type;
  int _scroll_size;
  scroll_drag_state _scroll_state;

  // box selection
  control * _selected_ctl;
};

/**
  UI Panel: Themed box
*/
class panel: public box, public tileframe {
public:
  panel(const rect & pos, 
        const box_type & t = box::vbox,
        const h_align & ha = h_align::center, 
        const v_align & ba = v_align::top, 
        const padding & pad = padding(2),
        const int & gap = 2);
  virtual ~panel();

  virtual std::string get_type_name() const { return "panel"; }

  virtual void draw(SDL_Renderer* r, const rect & dst);
  virtual void load(const json &);

private:
  color _color_back;
};

}; //namespace ui

#endif //_GMUI_BOX_H_
