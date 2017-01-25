#include "GMUIBox.h"

namespace ui {

/** UI BOX Container Implementation **/

box::box(const rect & pos, const box_type & t, const h_align & ha, const v_align & va, const padding & pad, const int & gap):
    control(pos), _type(t), _ha(ha), _va(va),
    _dirty(false),
    _pad(pad), 
    _children_rect(0, 0, _pad.top, _pad.left),
    _gap(gap),
    _scroll(NULL),
    _selected_ctl(NULL)
{
  mouse_wheel += boost::bind(&box::on_box_wheel, this, _1);
}

void box::scroll_to_point(const point & pnt)
{
  rect cursor_rect = _scroll->get_cursor_rect(_children_rect);
  int cursor_center_x = cursor_rect.x + cursor_rect.w / 2;
  int cursor_center_y = cursor_rect.y + cursor_rect.h / 2;
  do_scroll(pnt.x - cursor_center_x, pnt.y - cursor_center_y);
}


void box::set_sbar(scrollbar_type t, uint32_t ssize) 
{
  if (_scroll != NULL) {
    ui::destroy(_scroll);
    _scroll = NULL;
  }

  switch (t) {
    case scrollbar_right:
      _scroll = new scrollbar(this, rect(_pos.w - ssize, 0, ssize, _pos.h), t);
      _scroll->set_locked(true);
      break;

    case scrollbar_bottom:
      // FIXME: implement bottom scrollbar
      break;

    default:
    case scrollbar_hidden:
      // do nothing
      break;
  };
  
  // display new type of scrollbar if configured now
  if (_scroll) {
    add_child(_scroll);
  }
}

void box::draw_debug_frame(SDL_Renderer * r, const rect & dst)
{
  const control * hovered = ui::get_hovered_control();
  control_list::const_iterator child = find_child(hovered);

  if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LCTRL]) {
    return;
  }
  
  if (this == hovered || child != _children.end()) {

    color self_color = color::red().set_alpha(128);
    if (child != _children.end()) {
      self_color = color::dark_red().set_alpha(128);
      // hovered child frame
      rect child_rect = (*child)->pos() + dst.topleft();
      color::red().set_alpha(128).apply(r);
      SDL_RenderDrawRect(r, &child_rect);
    }

    if (!is_scrollbar_visible()) {
      // no offscreen rendering
      color::green().set_alpha(128).apply(r);
      SDL_RenderDrawRect(r, &dst);
      
      color::yellow().set_alpha(128).apply(r);
      rect area = theme::get_container_user_area(get_skin(), dst);
      SDL_RenderDrawRect(r, &area);
    }
    else {
      // with offset rendering
      self_color.set_alpha(128).apply(r);
      SDL_RenderDrawRect(r, &dst);

      color::yellow().set_alpha(128).apply(r);
      rect area = theme::get_container_user_area(get_skin(), dst);
      SDL_RenderDrawRect(r, &area);

      color::green().set_alpha(128).apply(r);
      rect scrollbar_rect = _scroll->pos() + dst.topleft();
      SDL_RenderDrawRect(r, &scrollbar_rect);
    }
  }
}

void box::draw(SDL_Renderer * r, const rect & dst)
{
  if (is_scrollbar_visible()) {
    
    /** offscreen children rendering **/
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
          // skip scrollbar since, it rendered separetly on top
          if (c == _scroll) continue;
          c->draw(r, c->pos());
        }
      }
      _dirty = false;
    }
    // render pre-made children
    _body.render(r, _scrolled_rect, rect(dst.x, dst.y, _scrolled_rect.w, _scrolled_rect.h));
    // render scrollbar
    rect scrollbar_rect = _scroll->pos() + dst.topleft();
    _scroll->draw(r, scrollbar_rect);
  }
  else {
    // direct children rendering
    control::draw(r, dst);
  }

  // debug rendering on top of offset texture
#if GM_DEBUG_UI
  draw_debug_frame(r, dst);
#endif
}

void box::clear_children()
{
  // take snapshot of childrens state under lock
  {
    lock_container(_children);
    // tmp copy dies in this scope
    std::vector<control*> children_copy(_children.get());
    // iterate over copy
    control_list::iterator it = children_copy.begin();
    for(; it != children_copy.end(); ++it) {
      ui::control * child = *it;
      if (child != _scroll) {
        ui::destroy(*it);
      }
    }
    // clear the source list
    _children.clear();
  }
  update_children();
}

void box::remove_child(control * c)
{
  // remove and update children
  control::remove_child(c);
  update_children();
}

void box::add_child(control* c)
{
  // box need to subscribe on children for book-keeping
  c->mouse_up += boost::bind( &box::on_child_click, this, _1 );
  c->hovered += boost::bind( &box::on_child_hover_changed, this, _1 );
  c->hover_lost += boost::bind( &box::on_child_hover_changed, this, _1 );
  c->mouse_wheel += boost::bind( &box::on_child_wheel, this, _1 );
  
  control::add_child(c);
  update_children();
}

void box::on_box_wheel(control * target)
{
  //SDL_Log("box::on_box_wheel");
  //const SDL_Event* sdl_ev = manager::instance()->current_event();
}

void box::on_child_wheel(control * target)
{
  //SDL_Log("box::on_child_wheel");
  //const SDL_Event* sdl_ev = manager::instance()->current_event();
}

void box::on_child_hover_changed(control * target)
{
  // rebuild on child hover change
  _dirty = true;
}

void box::on_child_click(control * target)
{
  // update selected control
  if (_selected_ctl != target) {
    switch_selection(target);
  }
  else if (_selected_ctl != NULL) {
    switch_selection(NULL);
  }
}

void box::load(const data & d)
{
  control::load(d);

  // optionally override whatever contol had set

  if (d.has_key("type")) {
    if (d["type"].is_value_number()) {
      uint32_t itype = d.get("type", (uint32_t)box_type::vbox);
      _type = (box_type)itype;
    }
    if (d["type"].is_value_string()) {
      std::string stype = d["type"].value<std::string>();
      if (stype == "vbox") {
        _type = box_type::vbox;
      }
      if (stype == "hbox") {
        _type = box_type::hbox;
      }
    }
  }

  if (d.has_key("v_align")) {
    if (d["v_align"].is_value_number()) {
      _va = (v_align)d["v_align"].value<uint32_t>();
    }
    if (d["v_align"].is_value_string()) {
      _va = valign_from_str(d["v_align"].value<std::string>());
    }
  }

  if (d.has_key("h_align")) {
    if (d["h_align"].is_value_number()) {
      _ha = (h_align)d["h_align"].value<uint32_t>();
    }
    if (d["h_align"].is_value_string()) {
      _ha = halign_from_str(d["h_align"].value<std::string>());
    }
  }

  if (d.has_key("scroll")) {
    if (d["scroll"].is_value_string()) {
      scrollbar_type stype = scrollbar_type::scrollbar_hidden;
      std::string scroll = d["scroll"].value<std::string>();
      if (scroll == "right")
        stype = scrollbar_type::scrollbar_right;
      if (scroll == "bottom")
        stype = scrollbar_type::scrollbar_bottom;
    
      int ssize = 16;
      if (d.has_key("scrollbar_size")) {
        if (d["scrollbar_size"].is_value_number()) {
          ssize = d["scrollbar_size"].value<int>();
        }
      }

      set_sbar(stype, ssize);
    }
  }

  if (d.has_key("padding")) {
    data::json * pad = d["padding"].value();
    if (json_is_array(pad)) {
      pad->unpack("[iiii]", &_pad.top, &_pad.left, &_pad.bottom, &_pad.right);
    }
    if (json_is_integer(pad)) {
      _pad = padding(pad->as<int>());
    }
  }

  if (d.has_key("gap") && d["gap"].is_value_integer()) {
    _gap = d["gap"].value<int>();
  }
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
    throw std::runtime_error("not implemented");
  }
#ifdef GM_DEBUG_UI
  SDL_Log("box::do_scroll - scrolled to %s by (%d,%d)",
    _scrolled_rect.tostr().c_str(), dx, dy);
#endif
}

std::string box::get_box_type_name() const
{
  switch (_type) {
  case box_type::none:
    return "none";
  case box_type::vbox:
    return "vbox";
  case box_type::hbox:
    return "hbox";
  default:
    throw std::runtime_error("Unkown box type");
  };
}

void box::update_children()
{
  lock_container(_children);

  // get available area for children
  // all box rect is available by default
  const theme::container_skin * sk = get_skin();
  rect area(0, 0, _pos.w, _pos.h);
  if (sk != theme::container_skin::dummy()) {
    area = theme::get_container_user_area(sk, area);
  }

#ifdef GM_DEBUG_UI
  SDL_Log("%s(%d) - type: %s, h_align: %s, v_align: %s, area: %s/%s",
    __METHOD_NAME__,
    _children.size(),
    get_box_type_name().c_str(),
    halign_to_str(_ha).c_str(),
    valign_to_str(_va).c_str(),
    area.tostr().c_str(),
    _pos.tostr().c_str());
#endif

  // minimum size
  _children_rect.w = _pad.left + _pad.right;
  _children_rect.h = _pad.top + _pad.bottom;

  // last positioned control 
  rect last_pos;
  control_list::iterator it = _children.begin();
  for(; it != _children.end(); ++it) {
    control* child = (*it);
    
    // skip locked children of this box
    if (child->locked())
      continue;
    // skip hidden
    if (!child->visible())
      continue;
    
    rect pos = child->pos();
    switch(_type) {

    case box::none:
      // take control size of it is less then area
      _children_rect.w = max(_pad.left + pos.x + pos.w + _pad.right, area.w);
      _children_rect.h = max(_pad.top + pos.y + pos.h + _pad.bottom, area.h);
      break;

    case box::vbox:
      {
        switch(_ha) {
        default:
          SDL_Log("%s - unsupported h_align value \"%d\" for vbox",
            __METHOD_NAME__, _ha);
          throw std::runtime_error("Unsupported h_align value for vbox");

        case h_align::left:
          // position child on the left side of the vbox
          pos.x = area.x + _pad.left;
          break;

        case h_align::right:
          // position child on the right side of the vbox
          pos.x = area.x + area.w - _pad.right - pos.w;
          break;

        case h_align::center:
          // position child in the center of the vbox
          pos.x = area.x + (area.w - pos.w) / 2;
          break;

        case h_align::expand:
          SDL_Log("FIXME");
          break;
        };

        switch(_va) {
        default:
          SDL_Log("%s - unsupported v_align value \"%d\" for vbox",
            __METHOD_NAME__, _ha);
          throw std::runtime_error("Unsupported v_align value for vbox");

        // ignore middle and fill in v_align for vbox
        // default to the top
        case v_align::middle:
        case v_align::fill:
        case v_align::top:
          // append from the top
          if (last_pos.y == 0)
            last_pos.y = area.y + _pad.top;
          pos.y = last_pos.y + last_pos.h + _gap;
          break;

        case v_align::bottom:
          // append at the bottom
          if (last_pos.y == 0)
            last_pos.y = area.y + area.h - _pad.bottom;
          pos.y = last_pos.y - _gap - pos.h;
          break;
        }; 
      }
      break;

    case box::hbox:
      {
        switch(_va) {
        default:
          SDL_Log("%s - unsupported v_align value \"%d\" for hbox",
            __METHOD_NAME__, _ha);
          throw std::runtime_error("Unsupported v_align value for hbox");

        case v_align::top:
          // position child at the top of the hbox
          pos.y = area.y + _pad.top;
          break;

        case v_align::bottom:
          // position child at the bottom of the hbox
          pos.y = area.y + area.h - _pad.bottom - pos.h;
          break;

        case v_align::middle:
          // position child in he center of the hbox
          pos.y = area.y + (area.h - pos.h) / 2;
          break;

        case v_align::fill:
          SDL_Log("FIXME");
          break;
        };
        
        switch(_ha) {
        default:
          SDL_Log("%s - unsupported h_align value \"%d\" for hbox",
            __METHOD_NAME__, _ha);
          throw std::runtime_error("Unsupported h_align value for hbox");

        // ignore center and expand in h_align for hbox
        // 
        case h_align::center:
        case h_align::expand:
        case h_align::left:
          if (last_pos.x == 0)
            last_pos.x = area.x + _pad.left;
          pos.x = last_pos.x + _gap;
          break;

        case h_align::right:
          if (last_pos.x == 0)
            last_pos.x = area.x + area.w - _pad.right;
          pos.x = last_pos.x - _gap - pos.w;
          break;
        };
      }
      break;
    default:
      SDL_Log("%s - unknown box type value \"%d\"", __METHOD_NAME__, _type);
      throw std::runtime_error("Unknown box type value");
      break;
    };

    // update child position (rect) & remember it
    child->set_pos(pos);
    last_pos = pos;

    // update total width & height
    _children_rect.w = max(_children_rect.w, last_pos.x + last_pos.w);
    _children_rect.h = max(_children_rect.h, last_pos.y + last_pos.h);
  }

  // normalize children rect
  _children_rect.w = max(_children_rect.w, _scrolled_rect.w);
  _children_rect.h = max(_children_rect.h, _scrolled_rect.h);
  

  if (_scroll) {
    it = find_child(_scroll);
    if (it != _children.end())
      _children.erase(it);
    _children.push_back(_scroll);
  }
  
  // rebuild on children add/remove/show/hide/reorder
  _dirty = true;
}

/**
  UI Panel box
  */

panel::panel(const rect & pos, 
             const panel_style & ps, 
             const box_type & t,
             const h_align & ha, 
             const v_align & va,
             const padding & pad,
             const int & gap):
  box(pos, t, ha, va, pad, gap),
  _ps(ps),
  _color_back(get_skin()->color_back)
{
}

panel::~panel()
{
}

void panel::load(const data & d)
{
  if (d.has_key("color_back")) {
    if (d["color_back"].is_value_array(4)) {
      _color_back = d["color_back"].value<color>();
    }
    if (d["color_back"].is_value_string()) {
      _color_back = color::from_string(d["color_back"].value<std::string>());
    }
  }

  if (d.has_key("panel_style")) {
    if (d["panel_style"].is_value_string()) {
      std::string pstyle = d["panel_style"].value<std::string>();
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
    // panel is dialog by default
    set_panel_style(panel_style::dialog);
  }

  box::load(d);
}

void panel::draw(SDL_Renderer * r, const rect & dst)
{
  // render background
  const theme::container_skin * s = get_skin();
  rect user_area = theme::get_container_user_area(s, dst);
  _color_back.apply(r);
  SDL_RenderFillRect(r, &user_area);

  // render box content
  box::draw(r, dst);

  // render box frame
  current_theme().draw_container_skin(s, r, dst);
}

} //namespace ui