#include "combo.h"

namespace ui {

combo::combo(const rect & pos,
             const int max_box_height,
             const int item_height,
             const input_validation & filter,
             const icon_pos & ip,
             const h_align & ha, 
             const v_align & va,
             const padding & pad):
  text_input(pos, filter, ip, ha, va, pad),
  _expand_on_hover(false),
  _max_box_height(max_box_height),
  _item_height(item_height),
  _area(new combo::area(rect(pos.x, pos.y + pos.h, pos.w, 1)))
{ 
  _area->set_identifier( identifier() + std::string("_area") );
  _area->set_visible(false);
  set_readonly(true);
  
  focused += boost::bind(&combo::on_focused, this, _1);
  focus_lost += boost::bind(&combo::on_focus_lost, this, _1);
  hovered += boost::bind(&combo::on_hover, this, _1);
  hover_lost += boost::bind(&combo::on_hover_lost, this, _1);
}

combo::~combo()
{
}

label * combo::get_item(size_t item)
{
  if (item >= _area->children().size()) {
    SDL_Log("combo::get_item - invalid item index %zu", item);
    throw std::runtime_error("invalid combo item index");
  }

  return dynamic_cast<label*>(_area->get_child_at_index(item));
}

void combo::resize_area()
{
  control_list::const_iterator it = _area->children().begin();
  const padding & pad = get_padding();
  int area_len = pad.top + pad.bottom;
  for(; it != _area->children().end(); ++it) {
    area_len += (*it)->pos().h;
  }
  rect area_pos = _area->pos();
  if (area_len < _max_box_height) {
    // still can resize area
    area_pos.h = area_len;
    _area->set_pos(area_pos);
    SDL_Log("area size %d, %d", _area->pos().w, _area->pos().h);
  }
  else {
    // enable vscroll for combo items area
    _area->set_scroll_type(box::scroll_type::scrollbar_vertical, 10);
  }
}
  
size_t combo::add_item(const std::string & text, const padding & pad, const h_align & ha, const v_align & va)
{
  label * item = new label(rect(0, 0, _pos.w, _item_height), icon_left, ha, va, pad);
  item->set_text(text);
  item->set_font(get_font());
  item->set_idle_color(get_idle_color());
  item->mouse_up += boost::bind(&combo::on_item_mouseup, this, _1);
  
  _area->add_child(item);
  resize_area();

  return _area->children().size() - 1;
}

void combo::delete_item(size_t item)
{
  if (item >= _area->children().size()) {
    SDL_Log("combo::delete_item - invalid item index %zu", item);
    throw std::runtime_error("invalid combo item index");
  }

  control * child = _area->get_child_at_index(item);
  remove_child(child);
  resize_area();
}
  
int combo::find_text(const std::string & text)
{
  return -1;
}

void combo::load(const json & d)
{
  text_input::load(d);

  if (d.find("max_box_height") != d.end()) {
    _max_box_height = d["max_box_height"];
  }

  if (d.find("items") != d.end()) {
    json::const_iterator itm = d["item"].begin();
    for(; itm != d["item"].end(); ++itm) {
      if (itm->is_object() && itm->find("text") != itm->end()) {
        size_t i = add_item( (*itm)["text"] );
        if (itm->find("icon") != itm->end()) {
          label * lbl = get_item(i);
          lbl->set_icon( (*itm)["icon"] );
        }
      }
    }
  }
}

void combo::on_hover(control * target)
{
  if (_expand_on_hover) {
    rect abs_pos = get_absolute_pos();
    rect area_pos = _area->pos();
    _area->set_pos( rect(abs_pos.x, abs_pos.y + _pos.h, area_pos.w, area_pos.h) );
    _area->set_visible(true);
    ui::pop_front(_area);
  }
}

void combo::on_hover_lost(control * target)
{
  if (_expand_on_hover) {
    _area->set_visible(false);
  }
}

void combo::on_focused(control * target)
{
  if (!_expand_on_hover) {
    rect abs_pos = get_absolute_pos();
    rect area_pos = _area->pos();
    _area->set_pos( rect(abs_pos.x, abs_pos.y + _pos.h, area_pos.w, area_pos.h) );
    _area->set_visible(true);
    ui::pop_front(_area);
  }
}

void combo::on_focus_lost(control * target)
{
  if (!_expand_on_hover) {
    _area->set_visible(false);
  }
}

void combo::on_item_mouseup(control * target)
{
}

void combo::update()
{
  text_input::update();
}

void combo::draw(SDL_Renderer* r, const rect & dst)
{
  text_input::draw(r, dst);
}

void combo::area::draw(SDL_Renderer* r, const rect & dst)
{
	draw_frame(r, 0, 0, dst, true, true);
  box::draw(r, dst);
}

}; //namespace ui
