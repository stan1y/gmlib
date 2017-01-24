#include "GMUIBox.h"

/* Names of the drag states */
static const char * DRAG_STATE_NAME[] = {
  "Stop",
  "Start",
  "Moving"
};

namespace ui {

/* UI scrollbar */

box::scrollbar::scrollbar(box * container, 
                          rect relpos,
                          scrollbar_type type):
  control(relpos),
  _type(type),
  _drag(cursor_drag_state::stop),
  _container(container),
  _color_idle(get_skin()->color_idle),
  _color_highlight(get_skin()->color_highlight),
  _color_back(get_skin()->color_back)
{
  if (_container == NULL) throw std::runtime_error("NULL container specified for scrollbar");
  _drag = cursor_drag_state::stop;
  set_type(type);

  // add box scrolling support
  mouse_up += boost::bind(&scrollbar::on_mouse_up, this, _1);
  mouse_down += boost::bind(&scrollbar::on_mouse_down, this, _1);
  mouse_wheel += boost::bind(&scrollbar::on_mouse_wheel, this, _1);
  mouse_move += boost::bind(&scrollbar::on_mouse_move, this, _1);
}

/* rect for cursor, position relative to (inside) the scrollbar's pos */
rect box::scrollbar::get_cursor_rect(const rect & children_rect) const
{
  if (_type == scrollbar_type::scrollbar_right) {
    rect avail = _container->get_scrolled_rect();
    float step = _pos.h / (float)children_rect.h;
    int cursor_h = float_to_sint32(avail.h * step);
    /* Y coord of the avail rect affects position of the cursor */
    return rect(
      2,
      2 + float_to_sint32(avail.y * step), 
      _pos.w - 4, cursor_h - 4);
  }
  if (_type == scrollbar_type::scrollbar_bottom) {
  }
  throw std::runtime_error("not implemented");
}

void box::scrollbar::render(SDL_Renderer * r, const rect & dst)
{
  // draw cursor
  rect cursor_rect = get_cursor_rect(_container->get_children_rect());
  cursor_rect += dst.topleft();
  if (_drag != cursor_drag_state::stop) {
    // drag state is active
    _color_highlight.apply(r);
  }
  else {
    _color_idle.apply(r);
  }
  SDL_RenderFillRect(r, &cursor_rect);

  const point & pointer = manager::instance()->get_pointer();
  if (get_absolute_pos().collide_point(pointer)) {
    // hovered scrollbar rect -> hightlight
    _color_highlight.apply(r);
  }
  else {
    _color_idle.apply(r);
  }
  // draw border
  SDL_RenderDrawRect(r, &dst);
  // draw cursor border
  SDL_RenderDrawRect(r, &cursor_rect);
  
}

void box::scrollbar::set_state(cursor_drag_state s) 
{
  SDL_Log("box::scrollbar::set_state(%d) - change state to %s", s, DRAG_STATE_NAME[s]);
  _drag = s; 
}

void box::scrollbar::update()
{
  const point & pointer = manager::instance()->get_pointer();
  rect scrollbar_rect = _pos + _container->get_absolute_pos().topleft();
  if (!scrollbar_rect.collide_point(pointer) && get_state() == scrollbar::cursor_drag_state::moving) {
    set_state(scrollbar::cursor_drag_state::stop);
  }
  control::update();
}

void box::scrollbar::on_mouse_move(control * target)
{
  const point & pnt = manager::instance()->get_pointer();
  rect scrollbar_rect = _pos + _container->get_absolute_pos().topleft();
  
  if (get_state() == scrollbar::cursor_drag_state::start) {
    set_state(scrollbar::cursor_drag_state::moving);
  }
  if (get_state() == scrollbar::cursor_drag_state::moving) {
    _container->scroll_to_point(pnt - scrollbar_rect.topleft());
  }
}

void box::scrollbar::on_mouse_up(control * target)
{
  if (get_state() == scrollbar::cursor_drag_state::moving) {
    // drag is done 
    set_state(scrollbar::cursor_drag_state::stop);
  }

  // click on the scrollbar itself - empty area
  const point & pnt = manager::instance()->get_pointer();
  rect scrollbar_rect = _pos + _container->get_absolute_pos().topleft();
  if (scrollbar_rect.collide_point(pnt)) {
    // user clicked and holds button while hovering over scrollbar
    _container->scroll_to_point(pnt - scrollbar_rect.topleft());
  }
}

void box::scrollbar::on_mouse_down(control * target)
{
  const point & pointer = manager::instance()->get_pointer();
  rect scrollbar_rect = _pos + _container->get_absolute_pos().topleft();
  rect scrollbar_cursor_rect = get_cursor_rect(_container->get_children_rect()) + scrollbar_rect.topleft();
  if (scrollbar_cursor_rect.collide_point(pointer)) {
    set_state(scrollbar::cursor_drag_state::start);
  }
}

void box::scrollbar::on_mouse_wheel(control * target)
{
  SDL_Log("box::scrollbar::on_mouse_wheel");
  const SDL_Event* sdl_ev = manager::instance()->current_event();
  _container->do_scroll(
    sdl_ev->wheel.x * scrollbar::scroll_speed, 
    sdl_ev->wheel.y * scrollbar::scroll_speed
  );
}

}