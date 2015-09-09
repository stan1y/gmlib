#include "GMUIButton.h"

namespace ui {

button::button(rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  label(pos, pad, ip, ha, va),
  _checked(false)
{
  hovered += boost::bind( &button::on_hovered, this, _1 );
  hover_lost += boost::bind( &button::on_hover_lost, this, _1 );
}

void button::on_hovered(control * target)
{
  // change text font to hightlight
  const theme & th = UI_GetTheme();
  set_font_color(th.btn.font_hover_color);
}

void button::on_hover_lost(control * target)
{
  // change text font to hightlight
  const theme & th = UI_GetTheme();
  set_font_color(th.btn.font_color);
}

void button::render_as(const theme & th, const theme::button_frame & f, SDL_Renderer * r, const rect & dst)
{
  // render button frame & back
  th.draw_button_frame(f, r, dst);
  // render label contents on top
  label::render(r, dst);
}

};