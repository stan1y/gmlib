#include "GMUICombo.h"

namespace ui {

combo::combo(rect pos,
            int area_maxlen, 
            input_validation valid,
            margin pad,
            icon_pos ip,
            h_align ha, 
            v_align va):
  text_input(pos, valid, pad, ip, ha, va),
  _area_maxlen(area_maxlen),
  _expand_on_hover(false),
  _area(new combo::area(rect(pos.x, pos.y + pos.h, pos.w, 1), UI_GetTheme().color_back ))
{ 
  _area->set_identifier( identifier() + std::string("_area") );
  _area->set_visible(false);
  
  focus += boost::bind(&combo::on_focus, this, _1);
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

  return dynamic_cast<label*>(_area->children()[item]);
}

void combo::resize_area()
{
  control_list::const_iterator it = _area->children().begin();
  int area_len = _pad.top + _pad.bottom;
  for(; it != _area->children().end(); ++it) {
    area_len += (*it)->pos().h;
  }
  rect area_pos = _area->pos();
  if (area_len < _area_maxlen) {
    // still can resize area
    area_pos.h = area_len;
    _area->set_pos(area_pos);
    SDL_Log("area size %d, %d", _area->pos().w, _area->pos().h);
  }
  else {
    // enable scrolls
    _area->set_sbar(box::scrollbar_type::right, 10);
  }
}
  
size_t combo::add_item(const std::string & text, margin pad, h_align ha, v_align va)
{
  label * lbl = new label(rect(0, 0, _pos.w, 20), pad, icon_left, ha, va);
  lbl->set_text(text);
  lbl->set_font(get_font());
  lbl->set_font_color(get_font_color());
  lbl->mouse_up += boost::bind(&combo::on_item_mouseup, this, _1);
  
  _area->add_child(lbl);
  size_t item = _area->children().size() - 1;
  
  resize_area();
  
  return item;
}

void combo::delete_item(size_t item)
{
  if (item >= _area->children().size()) {
    SDLEx_LogError("combo::delete_item - invalid item index %d", item);
    throw std::exception("invalid combo item index");
  }

  control * child = _area->children()[item];
  _area->remove_child(child);
  ui::destroy(child);

  resize_area();
}
  
int combo::find_text(const std::string & text)
{
  return -1;
}

void combo::load(const data & d)
{
  text_input::load(d);

  if (d.has_key("area_maxlen")) {
    _area_maxlen = d["area_maxlen"].as<int>();
  }
  if (d.has_key("area_color")) {
    if (d["area_color"].is_string()) {
      std::string sclr = d["area_color"].as<std::string>();
      _area->set_back_color(color::from_string(sclr));
    }
    if (d["area_color"].is_array()) {
      _area->set_back_color(d["area_color"].as<color>());
    }
  }

  if (d.has_key("items")) {
    data items = d["items"];
    if (items.is_array()) {
      for(size_t i = 0; i < items.length(); ++i) {
        data itm = items[i];
        if (itm.has_key("text")) {
          size_t item = add_item(itm["text"].as<std::string>());
          label * lbl = get_item(item);
          if (itm.has_key("icon"))
            lbl->set_icon(itm["icon"].as<std::string>());
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

void combo::on_focus(control * target)
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