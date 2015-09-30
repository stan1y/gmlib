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
  button(rect pos, 
    margin pad = margin_t(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  virtual ~button() {}

  bool checked() { return _checked; }
  void set_checked(bool c) { _checked = c; }

  void render_as(const theme & th, const theme::button_frame & f, SDL_Renderer * r, const rect & dst);

protected:
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
  button(pos, pad, ip, ha, va)
  {}

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
  button(pos, pad, ip, ha, va)
  {}

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    const theme & th = UI_GetTheme();
    render_as(th, th.sbtn, r, dst);
  }
};

}; //namespace ui

#endif //_GMUI_BUTTON_H_