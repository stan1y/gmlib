#ifndef _GMUI_BUTTON_H_
#define _GMUI_BUTTON_H_

#include "GMUILabel.h"

namespace ui {

/**
  UI Button base class.
  Does not render any frames, see theme specific classes
  such as ui::btn, ui::sbtn, ui::tbtn
*/
class button : public label {
public:
  virtual ~button() {}

  bool checked() { return _checked; }
  void set_checked(bool c) { _checked = c; }

  void render_as(const theme & th, const theme::button_frame & f, SDL_Renderer * r, const rect & dst);

  virtual void load(const data &d) { label::load(d); }

protected:
  /* Private constructor for button sub-classes. 
     No direct 'button' instances allowed'
  */
  button(
    const std::string & button_subtype_name,
    rect pos, 
    margin pad = margin_t(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  bool _checked;
};

/**
  UI Standard size button. 
  It is using "btn" frame from theme
*/
class btn : public button {
public:
  btn(rect pos, 
    margin pad = margin_t(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top):
  button("btn", pos, pad, ip, ha, va)
  {
    set_font(&UI_GetTheme().font_text_bold);
    set_font_idle_color(UI_GetTheme().color_front);
    set_font_hover_color(UI_GetTheme().color_highlight);
  }

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    const theme & th = UI_GetTheme();
    render_as(th, th.btn, r, dst);
  }
};

/**
  UI Small size button.
  It is using "sbtn" frame from theme
*/
class sbtn : public button {
public:
  sbtn(rect pos, 
    margin pad = margin_t(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top):
  button("sbtn", pos, pad, ip, ha, va)
  {
    set_font(&UI_GetTheme().font_text_norm);
    set_font_idle_color(UI_GetTheme().color_front);
    set_font_hover_color(UI_GetTheme().color_highlight);
  }

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    const theme & th = UI_GetTheme();
    render_as(th, th.sbtn, r, dst);
  }
};

/**
  UI Small size button.
  It is rendered with privitive rect, roundedrect, and so on
*/
class lbtn : public button {
public:
  typedef enum {
    rectangle = 1,
    rounded   = 2,
    prism     = 3
  } btn_style;

  lbtn(rect pos, 
    margin pad = margin_t(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top):
  button("lbtn", pos, pad, ip, ha, va)
  {
    set_font(&UI_GetTheme().font_text_norm);
    set_font_idle_color(UI_GetTheme().color_front);
    set_font_hover_color(UI_GetTheme().color_toolbox);
  }

  void set_btn_style(btn_style s) { _btn_style = s; }
  btn_style get_btn_style() { return _btn_style; }

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    get_font_color().apply(r);
    switch(_btn_style) {
      case lbtn::rectangle:
      default:
        SDL_RenderDrawRect(r, &dst);
        break;
      case lbtn::rounded:
        SDLEx_RenderDrawRoundedRect(r,
          dst.x, dst.y, dst.x + dst.w, dst.y + dst.h, 3);
        break;
      case lbtn::prism:
        break;
    }
    label::render(r, dst);
  }

  virtual void load(const data &);

private:
  btn_style _btn_style;
};

}; //namespace ui

#endif //_GMUI_BUTTON_H_