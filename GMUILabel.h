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
  typedef enum {
    left,
    right,
    center,
  } h_align;

  typedef enum {
    top,
    bottom,
    middle
  } v_align;

  // this is the position of
  // label icon if used
  typedef enum {
    icon_right,
    icon_left
  } icon_pos;

  // this structure defines a padding around a controls
  // children area
  typedef struct margin_t {
    int top;
    int left;
    int bottom;
    int right;

    margin_t() {
      top = 0; left = 0;
      bottom = 0; right = 0;
    }
    margin_t(int m) {
      top = m; left = m;
      bottom = m; right = m;
    }
    margin_t(int t, int l, int b, int r) {
      top = t; left = l;
      bottom = b; right = r;
    }
  } margin;

  // the style of label's font rendering
  typedef enum {
    solid   = 0,
    blended = 1
  } font_style;

  /* Public constructor for labels */
  label(rect pos, 
    margin pad = margin(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  virtual ~label();

  void set_margin(uint32_t pad) 
  { 
    _pad.top = pad; 
    _pad.left = pad;
    _pad.bottom = pad;
    _pad.right = pad;
  }
  void set_margin(margin & pad) { _pad = pad; }
  margin & get_margin() { return _pad; }

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
  virtual void render(SDL_Renderer* r, const rect & dst);

  /* Show-Hide animation */
  virtual void update();
  void toggle(int step = 25);

  const h_align get_halign() const { return _ha; }
  const v_align get_valign() const { return _va; }

  const bool is_hovered() const { return _hovered; }
  const bool is_focused() const { return _focused; }

  void set_halign(const h_align & ha) { _ha = ha; }
  void set_valign(const v_align & va) { _va = va; }

  const theme::label_frame * get_frame() { 
    return dynamic_cast<const theme::label_frame*>(current_theme().get_frame("label")); 
  }
  
protected:
  /* Private contructor for sub-classes to specify different type_name */
  label(const std::string & type_name,
    rect pos, 
    margin pad = margin(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  virtual void paint(SDL_Renderer * r);

  /* let derived classes control the "dirty" flag */
  void mark_dirty() { _dirty = true; }
  const texture & get_text_texture() const { return _text_tx; }
  const point & get_text_offset() const { return _text_offset; }

private:
  void on_hovered(control * target);
  void on_hover_lost(control * target);

  void on_focused(control * target);
  void on_focus_lost(control * target);

  icon_pos _ip;
  margin _pad;
  h_align _ha;
  v_align _va;

  std::string _text;
  point _text_offset;
  texture _text_tx;
  
  point _icon_offset;
  uint32_t _icon_gap;
  texture _icon_tx;
  std::string _icon_file;
  color _icon_color;
  
  ttf_font const * _font_text;
  font_style _font_style;
  color _color_idle;
  color _color_highlight;
  color _color_back;
  
  bool _dirty;
  bool _animating;
  bool _hovered;
  bool _focused;
  int _alpha;
  int _alpha_step;
};

}; //namespace ui

#endif //_GMUI_LABEL_H_