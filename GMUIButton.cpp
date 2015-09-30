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

};