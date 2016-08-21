#include "GMUIBox.h"

namespace ui {

/* UI scrollbar */

box::scrollbar::scrollbar(box * container, 
                          rect relpos,
                          scrollbar_type type):
  control(relpos),
  _type(type),
  _container(container)
{
  _drag = cursor_drag_state::stop;
  set_type(type);
}

/* rect for cursor, position relative to (inside) the scrollbar's pos */
rect box::scrollbar::get_cursor_rect(const rect & children_rect) const
{
  if (_type == scrollbar_type::scrollbar_hidden) 
    return rect();

  if (_type == scrollbar_type::scrollbar_right) {
    rect avail = _container->get_scrolled_rect();
    float step = _pos.h / (float)children_rect.h;
    int cursor_h = float_to_sint32(avail.h * step);
    /* Y coord of the avail rect affects position of the cursor */
    return rect(
      2,
      2 + float_to_sint32(avail.y * step), 
      _pos.w - 4, cursor_h);
  }
  if (_type == scrollbar_type::scrollbar_bottom) {
  }
  throw std::exception("not implemented");
}

void box::scrollbar::render(SDL_Renderer * r, const rect & dst)
{
  const theme & th = UI_GetTheme();

  // draw border
  th.color_front.apply(r);
  SDL_RenderDrawRect(r, &dst);
  
  // draw cursor
  rect cursor_rect = get_cursor_rect(_container->get_children_rect());
  cursor_rect += dst.topleft();
  if (_drag != cursor_drag_state::stop) {
    // drag state is active
    th.color_highlight.apply(r);
  }
  else {
    th.color_back.apply(r);
  }
  SDL_RenderFillRect(r, &cursor_rect);

  const point & pointer = manager::instance()->get_pointer();
  if (get_absolute_pos().collide_point(pointer)) {
    // hovered scrollbar rect -> hightlight
    th.color_highlight.apply(r);
  }
  else {
    th.color_front.apply(r);
  }
  // draw cursor border
  SDL_RenderDrawRect(r, &cursor_rect);
  
}

}