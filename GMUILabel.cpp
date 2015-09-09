#include "GMUILabel.h"

namespace ui {

label::label(rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  _pad(pad), _ip(ip), _ha(ha), _va(va), _icon_gap(0),
  _font(&UI_GetTheme().font_text_norm),
  _font_color(UI_GetTheme().color_text),
  _dirty(true),
  control(pos)
{
}

label::~label()
{
}


void label::set_text(const std::string& txt)
{
  _text = txt;
  _dirty = true;
}

void label::set_icon(const std::string& res)
{
  _icon_res = res;
  _dirty = true;
}

void label::set_icon(SDL_Surface* icon)
{
  _icon_res.clear();
  _icon_tx.load_surface(icon, SDL_TEXTUREACCESS_STATIC, SDL_BLENDMODE_BLEND);
}

void label::set_icon(SDL_Texture* icon)
{
  _icon_res.clear();
  _icon_tx = texture(icon);
}

void label::load(data & d)
{
  _text = d.get("text", "");
  _icon_res = d.get("icon", "");

  if (d.has_key("v_align")) {
    if (d["v_align"].is_number()) {
      _va = (v_align)d["v_align"].as<uint32_t>();
    }
    if (d["v_align"].is_string()) {
      std::string va = d["v_align"].as<std::string>();
      if (va == "top") {
        _va = v_align::top;
      }
      if (va == "bottom") {
        _va = v_align::bottom;
      }
      if (va == "middle") {
        _va = v_align::middle;
      }
    }
  }
  else {
    _va = v_align::middle;
  }

  if (d.has_key("h_align")) {
    if (d["h_align"].is_number()) {
      _ha = (h_align)d["h_align"].as<uint32_t>();
    }
    if (d["h_align"].is_string()) {
      std::string ha = d["h_align"].as<std::string>();
      if (ha == "left") {
        _ha = h_align::left;
      }
      if (ha == "right") {
        _ha = h_align::right;
      }
      if (ha == "center") {
        _ha = h_align::center;
      }
    }
  }
  else {
    _ha = h_align::center;
  }

  if (d.has_key("icon_pos")) {
    if (d["icon_pos"].is_number()) {
      _ip = (icon_pos)d["icon_pos"].as<uint32_t>();
    }
    if (d["icon_pos"].is_string()) {
      std::string ip = d["icon_pos"].as<std::string>();
      if (ip == "left") {
        _ip = icon_pos::icon_left;
      }
      if (ip == "right") {
        _ip = icon_pos::icon_right;
      }
    }
  }
  else {
    _ip = icon_pos::icon_left;
  }

  if (d.has_key("icon_gap") && d["icon_gap"].is_number()) {
    _icon_gap = d["icon_gap"].as<uint32_t>();
  }

  if (d.has_key("font")) {
    std::string sfont = d["font"].as<std::string>();
    if (sfont == "normal") 
      _font = &UI_GetTheme().font_text_norm;
    if (sfont == "bold")
      _font = &UI_GetTheme().font_text_bold;
    if (sfont == "ital")
      _font = &UI_GetTheme().font_text_ital;
  }

  if (d.has_key("margin")) {
    data margin = d["margin"];
    if (margin.is_array()) {
      margin.unpack("[iiii]", &_pad.top, &_pad.left, &_pad.bottom, &_pad.right);
    }
    if (margin.is_number()) {
      _pad = margin_t(margin.as<uint32_t>());
    }
  }
  _icon_gap = d.get("icon_gap", 0);

  control::load(d);
}

void label::render(SDL_Renderer * r, const rect & dst)
{
  if (_dirty) paint(r);
  _icon_tx.render(r, dst.topleft() + _icon_offset);
  _text_tx.render(r, dst.topleft() + _text_offset);

  control::render(r, dst);
}

void label::paint(SDL_Renderer * r)
{
  if (!_dirty) return;
  const theme & th = manager::instance()->get_theme();
  // render text, it can be empty string
  _font->print(_text_tx, _text.length() > 0 ? _text : " ", _font_color);
  // load icon as resource only if given
  if (_icon_res.size() > 0) {
    _icon_tx.load(_icon_res);
  }
  else {
    // assume that _icon_tx contains valid icon image
    // setup by the corresponding call to label::set_icon
  }
  
  // reset dirty flag
  _dirty = false;

  int total_avail_w = _pos.w - (_pad.left + _pad.right);
  int total_avail_h = _pos.h - (_pad.top + _pad.bottom);
  int total_label_w = _icon_tx.width() + _text_tx.width();

  // horizontal alignment
  if (_ha == h_align::center) {
    if (_ip == icon_pos::icon_left) {
      // icon first
      _icon_offset.x = _pad.left + (total_avail_w - total_label_w) / 2;
      _text_offset.x = _icon_offset.x + _icon_tx.width() + _icon_gap;
    }
    if (_ip == icon_pos::icon_right) {
      // text first
      _text_offset.x = _pad.left + (total_avail_w - total_label_w) / 2;
      _icon_offset.x = _text_offset.x + _text_tx.width() + _icon_gap;
    }
  }
  if (_ha == h_align::left) {
    if (_ip == icon_pos::icon_left) {
      // icon first
      _icon_offset.x = _pad.left;
      _text_offset.x = _icon_offset.x + _icon_tx.width() + _icon_gap;
    }
    if (_ip == icon_pos::icon_right) {
      // text first
      _text_offset.x = _pad.left;
      _icon_offset.x = _text_offset.x + _text_tx.width() + _icon_gap;
    }
  }
  if (_ha == h_align::right) {
    if (_ip == icon_pos::icon_left) {
      // icon first
      _text_offset.x = _pos.w - _text_tx.width() - _pad.right;
      _icon_offset.x = _text_offset.x -_icon_tx.width() - _icon_gap;
    }
    if (_ip == icon_pos::icon_right) {
      // text first
      _icon_offset.x = _pos.w - _icon_tx.width() - _pad.right;
      _text_offset.x = _icon_offset.x - _text_tx.width() - _icon_gap;
    }
  }

  // vertical alignment
  if (_va == v_align::middle) {
    _icon_offset.y = _pad.top + (total_avail_h - _icon_tx.height()) / 2;
    _text_offset.y = _pad.top + (total_avail_h - _text_tx.height()) / 2;
  }
  if (_va == v_align::top) {
    _icon_offset.y = _pad.top;
    _text_offset.y = _pad.top;
  }
  if (_va == v_align::bottom) {
    _icon_offset.y = _pos.h - _icon_tx.height() - _pad.bottom;
    _text_offset.y = _pos.h - _text_tx.height() - _pad.bottom;
  }
}

} //namespace ui