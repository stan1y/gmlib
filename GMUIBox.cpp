#include "GMUIBox.h"

namespace ui {

/** UI BOX Container Implementation **/

box::box(rect pos, box_type t, box_style s, int margin):
    control(pos), _margin(margin), _type(t), _style(s),
    _scroll(NULL)
{
  _children_rect = rect(0, 0, _margin, _margin);
  _selected_ctl = NULL;
  _dirty = false;

  // add box scrolling support
  mouse_up += boost::bind(&box::on_mouse_up, this, _1);
  mouse_down += boost::bind(&box::on_mouse_down, this, _1);
  mouse_wheel += boost::bind(&box::on_mouse_wheel, this, _1);
  mouse_move += boost::bind(&box::on_mouse_move, this, _1);
}

void box::update()
{
  if (_scroll != NULL) {
    const point & pointer = manager::instance()->get_pointer();
    rect scrollbar_rect = _scroll->get_absolute_pos();
    if (!scrollbar_rect.collide_point(pointer) && _scroll->get_state() == scrollbar::cursor_drag_state::moving) {
      _scroll->set_state(scrollbar::cursor_drag_state::stop);
    }
  }
  control::update();
}

void box::on_mouse_move(control * target)
{
  if (sbar() == scrollbar_type::scrollbar_hidden)
    return;
  
  const point & pnt = manager::instance()->get_pointer();
  rect scrollbar_rect = _scroll->get_absolute_pos();
  
  if (_scroll->get_state() == scrollbar::cursor_drag_state::start) {
    _scroll->set_state(scrollbar::cursor_drag_state::moving);
  }
  if (_scroll->get_state() == scrollbar::cursor_drag_state::moving) {
    scroll_to_point(pnt - scrollbar_rect.topleft());
  }
}

void box::on_mouse_up(control * target)
{
  if (sbar() == scrollbar_type::scrollbar_hidden)
    return;

  if (_scroll->get_state() == scrollbar::cursor_drag_state::moving) {
    _scroll->set_state(scrollbar::cursor_drag_state::stop);
  }

  const point & pnt = manager::instance()->get_pointer();
  rect scrollbar_rect = _scroll->pos() + get_absolute_pos().topleft();
  if (scrollbar_rect.collide_point(pnt)) {
    // user clicked and holds button while hovering over scrollbar
    scroll_to_point(pnt - scrollbar_rect.topleft());
  }
}

void box::scroll_to_point(const point & pnt)
{
  rect cursor_rect = _scroll->get_cursor_rect(_children_rect);
  int cursor_center_x = cursor_rect.x + cursor_rect.w / 2;
  int cursor_center_y = cursor_rect.y + cursor_rect.h / 2;
  do_scroll(pnt.x - cursor_center_x, pnt.y - cursor_center_y);
}

void box::on_mouse_down(control * target)
{
  if (sbar() == scrollbar_type::scrollbar_hidden)
    return;
  const point & pointer = manager::instance()->get_pointer();
  rect scrollbar_rect = _scroll->pos() + get_absolute_pos().topleft();
  rect scrollbar_cursor_rect = _scroll->get_cursor_rect(_children_rect) + scrollbar_rect.topleft();
  if (scrollbar_cursor_rect.collide_point(pointer)) {
    _scroll->set_state(scrollbar::cursor_drag_state::start);
  }
}

void box::on_mouse_wheel(control * target)
{
  if (sbar() == scrollbar_type::scrollbar_hidden)
    return;
  const SDL_Event* sdl_ev = manager::instance()->current_event();
  do_scroll(
    sdl_ev->wheel.x * scrollbar::scroll_speed, 
    sdl_ev->wheel.y * scrollbar::scroll_speed
  );
}

void box::set_sbar(scrollbar_type t, uint32_t ssize) 
{
  if (_scroll != NULL) {
    remove_child(_scroll);
    ui::destroy(_scroll);
    _scroll = NULL;
  }

  switch (t) {
  case scrollbar_right:
    _scroll = new scrollbar(this, rect(_pos.w - ssize, 0, ssize, _pos.h), t);
    break;
  // FIXME: Add support for scrollbar_bottom type too
  };
  
  // display new type of scrollbar if configured now
  if (_scroll) {
    add_child(_scroll);
  }
}

void box::render(SDL_Renderer * r, const rect & dst)
{
  if (is_scrollbar_visible()) {
    // offscreen children rendering 
    if (_dirty) {
      //create render target
      _body.blank(_children_rect.w, _children_rect.h, SDL_TEXTUREACCESS_TARGET);
      {
        texture::render_context ctx(&_body, r);
        control_list::iterator it = _children.begin();
        for(; it != _children.end(); ++it) {
          control * c = *it;
          // render control with it's relative position
          // to the target of the render_context 
          c->render(r, c->pos());
        }
      }
      _dirty = false;
    }
    // render pre-made children
    _body.render(r, _scrolled_rect, rect(dst.x, dst.y, _scrolled_rect.w, _scrolled_rect.h));
    // render scrollbar
    //_scroll->render(r, dst);
  }
  else {
    // direct children rendering
    control::render(r, dst);
  }
}

void box::remove_child(control * c)
{
  // remove and update children
  control::remove_child(c);
  //c->set_offscreen(false);
  update_children();
}

void box::add_child(control* c)
{
  // box need to subscribe on children for book-keeping
  c->mouse_up += boost::bind( &box::on_child_click, this, _1 );
  c->hovered += boost::bind( &box::on_child_hover_changed, this, _1 );
  c->hover_lost += boost::bind( &box::on_child_hover_changed, this, _1 );
  c->mouse_wheel += boost::bind( &box::on_mouse_wheel, this, _1 );
  
  //c->set_offscreen(true);

  control::add_child(c);
  update_children();
}

void box::on_child_hover_changed(control * target)
{
  //rebuild on child hover change
  _dirty = true;
}

void box::on_child_click(control * target)
{
  // update selected control
  if (_selected_ctl != target) {
    switch_selection(dynamic_cast<control*>(target));
  }
  else if (_selected_ctl != NULL) {
    switch_selection(NULL);
  }
}


void box::load(const data & d)
{
  if (d.has_key("type")) {
    if (d["type"].is_number()) {
      uint32_t itype = d.get("type", (uint32_t)box_type::vbox);
      _type = (box_type)itype;
    }
    if (d["type"].is_string()) {
      std::string stype = d["type"].as<std::string>();
      if (stype == "vbox") {
        _type = box_type::vbox;
      }
      if (stype == "hbox") {
        _type = box_type::hbox;
      }
    }
  }
  else {
    _type = box_type::none;
  }

  if (d.has_key("style")) {
    if (d["style"].is_number()) {
      uint32_t istyle = d.get("style", (uint32_t)box_style::center);
      _style = (box_style)istyle;
    }
    if (d["style"].is_string()) {
      std::string sstyle = d["style"].as<std::string>();
      if (sstyle == "center") {
        _style = box_style::center;
      }
      if (sstyle == "fill") {
        _style = box_style::fill;
      }
      if (sstyle == "pack_start") {
        _style = box_style::pack_start;
      }
      if (sstyle == "pack_end") {
        _style = box_style::pack_end;
      }
    }
  }

  if (d.has_key("scroll")) {
    if (d["scroll"].is_string()) {
      scrollbar_type stype = scrollbar_type::scrollbar_hidden;
      std::string scroll = d["scroll"].as<std::string>();
      if (scroll == "right")
        stype = scrollbar_type::scrollbar_right;
      if (scroll == "bottom")
        stype = scrollbar_type::scrollbar_bottom;
    
      int ssize = 16;
      if (d.has_key("scrollbar_size")) {
        if (d["scrollbar_size"].is_number()) {
          ssize = d["scrollbar_size"].as<int>();
        }
      }

      set_sbar(stype, ssize);
    }
  }
  else {
    set_sbar(
      scrollbar_type::scrollbar_hidden,
      d.get("scrollbar_size", 0)
    );
  }

  _margin = d.get("margin", 0);

  control::load(d);
}

void box::switch_selection(control * target)
{
  // switch selected control
  control * prev = _selected_ctl;
  _selected_ctl = target;
  if (prev != NULL)
    prev->selection_change(prev);
  if (_selected_ctl != NULL)
    _selected_ctl->selection_change(_selected_ctl);
  // rebuild on selection change
  _dirty = true;
}

void box::do_scroll(int dx, int dy)
{
  if (_scroll == NULL)
    return;

  if (_scroll->type() == scrollbar_type::scrollbar_right) {
    float step = _scroll->pos().h / (float)_children_rect.h;
    _scrolled_rect.y += float_to_sint32(step * dy);
    if (_scrolled_rect.y < 0) 
      _scrolled_rect.y = 0;
    if (_scrolled_rect.y > _children_rect.h - _scrolled_rect.h)
      _scrolled_rect.y = _children_rect.h - _scrolled_rect.h;
  }
  if (_scroll->type() == scrollbar_type::scrollbar_bottom) {
    throw std::exception("not implemented");
  }
}

void box::update_children()
{
  //std::sort(_children.begin(), _children.end());
  int last_pos = _margin;
  _children_rect.w = last_pos;
  _children_rect.h = last_pos;
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control* child = (*it);
    // skip scrollbar of this box if exist
    if (_scroll != NULL && child == _scroll)
      continue;
    // skip hidden
    if (!child->visible())
      continue;
    rect pos = child->pos();
    // position type
    switch(_type) {
    case box::none:
      _children_rect.w = max(pos.x + pos.w, _children_rect.w);
      _children_rect.h = max(pos.y + pos.h, _children_rect.h);
      break;
    case box::vbox:
      pos.x = _margin;
      pos.y = last_pos;
      last_pos += pos.h + _margin;
      _children_rect.h = last_pos;
      break;
    case box::hbox:
      pos.x = last_pos;
      pos.y = _margin;
      last_pos += pos.w + _margin;
      _children_rect.w += last_pos;
      break;
    default:
      SDLEx_LogError("box: unknown box type value = %d", _type);
      throw std::exception("unknown box type value");
      break;
    };
    // position by style
    switch(_style) {
    case box::pack_start:
      if (_type == box::vbox) {
        //align left
        pos.x = _margin;
      }
      if (_type == box::hbox) {
        //align up
        pos.y = _margin;
      }
      break;
    case box::pack_end:
      if (_type == box::vbox) {
        //align right
        pos.x = _pos.w - _margin - pos.w;
      }
      if (_type == box::hbox) {
        //align down
        pos.y = _pos.h - _margin - pos.h;
      }
      break;
    case box::center:
      if (_type == box::vbox) {
        pos.x = (_pos.w - pos.w) / 2;
      }
      if (_type == box::hbox) {
        pos.y = (_pos.h - pos.h) / 2;
      }
      break;
    case box::fill:
      if (_type == box::vbox) {
        pos.w = _pos.w - 2 * _margin;
      }
      if (_type == box::hbox) {
        pos.h = _pos.h - 2 * _margin;
      }

      break;
    case box::no_style:
      break;
    default:
      SDLEx_LogError("box: unknown box type value = %d", _type);
      throw std::exception("unknown box type value");
      break;
    };
    //update child position (rect)
    SDL_Log("box: {%s} type(%d) style(%d) - place child {%s} at %s",
      identifier().c_str(), _type, _style,
      child->identifier().c_str(),
      pos.tostr().c_str());
    child->set_pos(pos);
  }

  // normalize children rect
  _children_rect.w = max(_children_rect.w, _scrolled_rect.w);
  _children_rect.h = max(_children_rect.h, _scrolled_rect.h);
  

  if (_scroll) {
    control_list::iterator it = find_child(_scroll);
    _children.erase(it);
    _children.push_back(_scroll);
  }
  
  // rebuild on children add/remove/show/hide/reorder
  _dirty = true;
}

/**
  UI Panel box
  */

panel::panel(rect pos, panel_style ps, box_type t, box_style s, int margin):
  box(pos, t, s, margin)
{
  set_panel_style(ps);
}

panel::~panel()
{
}

void panel::load(const data & d)
{
  if (d.has_key("panel_style")) {
    if (d["panel_style"].is_string()) {
      std::string pstyle = d["panel_style"].as<std::string>();
      if (pstyle == "dialog") {
        set_panel_style(panel_style::dialog);
      }
      if (pstyle == "toolbox") {
        set_panel_style(panel_style::toolbox);
      }
      if (pstyle == "group") {
        set_panel_style(panel_style::group);
      }
    }
  }
  else {
    _ps = panel_style::dialog;
  }

  if (d.has_key("background")) {
    if (d["background"].is_array()) {
      _back = d["background"].as<color>();
    }
  }
  box::load(d);
}

void panel::render(SDL_Renderer * r, const rect & dst)
{
  const theme & th = UI_GetTheme();

  // render background
  _back.apply(r);
  SDL_RenderFillRect(r, &dst);

  // render box content
  box::render(r, dst);

  // render box frame
  switch(_ps) {
  case panel_style::dialog:
    th.draw_container_frame(th.dialog, r, dst);
    break;
  case panel_style::toolbox:
    th.draw_container_frame(th.toolbox, r, dst);
    break;
  case panel_style::group:
    th.draw_container_frame(th.group, r, dst);
    break;
  };
}

panel::panel_style panel::get_panel_style() 
{ 
  return _ps;
}

void panel::set_panel_style(panel_style ps) 
{ 
  _ps = ps;
  const theme & th = UI_GetTheme();
  switch (_ps) {
  default:
  case panel_style::dialog:
  case panel_style::group:
    set_background_color(th.color_back);
    break;

  case panel_style::toolbox:
    set_background_color(th.color_toolbox);
    break;
  }
}

} //namespace ui