#ifndef _GMUI_LABEL_H_
#define _GMUI_LABEL_H_

#include "GMUI.h"

namespace ui {

/**
  UI Label Control.
  Static display of text and/or icon
*/
class label: public control {
public:

  // this is the position of
  // label icon if used
  typedef enum {
    icon_right,
    icon_left
  } icon_pos;

  // the style of label's font rendering
  typedef enum {
    solid   = 0,
    blended = 1
  } font_style;

  static font_style font_style_from_str(const std::string & s);

  /* Public constructor for labels */
  label(const rect & pos, 
    const icon_pos & icon = icon_pos::icon_left,
    const h_align & ha = h_align::center,
    const v_align & va = v_align::middle,
    const padding & pad = padding(2),
    const int gap = 2);

  virtual ~label();

  virtual std::string get_type_name() const { return "label"; }

  /* label contents padding */
  void set_padding(uint32_t pad) 
  { 
    _pad.top = pad; 
    _pad.left = pad;
    _pad.bottom = pad;
    _pad.right = pad;
  }
  void set_padding(padding & pad) { _pad = pad; }
  const padding & get_padding() { return _pad; }

  virtual void load(const data &);

  /* Label text */
  void set_text(const std::string& txt);
  const std::string& get_text() { return _text; }
  
  /* Label text font */
  void set_font(ttf_font const * f)
  {
    _font_text = f;
    _dirty = true;
  }
  ttf_font const * get_font() { return _font_text; }

  void set_font_style(font_style s) { _font_style = s; }
  const font_style get_font_style() { return _font_style; }

  /* Idle and hovered color options */
  void set_highlight_color(const color & c)
  {
    _color_highlight = c;
    _dirty = true;
  }
  color get_hightlight_color() { return _color_highlight; }

  void set_idle_color(const color & c)
  {
    _color_idle = c;
    _dirty = true;
  }
  color get_idle_color() { return _color_idle; }

  void set_background_color(const color & c)
  {
    _color_back = c;
    _dirty = true;
  }
  color get_background_color() { return _color_back; }

  void enable_hightlight_on_focus()
  {
    _highlight_on_focus = true;
    _dirty = true;
  }
  void disable_hightlight_on_focus()
  {
    _highlight_on_focus = false;
    _dirty = true;
  }

  void enable_hightlight_on_hover()
  {
    _highlight_on_hover = true;
    _dirty = true;
  }
  void disable_hightlight_on_hover()
  {
    _highlight_on_hover = false;
    _dirty = true;
  }

  /* Label icon */
  void set_icon(const std::string& icon_file);
  void set_icon(SDL_Surface* icon);
  void set_icon(SDL_Texture* icon);
  const texture & get_icon() { return _icon_tx; }
  color get_icon_color() { return _icon_color; }
  void set_icon_color(color & c)
  {
    _icon_color = c;
    _dirty = true;
  }

  /* Label rendering */
  virtual void draw(SDL_Renderer* r, const rect & dst);

  /* Show-Hide animation */
  virtual void update();
  void toggle(int step = 25);

  const h_align & get_halign() const { return _ha; }
  const v_align & get_valign() const { return _va; }

  void set_halign(const h_align & ha) { _ha = ha; }
  void set_valign(const v_align & va) { _va = va; }

  const bool is_hovered() const { return _hovered; }
  const bool is_focused() const { return _focused; }
  const bool is_pressed() const { return _pressed; }

protected:

  virtual void paint(SDL_Renderer * r);

  /* let derived classes control the "dirty" flag */
  void mark_dirty() { _dirty = true; }
  const texture & get_text_texture() const { return _text_tx; }
  const point & get_text_offset() const { return _text_offset; }
  void paint_text(texture & tx, const std::string & text, const ttf_font * fnt, const color & clr);

private:
  void on_hovered(control * target);
  void on_hover_lost(control * target);

  void on_focused(control * target);
  void on_focus_lost(control * target);

  void on_mouse_up(control * target);
  void on_mouse_down(control * target);

  padding _pad;
  h_align _ha;
  v_align _va;

  std::string _text;
  point _text_offset;
  texture _text_tx;
  
  icon_pos _icon_pos;
  uint32_t _icon_gap;
  std::string _icon_file;
  color _icon_color;

  point _icon_offset;
  texture _icon_tx;
  
  ttf_font const * _font_text;
  font_style _font_style;

  color _color_idle;
  color _color_highlight;
  color _color_back;
  
  // call ::paint if this control
  // is dirty in ::render stage
  bool _dirty;

  bool _hovered;
  bool _highlight_on_hover;
  bool _focused;
  bool _highlight_on_focus;
  bool _pressed;

  bool _animating;
  int _alpha;
  int _alpha_step;
};

}; //namespace ui

#endif //_GMUI_LABEL_H_
