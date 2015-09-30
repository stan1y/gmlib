#include "RESLib.h"
#include "GMUITheme.h"
#include "GMData.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

std::string UI_GetThemeRoot(const std::string & theme_path)
{
  path root(RES_GetAssetsRoot());
  std::string t(theme_path);
  root /= t;
  return root.string();
}

namespace ui {

theme::font::font()
{
  _f = NULL;
}

theme::font::font(const std::string & font_res, uint32_t pt_size, font_style st)
{
  load(font_res, pt_size, st);
}

void theme::font::load(const std::string & font_res, uint32_t pt_size, font_style st)
{
  _f = GM_LoadFont(font_res, pt_size);
  _fs = st;
}

void theme::font::load(const std::string & font_res, uint32_t pt_size, const std::string str)
{
  font_style st;
  if (str == std::string("blended")) st = font_style::blended;
  if (str == std::string("solid")) st = font_style::solid;
  load(font_res, pt_size, st);
}

void theme::font::print(texture & target,  const std::string & text, const color & c) const
{
  if (_fs == font_style::solid) target.load_text_solid(text, _f, c);
  if (_fs == font_style::blended) target.load_text_blended(text, _f, c);
}

void theme::container_frame::load(theme * t, const std::string & frame_name)
{
  corner_top_left.load(t->get_theme_respath(frame_name, "corner_top_left.png"));
  corner_top_right.load(t->get_theme_respath(frame_name, "corner_top_right.png"));
  corner_bottom_left.load(t->get_theme_respath(frame_name, "corner_bottom_left.png"));
  corner_bottom_right.load(t->get_theme_respath(frame_name, "corner_bottom_right.png"));

  border_top.load(t->get_theme_respath(frame_name, "border_top.png"));
  border_bottom.load(t->get_theme_respath(frame_name, "border_bottom.png"));
  border_left.load(t->get_theme_respath(frame_name, "border_left.png"));
  border_right.load(t->get_theme_respath(frame_name, "border_right.png"));
}

void theme::button_frame::load(theme * t, const std::string & frame_name)
{
  left.load(t->get_theme_respath(frame_name, "left.png"));
  right.load(t->get_theme_respath(frame_name, "right.png"));
  center.load(t->get_theme_respath(frame_name, "center.png"));
  // try loading optional hover
  if (t->theme_respath_exists(frame_name, "left_hover.png")) {
    left_hover.load(t->get_theme_respath(frame_name, "left_hover.png"));
    right_hover.load(t->get_theme_respath(frame_name, "right_hover.png"));
    center_hover.load(t->get_theme_respath(frame_name, "center_hover.png"));
  }
}

void theme::push_button_frame::load(theme * t, const std::string & frame_name)
{
  idle.load(t->get_theme_respath(frame_name, "idle.png"));
  hovered.load(t->get_theme_respath(frame_name, "hovered.png"));
  if (t->theme_respath_exists(frame_name, "pressed.png")) {
    pressed.load(t->get_theme_respath(frame_name, "pressed.png"));
  }
}

theme::theme(const std::string & res_folder):
  ptr_type(pointer::normal),
  _res_root( UI_GetThemeRoot(res_folder) )
{
  path p(_res_root);
  p /= "theme.json";
  _desc.load(p.string());

  // load colors

  color_front = _desc["color_front"].as<color>();
  color_back = _desc["color_back"].as<color>();
  color_highlight = _desc["color_highlight"].as<color>();
  color_text = _desc["color_text"].as<color>();
  color_toolbox = _desc["color_toolbox"].as<color>();
  font_text_norm.load(
    _desc["font_norm"]["face"].as<std::string>(),
    _desc["font_norm"]["size"].as<uint32_t>(),
    _desc["font_norm"]["style"].as<std::string>() );
  font_text_bold.load(
    _desc["font_bold"]["face"].as<std::string>(),
    _desc["font_bold"]["size"].as<uint32_t>(),
    _desc["font_bold"]["style"].as<std::string>());
  font_text_ital.load(
    _desc["font_ital"]["face"].as<std::string>(),
    _desc["font_ital"]["size"].as<uint32_t>(),
    _desc["font_ital"]["style"].as<std::string>());

  // load pointer
  if (_desc.has_key("pointer") && _desc["pointer"].is_object()) {
    char *norm = NULL, *rsz = NULL, *slt = NULL;
    _desc["pointer"].unpack("{s:s s:s s:s}", 
      "normal", &norm, "resize", &rsz, "select", &slt);
    ptr.tx_normal.load(norm);
    ptr.tx_resize.load(rsz);
    ptr.tx_select.load(slt);
    // disable SDL pointer rendering
    SDL_ShowCursor(SDL_DISABLE);
    SDL_Log("theme: customer pointer loaded");
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

std::string theme::get_theme_respath(const std::string & frame_name, const std::string & frame_res) const
{
  path p(_res_root);
  p /= frame_name;
  p /= frame_res;
  if (!exists(p)) {
    SDLEx_LogError("theme:get_theme_respath - resource not found at [%s]", p.string().c_str());
    throw std::exception("theme: resource does not exists");
  }
  return p.string();
}

bool theme::theme_respath_exists(const std::string & frame_name, const std::string & frame_res) const
{
  path p(_res_root);
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