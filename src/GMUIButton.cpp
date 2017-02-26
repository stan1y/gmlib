#include "GMUIButton.h"

namespace ui {

button::button(const rect & pos, 
    const icon_pos & ip,
    const h_align & ha, 
    const v_align & va,
    const padding & pad):
  label(pos, ip, ha, va, pad),
	tileframe(ui::current_theme())
  {
    set_font(ui::manager::instance()->get_font());
    set_idle_color(ui::manager::instance()->get_color_idle());
    set_highlight_color(ui::manager::instance()->get_color_highlight());
  }


  void button::draw(SDL_Renderer * r, const rect & dst)
  {
    // draw button frame & back
    if (is_pressed()) {
      draw_frame(r, 3, 3, dst, false, false);
    }
    else if (is_hovered()) {
      draw_frame(r, 3, 3, dst, false, false);  
    }
    else {
      draw_frame(r, 0, 3, dst, false, false);
    }
  
    // draw label contents on top
    label::draw(r, dst);
  }
	
}