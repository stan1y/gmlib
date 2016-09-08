#include "RESLib.h"
#include "GMUITheme.h"
#include "GMData.h"

#include <boost/filesystem.hpp>
using namespace boost;

std::string UI_GetThemeRoot(const std::string & theme_path)
{
  filesystem::path root(resources::root_path());
  std::string t(theme_path);
  root /= t;
  return root.string();
}

namespace ui {

theme::font::font(const std::string & file_path, size_t pt_size, font_style st)
{
  load(file_path, pt_size);
  _fs = st;
}

void theme::font::print(texture & target,  const std::string & text, const color & c) const
{
  if (_fs == font_style::solid) target.load_text_solid(text, _f, c);
  if (_fs == font_style::blended) target.load_text_blended(text, _f, c);
}

void theme::container_frame::load(theme * t, const std::string & frame_name)
{
  corner_top_left.load(t->get_frame_resource(frame_name, "corner_top_left.png"));
  corner_top_right.load(t->get_frame_resource(frame_name, "corner_top_right.png"));
  corner_bottom_left.load(t->get_frame_resource(frame_name, "corner_bottom_left.png"));
  corner_bottom_right.load(t->get_frame_resource(frame_name, "corner_bottom_right.png"));

  border_top.load(t->get_frame_resource(frame_name, "border_top.png"));
  border_bottom.load(t->get_frame_resource(frame_name, "border_bottom.png"));
  border_left.load(t->get_frame_resource(frame_name, "border_left.png"));
  border_right.load(t->get_frame_resource(frame_name, "border_right.png"));
}

void theme::button_frame::load(theme * t, const std::string & frame_name)
{
  left.load(t->get_frame_resource(frame_name, "left.png"));
  right.load(t->get_frame_resource(frame_name, "right.png"));
  center.load(t->get_frame_resource(frame_name, "center.png"));
  // try loading optional hover
  if (t->frame_resource_exists(frame_name, "left_hover.png")) {
    left_hover.load(t->get_frame_resource(frame_name, "left_hover.png"));
    right_hover.load(t->get_frame_resource(frame_name, "right_hover.png"));
    center_hover.load(t->get_frame_resource(frame_name, "center_hover.png"));
  }
}

void theme::push_button_frame::load(theme * t, const std::string & frame_name)
{
  idle.load(t->get_frame_resource(frame_name, "idle.png"));
  hovered.load(t->get_frame_resource(frame_name, "hovered.png"));
  if (t->frame_resource_exists(frame_name, "pressed.png")) {
    pressed.load(t->get_frame_resource(frame_name, "pressed.png"));
  }
}

theme::theme(const std::string & theme_name):
  ptr_type(pointer::normal),
  _name(theme_name)
{
  auto theme_root = filesystem::path(get_root());
  auto theme_descriptor = theme_root / "theme.json";
  if (!filesystem::exists(theme_descriptor) || !filesystem::is_regular_file(theme_descriptor)) {
    SDLEx_LogError("theme::theme - invalid theme name, descriptor is missing. '%s'",
      theme_descriptor.string().c_str());
    throw std::exception("Invalid theme name, descriptor is missing.");
  }
  SDL_Log("theme::theme - loading theme '%s'", theme_descriptor.c_str());
  _desc.load(theme_descriptor.string());

  // load colors

  color_front = _desc["color_front"].as<color>();
  color_back = _desc["color_back"].as<color>();
  color_highlight = _desc["color_highlight"].as<color>();
  color_text = _desc["color_text"].as<color>();
  color_toolbox = _desc["color_toolbox"].as<color>();
  font_text_norm.load(
    (theme_root /_desc["font_norm"]["face"].as<std::string>()).string(),
    _desc["font_norm"]["size"].as<uint32_t>());
  font_text_norm.set_style(_desc["font_norm"]["style"].as<std::string>());
  font_text_bold.load(
    (theme_root /_desc["font_bold"]["face"].as<std::string>()).string(),
    _desc["font_bold"]["size"].as<uint32_t>());
  font_text_bold.set_style(_desc["font_bold"]["style"].as<std::string>());
  font_text_ital.load(
    (theme_root /_desc["font_ital"]["face"].as<std::string>()).string(),
    _desc["font_ital"]["size"].as<uint32_t>());
  font_text_ital.set_style(_desc["font_ital"]["style"].as<std::string>());

  // load pointer
  if (_desc.has_key("pointer") && _desc["pointer"].is_object()) {
    char *norm = NULL, *rsz = NULL, *slt = NULL;
    _desc["pointer"].unpack("{s:s s:s s:s}", 
      "normal", &norm, "resize", &rsz, "select", &slt);
    ptr.tx_normal.load( (theme_root / filesystem::path(norm)).string() );
    ptr.tx_resize.load( (theme_root / filesystem::path(rsz)).string() );
    ptr.tx_select.load( (theme_root / filesystem::path(slt)).string() );
    // disable SDL pointer rendering
    SDL_ShowCursor(SDL_DISABLE);
  }

  // load frames
  dialog.load(this, "dialog");
  toolbox.load(this, "toolbox");
  group.load(this, "group");

  // buttons
  btn.load(this, "btn");
  sbtn.load(this, "sbtn");
  input.load(this, "input");
}

std::string theme::get_root() const
{
  filesystem::path theme_root(resources::root_path());
  theme_root /= "ui";
  theme_root /= _name;
  return theme_root.string();
}

std::string theme::get_frame_resource(const std::string & frame_name, const std::string & frame_res) const
{
  filesystem::path p(get_root());
  p /= frame_name;
  p /= frame_res;
  if (!exists(p)) {
    SDLEx_LogError("theme:get_frame_resource - resource not found at [%s]", p.string().c_str());
    throw std::exception("theme: resource does not exists");
  }
  return p.string();
}

bool theme::frame_resource_exists(const std::string & frame_name, const std::string & frame_res) const
{
  filesystem::path p(get_root());
  p /= frame_name;
  p /= frame_res;
  return exists(p);
}

void theme::draw_pointer(SDL_Renderer* r, const rect & dst)
{
  if (!ptr.tx_normal.is_valid() || !ptr.tx_resize.is_valid()) {
    return;
  }

  switch (ptr_type) {
  case pointer::pointer_type::normal:
    ptr.tx_normal.render(r, dst);
    break;
  case pointer::pointer_type::select:
    ptr.tx_select.render(r, dst);
    break;
  case pointer::pointer_type::resize_topleft:
    ptr.tx_resize.render(r, dst, 0.0, NULL, SDL_FLIP_HORIZONTAL);
    break;
  case pointer::pointer_type::resize_topright:
    ptr.tx_resize.render(r, dst);
    break;
  case pointer::pointer_type::resize_bottomleft:
    ptr.tx_resize.render(r, dst, 0.0, NULL, (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
    break;
  case pointer::pointer_type::resize_bottomright:
    ptr.tx_resize.render(r, dst, 0.0, NULL, SDL_FLIP_VERTICAL);
    break;
  }
}

void theme::draw_container_frame(const container_frame & f, SDL_Renderer * r, const rect & dst) const
{
  // const
  int BORDER_LENGTH = min(f.border_top.width(), f.border_left.height());
  int BORDER_HEIGHT = min(f.corner_top_left.width(), f.corner_top_left.height());

  // paint borders
  int bx = 0, xdiff = 0;
  rect bsrc(0, 0, BORDER_LENGTH, BORDER_HEIGHT);
  rect bdst(0, 0, BORDER_LENGTH, BORDER_HEIGHT);
  while (bx < dst.w) {
    bdst.x = dst.x + bx;
    bdst.y = dst.y - BORDER_HEIGHT;
    xdiff = dst.w - bx;
    if (xdiff < BORDER_LENGTH) {
      bdst.w = xdiff;
      bsrc.w = xdiff;
    }
    f.border_top.render(r, bsrc, bdst);
    bdst.y = dst.y + dst.h;
    f.border_bottom.render(r, bsrc, bdst);
    bx += BORDER_LENGTH;
  }
  int by = 0, ydiff = 0;
  bsrc.x = 0;
  bsrc.y = 0;
  bsrc.w = BORDER_HEIGHT;
  bsrc.h = BORDER_LENGTH;
  bdst.x = 0;
  bdst.y = 0;
  bdst.w = BORDER_HEIGHT;
  bdst.h = BORDER_LENGTH;
  while (by < dst.h) {
    bdst.x = dst.x - BORDER_HEIGHT;
    bdst.y = dst.y + by;
    ydiff = dst.h - by;
    if (ydiff < BORDER_LENGTH) {
      bdst.h = ydiff;
      bsrc.h = ydiff;
    }
    f.border_left.render(r, bsrc, bdst);
    bdst.x = dst.x + dst.w;
    f.border_right.render(r, bsrc, bdst);
    by += BORDER_LENGTH;
  }
    
  // paint corners
  bsrc.x = 0;
  bsrc.y = 0;
  bsrc.w = BORDER_HEIGHT;
  bsrc.h = BORDER_HEIGHT;
  bdst.x = dst.x - BORDER_HEIGHT;
  bdst.y = dst.y - BORDER_HEIGHT;
  bdst.w = BORDER_HEIGHT;
  bdst.h = BORDER_HEIGHT;
  f.corner_top_left.render(r, bsrc, bdst);
  bdst.y = dst.y + dst.h;
  f.corner_bottom_left.render(r, bsrc, bdst);
  bdst.x = dst.x + dst.w;
  f.corner_bottom_right.render(r, bsrc, bdst);
  bdst.y = dst.y - BORDER_HEIGHT;
  f.corner_top_right.render(r, bsrc, bdst);
}

void theme::draw_button_frame(const button_frame & f, SDL_Renderer * r, const rect & dst) const
{
  rect ldst(dst.x, dst.y, f.left.width(), dst.h);
  f.left.render(r, ldst);
  rect cdst(dst.x + f.left.width(), dst.y, f.center.width(), dst.h);
  int center = dst.w - (f.right.width() + f.left.width());
  for(int x = 0; x < center; ++x) {
    cdst.x = dst.x + f.left.width() + x;
    f.center.render(r, cdst);
  }
  rect rdst(dst.x + f.left.width() + center, dst.y, f.right.width(), dst.h);
  f.right.render(r, rdst);
}

} //namespace ui