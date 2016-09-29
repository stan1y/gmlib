#include "GMUILabel.h"

namespace ui {

label::label(rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  _pad(pad), _ip(ip), _ha(ha), _va(va), _icon_gap(0),
  _font_text(get_frame()->font_text),
  _font_style(font_style::blended),
  _color_idle(get_frame()->color_idle),
  _color_back(get_frame()->color_back),
  _color_highlight(get_frame()->color_highlight),
  _hovered(false),
  _focused(false),
  _dirty(true),
  _animating(false),
  _highlight_on_hover(false),
  _highlight_on_focus(false),
  _alpha(255),
  control("label", pos)
{
  hovered += boost::bind( &label::on_hovered, this, _1 );
  hover_lost += boost::bind( &label::on_hover_lost, this, _1 );
  focused += boost::bind( &label::on_focused, this, _1 );
  focus_lost += boost::bind( &label::on_focus_lost, this, _1 );
}

label::label(const std::string & type_name, rect pos, margin pad, icon_pos ip, h_align ha, v_align va):
  _pad(pad), _ip(ip), _ha(ha), _va(va), _icon_gap(0),
  _font_text(get_frame()->font_text),
  _font_style(font_style::blended),
  _color_idle(get_frame()->color_idle),
  _color_back(get_frame()->color_back),
  _color_highlight(get_frame()->color_highlight),
  _hovered(false),
  _focused(false),
  _dirty(true),
  _animating(false),
  _highlight_on_hover(false),
  _highlight_on_focus(false),
  _alpha(255),
  control(type_name, pos)
{
  hovered += boost::bind( &label::on_hovered, this, _1 );
  hover_lost += boost::bind( &label::on_hover_lost, this, _1 );
  focused += boost::bind( &label::on_focused, this, _1 );
  focus_lost += boost::bind( &label::on_focus_lost, this, _1 );
}

label::~label()
{
}

void label::set_text(const std::string& txt)
{
  if (_animating) {
    // ignore text change during animation
    return;
  }
  _text = txt;
  _dirty = true;
}

void label::set_icon(const std::string& icon_file)
{
  if (_animating) {
    // ignore icon change during animation
    return;
  }
  _icon_file = icon_file;
  _dirty = true;
}

void label::set_icon(SDL_Surface* icon)
{
  if (_animating) {
    // ignore icon change during animation
    return;
  }
  _icon_file.clear();
  _icon_tx.convert_surface(icon);
}

void label::set_icon(SDL_Texture* icon)
{
  if (_animating) {
    // ignore icon change during animation
    return;
  }
  _icon_file.clear();
  _icon_tx = texture(icon);
}

void label::on_hovered(control * target)
{
  _hovered = true;
  _dirty = true;
}

void label::on_hover_lost(control * target)
{
  _hovered = false;
  _dirty = true;
}

void label::on_focused(control * target)
{
  _focused = true;
  _dirty = true;
}

void label::on_focus_lost(control * target)
{
  _focused = false;
  _dirty = true;
}

void label::load(const data & d)
{
  _text = d.get("text", "");
  _icon_file = d.get("icon", "");

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

  if (d.has_key("font_text") && d.has_subkey("font_text.face") && d.has_subkey("font_text.size")) {
    std::string font_style = "blended";
    std::stringstream res_id;
    res_id << d["font_text"]["face"].as<std::string>() \
           << ":" \
           << d["font_text"]["size"].as<size_t>();
    _font_text = resources::get_font(res_id.str());
  }
  else {
    // use font from theme
    _font_text = get_frame()->font_text;
  }

  if (d.has_key("color_back")) {
    if (d["color_back"].is_string()) {
      std::string sclr = d["color_back"].as<std::string>();
      _color_back = color::from_string(sclr);
    }
    if (d["color_back"].is_array()) {
      _color_back = d["color_back"].as<color>();
    }
  }

  if (d.has_key("color_idle")) {
    if (d["color_idle"].is_string()) {
      std::string sclr = d["color_idle"].as<std::string>();
      _color_idle = color::from_string(sclr);
    }
    if (d["color_idle"].is_array()) {
      _color_idle = d["color_idle"].as<color>();
    }
  }

  if (d.has_key("color_highlight")) {
    if (d["color_highlight"].is_string()) {
      std::string sclr = d["color_highlight"].as<std::string>();
      _color_highlight = color::from_string(sclr);
    }
    if (d["color_highlight"].is_array()) {
      _color_highlight = d["color_highlight"].as<color>();
    }
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

void label::update()
{
  if (_animating) {
    _alpha += _alpha_step;
    if (_alpha <= 0 || _alpha >= 255 ) {
      _animating = false;
      if (_alpha <= 0) {
        SDL_Log("label::update - fade out done");
        set_visible(false);
        return;
      }
      else {
        SDL_Log("label::update - fade in done");
        return;
      }
    }

    if (_alpha < 0 ) _alpha = 0;
    if (_alpha > 255) _alpha = 255;
    
    SDL_Log("label::update - fade in %d/%d", _alpha, _alpha_step);
  }
}

void label::toggle(int step)
{
  if (_animating) {
    return;
  }
  // check current state
  if (_visible) {
    SDL_Log("label::toggle - fade out");
    _alpha = 255;
    _alpha_step = -step;
  }
  else {
    SDL_Log("label::toggle - fade in");
    _alpha = 0;
    _alpha_step = step;
  }

  set_visible(true);
  _animating = true;
}

void label::render(SDL_Renderer * r, const rect & dst)
{
  if (_dirty) paint(r);
  _icon_tx.render(r, dst.topleft() + _icon_offset);
  _text_tx.set_alpha(int32_to_uint8(_alpha));
  _text_tx.render(r, dst.topleft() + _text_offset);

  control::render(r, dst);
}

void label::paint(SDL_Renderer * r)
{
  if (!_dirty) return;
  // render text, it can be empty string
  color clr = _color_idle;
  if (is_focused() && _highlight_on_focus) {
    clr = _color_highlight;
  }
  else if (is_hovered() && _highlight_on_hover) {
    clr = _color_highlight;
  }

  if (_font_style == font_style::blended)
    _text_tx.load_text_blended(_text.length() > 0 ? _text : " ", _font_text, clr);
  else
    _text_tx.load_text_solid(_text.length() > 0 ? _text : " ", _font_text, clr);

  // load icon as resource only if given
  if (_icon_file.size() > 0) {
    _icon_tx.load(_icon_file);
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