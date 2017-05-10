#include "label.h"

namespace ui {

label::label(const rect & pos,
             const icon_pos & icon,
             const h_align & ha,
             const v_align & va,
             const padding & pad,
             const int gap):
  control(pos),
  _pad(pad), _ha(ha), _va(va),
  _icon_pos(icon), _icon_gap(gap),
  _icon_tx(nullptr),
  _style(get_type_name()),
  _font(ui::manager::instance()->get_font(_style)),
  _font_style(font_style::blended),
  _color_idle(ui::manager::instance()->get_idle_color(_style)),
  _color_highlight(ui::manager::instance()->get_highlight_color(_style)),
  _color_back(ui::manager::instance()->get_back_color(_style)),
  _dirty(true),
  _sticky(false),
  _stuck(false),
  _hovered(false),
  _highlight_on_hover(false),
  _focused(false),
  _highlight_on_focus(false),
  _pressed(false),
  _animating(false),
  _alpha(255),
  _alpha_step(0)
{
  hovered += boost::bind( &label::on_hovered, this, _1 );
  hover_lost += boost::bind( &label::on_hover_lost, this, _1 );
  focused += boost::bind( &label::on_focused, this, _1 );
  focus_lost += boost::bind( &label::on_focus_lost, this, _1 );
  mouse_up += boost::bind( &label::on_mouse_up, this, _1 );
  mouse_down += boost::bind( &label::on_mouse_down, this, _1 );
}

label::~label()
{
  if (_icon_tx != nullptr)
    delete _icon_tx;
}

label::font_style label::font_style_from_str(const std::string & s)
{
  if (s == "solid")
    return font_style::solid;
  else if (s == "blended")
    return font_style::blended;
  else
    throw std::runtime_error("Failed to convert string to font_style");
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

  if (_icon_tx != nullptr)
    delete _icon_tx;
  _icon_tx = new texture(NULL, icon);
}

void label::set_icon(texture* icon)
{
  if (_animating) {
    // ignore icon change during animation
    return;
  }
  _icon_file.clear();

  if (_icon_tx != nullptr)
    delete _icon_tx;
  _icon_tx = icon;
}

void label::set_style(const std::string &st)
{
  _style = st;
  _font = ui::manager::instance()->get_font(_style);
  _font_style = font_style::blended;
  _color_idle = ui::manager::instance()->get_idle_color(_style);
  _color_highlight = ui::manager::instance()->get_highlight_color(_style);
  _color_back = ui::manager::instance()->get_back_color(_style);
}

void label::on_hovered(control * target)
{
  _hovered = true;
  _dirty = true;
}

void label::on_hover_lost(control * target)
{
  _hovered = false;
  _pressed = false;
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
  _pressed = false;
  _dirty = true;
}

void label::on_mouse_up(control * target)
{
  _pressed = false;
  _dirty = true;
  if (_sticky)
    _stuck = !_stuck;
}

void label::on_mouse_down(control * target)
{
  _pressed = true;
  _dirty = true;
}

void label::load(const json & d)
{
  if (d.find("text") != d.end())
    _text = d["text"];
  if (d.find("icon") != d.end())
    _icon_file = d["icon"];

  if (d.find("v_align") != d.end()) {
    if (d["v_align"].is_number()) {
      _va = (v_align)d["v_align"].get<uint32_t>();
    }
    if (d["v_align"].is_string()) {
      _va = valign_from_str(d["v_align"]);
    }
  }

  if (d.find("h_align") != d.end()) {
    if (d["h_align"].is_number()) {
      _ha = (h_align)d["h_align"].get<uint32_t>();
    }
    if (d["h_align"].is_string()) {
      _ha = halign_from_str(d["h_align"]);
    }
  }

  if (d.find("icon_pos") != d.end()) {
    if (d["icon_pos"].is_number()) {
      _icon_pos = (icon_pos)d["icon_pos"];
    }
    if (d["icon_pos"].is_string()) {
      std::string ip = d["icon_pos"];
      if (ip == "left") {
        _icon_pos = icon_pos::icon_left;
      }
      if (ip == "right") {
        _icon_pos = icon_pos::icon_right;
      }
    }
  }

  if (d.find("icon_gap") != d.end() && d["icon_gap"].is_number()) {
    _icon_gap = d["icon_gap"];
  }

  if (d.find("font") != d.end() && d["font"].is_array()) {
    _font = manager::load_font(d["font"].at(0), d["font"].at(1));
    if (d.find("font_style") != d.end())
      _font_style = font_style_from_str(d["font_style"]);
  }

  if (d.find("color_back") != d.end()) {
    _color_back = color::from_json(d["color_back"]);
  }

  if (d.find("color_idle") != d.end()) {
    _color_idle = color::from_json(d["color_idle"]);
  }

  if (d.find("color_highlight") != d.end()) {
    _color_highlight = color::from_json(d["color_highlight"]);
  }

  if (d.find("padding") != d.end()) {
    if (d["padding"].is_array()) {
      _pad = padding(
          d["padding"].at(0),
          d["padding"].at(1),
          d["padding"].at(2),
          d["padding"].at(3));
    }
    if (d["padding"].is_number())
      _pad = padding(d["padding"].get<int>());
  }

  if (d.find("icon_gap") != d.end() && d["icon_gap"].is_number())
    _icon_gap = d["icon_gap"];

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

void label::draw(SDL_Renderer * r, const rect & dst)
{
  if (_dirty)
    paint(r);

  if (_icon_tx != nullptr)
    _icon_tx->render(r, dst.topleft() + _icon_offset);

  _text_tx.set_alpha(int32_to_uint8(_alpha));
  _text_tx.render(r, dst.topleft() + _text_offset);

  control::draw(r, dst);
}

void label::paint_text(texture & tx, const std::string & text, const ttf_font * fnt, const color & clr)
{
  SDL_Surface *s;
  if (_font_style == font_style::blended)
    s = fnt->print_blended(text.length() > 0 ? text : " ", clr);
  else
    s = fnt->print_solid(text.length() > 0 ? text : " ", clr);
  tx.set_surface(s);
  SDL_FreeSurface(s);
}

void label::paint(SDL_Renderer * r)
{
  if (!_dirty) return;

  
  color clr = _color_idle;
  if (is_focused() && _highlight_on_focus) {
    clr = _color_highlight;
  }
  else if (is_hovered() && _highlight_on_hover) {
    clr = _color_highlight;
  }
  paint_text(_text_tx, _text, _font, clr);

  // load icon as resource only if given
  if (_icon_file.size() > 0 && _icon_tx == nullptr) {
    _icon_tx = new texture();
    _icon_tx->load(_icon_file);
  }
  else {
    // assume that _icon_tx contains valid icon image
    // setup by the corresponding call to label::set_icon
  }
  
  // reset dirty flag
  _dirty = false;

  int total_avail_w = _pos.w - (_pad.left + _pad.right);
  int total_avail_h = _pos.h - (_pad.top + _pad.bottom);
  int icon_w = (_icon_tx != nullptr ? _icon_tx->width() : 0);
  int icon_h = (_icon_tx != nullptr ? _icon_tx->height() : 0);
  int total_label_w = icon_w + _text_tx.width();

  // horizontal alignment
  if (_ha == h_align::center) {
    if (_icon_pos == icon_pos::icon_left) {
      // icon first
      _icon_offset.x = _pad.left + (total_avail_w - total_label_w) / 2;
      _text_offset.x = _icon_offset.x + icon_w + _icon_gap;
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
      _text_offset.x = _icon_offset.x + icon_w + _icon_gap;
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
      _icon_offset.x = _text_offset.x - icon_w - _icon_gap;
    }
    if (_icon_pos == icon_pos::icon_right) {
      // text first
      _icon_offset.x = _pos.w - icon_w - _pad.right;
      _text_offset.x = _icon_offset.x - _text_tx.width() - _icon_gap;
    }
  }

  // vertical alignment
  if (_va == v_align::middle) {
    _icon_offset.y = _pad.top + (total_avail_h - icon_h) / 2;
    _text_offset.y = _pad.top + (total_avail_h - _text_tx.height()) / 2;
  }
  if (_va == v_align::top) {
    _icon_offset.y = _pad.top;
    _text_offset.y = _pad.top;
  }
  if (_va == v_align::bottom) {
    _icon_offset.y = _pos.h - icon_h - _pad.bottom;
    _text_offset.y = _pos.h - _text_tx.height() - _pad.bottom;
  }
}

} //namespace ui
