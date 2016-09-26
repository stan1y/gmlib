#include "GMUIButton.h"

namespace ui {

button::button(const std::string & button_subtype_name, rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  label(button_subtype_name, pos, pad, ip, ha, va),
  _checked(false)
{
  // no styles setup in base button class
  // everything should be done by derived classes
}

void lbtn::load(const data &d) 
{
  button::load(d);

  // lbtn has several styles of renderering
  if (d.has_key("btn_shape")) {
    if (d["btn_shape"].is_number()) {
      shape::shape_type s = (shape::shape_type)d["btn_shape"].as<int>();
      set_btn_shape(s);
    }
    if (d["btn_shape"].is_string()) {
      std::string s = d["btn_shape"].as<std::string>();
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