#include "GMUIButton.h"

namespace ui {

button::button(const rect & pos,
               const icon_pos & ip,
               const h_align & ha,
               const v_align & va,
               const padding & pad):
  label(pos, ip, ha, va, pad),
  _checked(false)
{
  // enable text highlight for buttons
  enable_hightlight_on_hover();
}

void lbtn::load(const data &d) 
{
  button::load(d);

  // lbtn has several styles of renderering
  if (d.has_key("btn_shape")) {
    if (d["btn_shape"].is_value_number()) {
      shape::shape_type s = (shape::shape_type)d["btn_shape"].value<int>();
      set_btn_shape(s);
    }
    if (d["btn_shape"].is_value_string()) {
      std::string s = d["btn_shape"].value<std::string>();
      if (s == "rectangle") {
        set_btn_shape(shape::rectangle);
      }
      if (s == "rounded") {
        set_btn_shape(shape::rounded);
      }
      if (s == "prism") {
        set_btn_shape(shape::prism);
      }
    }
  }
  else {
    set_btn_shape(shape::rectangle);
  }
}

}