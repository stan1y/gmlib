#include "GMUICombo.h"

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
  _max_box_height(max_box_height),
  _item_height(item_height),
  _expand_on_hover(false),
  _area(new combo::area(rect(pos.x, pos.y + pos.h, pos.w, 1), get_skin()->color_back))
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
    SDLEx_LogError("combo::get_item - invalid item index %d", item);
    throw std::exception("invalid combo item index");
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
    // enable scrolls
    _area->set_sbar(box::scrollbar_right, 10);
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
    SDLEx_LogError("combo::delete_item - invalid item index %d", item);
    throw std::exception("invalid combo item index");
  }

  control * child = _area->get_child_at_index(item);
  remove_child(child);
  resize_area();
}
  
int combo::find_text(const std::string & text)
{
  return -1;
}

void combo::load(const data & d)
{
  text_input::load(d);

  if (d.has_key("max_box_height")) {
    _max_box_height = d["max_box_height"].value<int>();
  }
  if (d.has_key("area_color")) {
    if (d["area_color"].is_value_string()) {
      std::string sclr = d["area_color"].value<std::string>();
      _area->set_back_color(color::from_string(sclr));
    }
    if (d["area_color"].is_value_array()) {
      _area->set_back_color(d["area_color"].value<color>());
    }
  }

  if (d.has_key("items")) {
    data items = d["items"].value();
    if (items.is_array()) {
      for(size_t i = 0; i < items.length(); ++i) {
        data itm = items[i];
        if (itm.has_key("text")) {
          size_t item = add_item(itm["text"].value<std::string>());
          label * lbl = get_item(item);
          if (itm.has_key("icon"))
            lbl->set_icon(itm["icon"].value<std::string>());
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

void combo::render(SDL_Renderer* r, const rect & dst)
{
  text_input::render(r, dst);
}

void combo::area::render(SDL_Renderer* r, const rect & dst)
{
  _back.apply(r);
  SDL_RenderFillRect(r, &dst);
  
  box::render(r, dst);
}

}; //namespace ui