#include "GMUILabel.h"

namespace ui {

label::label(const rect & pos,
             const icon_pos & icon,
             const h_align & ha,
             const v_align & va,
             const padding & pad,
             const int gap):
  _pad(pad), _icon_pos(icon), _ha(ha), _va(va), _icon_gap(gap),
  _font_text(get_skin()->font_text),
  _font_style(font_style::blended),
  _color_idle(get_skin()->color_idle),
  _color_back(get_skin()->color_back),
  _color_highlight(get_skin()->color_highlight),
  _hovered(false),
  _focused(false),
  _dirty(true),
  _animating(false),
  _highlight_on_hover(false),
  _highlight_on_focus(false),
  _alpha(255),
  control(pos)
{
  hovered += boost::bind( &label::on_hovered, this, _1 );
  hover_lost += boost::bind( &label::on_hover_lost, this, _1 );
  focused += boost::bind( &label::on_focused, this, _1 );
  focus_lost += boost::bind( &label::on_focus_lost, this, _1 );
}

label::~label()
{
}

label::font_style label::font_style_from_str(const std::string & s)
{
  if (s == "solid")
    return font_style::solid;
  else if (s == "blended")
    return font_style::blended;
  else
    throw std::exception("Failed to convert string to font_style");
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

  if (d.has_key("icon_pos")) {
    if (d["icon_pos"].is_value_number()) {
      _icon_pos = (icon_pos)d["icon_pos"].value<uint32_t>();
    }
    if (d["icon_pos"].is_value_string()) {
      std::string ip = d["icon_pos"].value<std::string>();
      if (ip == "left") {
        _icon_pos = icon_pos::icon_left;
      }
      if (ip == "right") {
        _icon_pos = icon_pos::icon_right;
      }
    }
  }

  if (d.has_key("icon_gap") && d["icon_gap"].is_value_number()) {
    _icon_gap = d["icon_gap"].value<uint32_t>();
  }

  if (d.has_key("font_text") && d.has_subkey("font_text.face") && d.has_subkey("font_text.size")) {
    _font_text = resources::get_font(d["font_text.face"].value<std::string>(),
                                    d["font_text.size"].value<size_t>());
    if (d.has_subkey("font_text.style"))
      _font_style = font_style_from_str(d["font_text.style"].value<std::string>());
  }

  if (d.has_key("color_back")) {
    if (d["color_back"].is_value_string()) {
      std::string sclr = d["color_back"].value<std::string>();
      _color_back = color::from_string(sclr);
    }
    if (d["color_back"].is_value_array()) {
      _color_back = d["color_back"].value<color>();
    }
  }

  if (d.has_key("color_idle")) {
    if (d["color_idle"].is_value_string()) {
      std::string sclr = d["color_idle"].value<std::string>();
      _color_idle = color::from_string(sclr);
    }
    if (d["color_idle"].is_value_array()) {
      _color_idle = d["color_idle"].value<color>();
    }
  }

  if (d.has_key("color_highlight")) {
    if (d["color_highlight"].is_value_string()) {
      std::string sclr = d["color_highlight"].value<std::string>();
      _color_highlight = color::from_string(sclr);
    }
    if (d["color_highlight"].is_value_array()) {
      _color_highlight = d["color_highlight"].value<color>();
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
  if (d.has_key("icon_gap")) {
    _icon_gap = d.get("icon_gap", 0);
  }

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
    if (_icon_pos == icon_pos::icon_left) {
      // icon first
      _icon_offset.x = _pad.left + (total_avail_w - total_label_w) / 2;
      _text_offset.x = _icon_offset.x + _icon_tx.width() + _icon_gap;
    }
    if (_icon_pos == icon_pos::icon_right) {
      // text first
      _text_offset.x = _pad.left + (total_avail_w - total_label_w) / 2;
      _icon_offset.x = _text_offset.x + _text_tx.width() + _icon_gap;
    }
  }
  if (_ha == h_align::left) {
    if (_icon_pos == icon_pos::icon_left) {
      // icon first
      _icon_offset.x = _pad.left;
      _text_offset.x = _icon_offset.x + _icon_tx.width() + _icon_gap;
    }
    if (_icon_pos == icon_pos::icon_right) {
      // text first
      _text_offset.x = _pad.left;
      _icon_offset.x = _text_offset.x + _text_tx.width() + _icon_gap;
    }
  }
  if (_ha == h_align::right) {
    if (_icon_pos == icon_pos::icon_left) {
      // icon first
      _text_offset.x = _pos.w - _text_tx.width() - _pad.right;
      _icon_offset.x = _text_offset.x -_icon_tx.width() - _icon_gap;
    }
    if (_icon_pos == icon_pos::icon_right) {
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