#ifndef _GMUI_PUSH_BUTTON_H_
#define _GMUI_PUSH_BUTTON_H_

#include "GMUI.h"

namespace ui {

class push_btn : public control {
public:

  // create push_btn with specific rect
  push_btn(const rect & pos, 
          std::string idle_file_path,
          std::string disabled_file_path = "",
          std::string pressed_file_path = "");

  // create push_btn with size of the idle icon
  push_btn(std::string idle_file_path,
          std::string disabled_file_path = "",
          std::string pressed_file_path = "");

  virtual std::string get_type_name() const { return "pushbtn"; }

  virtual ~push_btn();

  bool is_sticky() { return _sticky; }
  void set_sticky(bool s) { _sticky = s; }

  void set_idle_icon(const fs::path & file_path);
  void set_idle_icon(SDL_Texture* tex);

  void set_disabled_icon(const fs::path & file_path);
  void set_disabled_icon(SDL_Texture* tex);

  void set_pressed_icon(const fs::path & file_path);
  void set_pressed_icon(SDL_Texture* tex);

  virtual void load(const data &);
  virtual void render(SDL_Renderer* r, const rect & dst);

private:

  const theme::push_button_skin * get_skin() const
  {
    return dynamic_cast<const theme::push_button_skin*> (current_theme().get_skin("pushbtn"));
  }

  void on_disabled(control * target);
  void on_hover_lost(control * target);
  void on_mouse_down(control * target);
  void on_mouse_up(control * target);

  bool _sticky;
  bool _is_pressed;

  texture _idle_icon;
  texture _disabled_icon;
  texture _pressed_icon;
};

}; //namespace ui

#endif //_GMUI_PUSH_BUTTON_H_