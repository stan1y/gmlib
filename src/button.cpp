#include "button.h"

namespace ui {

button::button(const rect & pos, 
    const icon_pos & ip,
    const h_align & ha, 
    const v_align & va,
    const padding & pad):
  label(pos, ip, ha, va, pad),
  tileframe(ui::current_theme_sprites())
{
  set_style(get_type_name());
}

void button::load(const json &d)
{
  label::load(d);

  if (d.find("sticky") != d.end())
    set_sticky(d["sticky"]);
  if (d.find("pressed") != d.end() && is_sticky())
    set_stuck(d["pressed"]);
}

void button::draw(SDL_Renderer * r, const rect & dst)
{
  // draw button frame & back
  if (is_pressed() || is_stuck()) {
    draw_frame(r, 3, 6, dst, false, false);
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
