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

  typedef enum {
    icon_right,
    icon_left
  } icon_pos;

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
  void set_font(const theme::font * f)
  {
    _font = f;
    _dirty = true;
  }
  const theme::font * get_font() { return _font; }
  
  /* Currently used label text font color */
  void set_font_color(const color & c)
  {
    _font_color = c;
    _dirty = true;
  }
  color get_font_color() { return _font_color; }

  /* Idle and hovered color options */
  void set_font_hover_color(const color & c)
  {
    _font_hover_color = c;
    if (this == ui::get_hovered_control())
      set_font_color(_font_hover_color);
  }
  color get_font_hover_color() { return _font_hover_color; }

  void set_font_idle_color(const color & c)
  {
    _font_idle_color = c;
    if (this != ui::get_hovered_control())
      set_font_color(_font_idle_color);
  }
  color get_font_idle_color() { return _font_idle_color; }

  /* Label icon */
  void set_icon(const std::string& res);
  void set_icon(SDL_Surface* icon);
  void set_icon(SDL_Texture* icon);
  const texture & get_icon() { return _icon_tx; }
  color get_icon_color() { return _icon_clr; }
  void set_icon_color(color & c)
  {
    _icon_clr = c;
    _dirty = true;
  }

  /* Label rendering */
  virtual void render(SDL_Renderer* r, const rect & dst);

  /* Show-Hide animation */
  virtual void update();
  void toggle(int step = 25);

  const h_align get_halign() const { return _ha; }
  const v_align get_valign() const { return _va; }

  void set_halign(const h_align & ha) { _ha = ha; }
  void set_valign(const v_align & va) { _va = va; }
  
protected:
  /* Private contructor for sub-classes to specify different type_name */
  label(const std::string & type_name,
    rect pos, 
    margin pad = margin(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  void paint(SDL_Renderer * r);

  void on_hovered(control * target);
  void on_hover_lost(control * target);

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
  const theme::font * _font;
  color _font_color;
  color _font_hover_color;
  color _font_idle_color;
  std::string _icon_res;
  color _icon_clr;
  
  bool _dirty;
  bool _animating;
  int _alpha;
  int _alpha_step;
};

}; //namespace ui

#endif //_GMUI_LABEL_H_