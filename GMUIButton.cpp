#include "GMUIButton.h"

namespace ui {

button::button(rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  label(pos, pad, ip, ha, va),
  _checked(false)
{
  set_font_hover_color(UI_GetTheme().color_highlight);
}

void button::render_as(const theme & th, const theme::button_frame & f, SDL_Renderer * r, const rect & dst)
{
  // render button frame & back
  th.draw_button_frame(f, r, dst);
  // render label contents on top
  label::render(r, dst);
}

void lbtn::load(const data &d) 
{
  button::load(d);

  // lbtn has several styles of renderering
  if (d.has_key("btn_style")) {
    if (d["btn_style"].is_number()) {
      lbtn::btn_style s = (lbtn::btn_style)d["btn_style"].as<int>();
      set_btn_style(s);
    }
    if (d["btn_style"].is_string()) {
      std::string s = d["btn_style"].as<std::string>();
      if (s == "rectangle") {
        set_btn_style(lbtn::rectangle);
      }
      if (s == "roundede") {
        set_btn_style(lbtn::rounded);
      }
      if (s == "prism") {
        set_btn_style(lbtn::prism);
      }
    }
  }
  else {
    set_btn_style(lbtn::rectangle);
  }
}

}