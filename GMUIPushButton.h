#ifndef _GMUI_PUSH_BUTTON_H_
#define _GMUI_PUSH_BUTTON_H_

#include "GMUI.h"

namespace ui {

class push_button : public control {
public:

  push_button(rect pos, std::string idle = "", std::string hovered = "", std::string pressed = "");
  virtual ~push_button();

  bool is_sticky() { return _sticky; }
  void set_sticky(bool s) { _sticky = s; }

  void set_idle_icon(const std::string & res);
  void set_idle_icon(SDL_Texture* tex);

  void set_hoverd_icon(const std::string & res);
  void set_hovered_icon(SDL_Texture* tex);

  void set_pressed_icon(const std::string & res);
  void set_pressed_icon(SDL_Texture* tex);

  bool is_pressed() { return _press; }

  virtual void load(const data &);

private:

  void on_hover(control * target);
  void on_hover_lost(control * target);
  void on_mouse_down(control * target);
  void on_mouse_up(control * target);

  bool _sticky;
  bool _press;

  texture _idle;
  texture _hovered;
  texture _pressed;
};

}; //namespace ui

#endif //_GMUI_PUSH_BUTTON_H_