#include "RESLib.h"
#include "GMUITheme.h"
#include "GMData.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

namespace ui {

/** Frames Implementation **/

theme::label_skin::label_skin(theme * t, const std::string & skin_name):
  base_skin(skin_name),
  font_text(t->font_text)
{
  if (!t->get_data().has_key(skin_name)) {
    SDLEx_LogError("label_skin::label_skin - failed to find frame with name \"%s\"", skin_name.c_str());
    throw std::exception("Failed to find label frame");
  }

  data skin_data = t->get_data()[skin_name].value();

  // load label frame colors
  if (skin_data.has_key("color_back")) {
    color_back = skin_data["color_back"].value<color>();
  }
  else {
    color_back = t->color_back;
  }
  if (skin_data.has_key("color_idle")) {
    color_idle = skin_data["color_idle"].value<color>();
  }
  else {
    color_idle = t->color_idle;
  }
  if (skin_data.has_key("color_highlight")) {
    color_highlight = skin_data["color_highlight"].value<color>();
  }
  else {
    color_highlight = t->color_highlight;
  }
  // load frame font
  if (skin_data.has_key("font_text") && skin_data.has_subkey("font_text.face") && skin_data.has_subkey("font_text.size")) {
    font_text = new ttf_font(t->get_skin_resource(skin_name, skin_data["font_text.face"].value<std::string>()),
                                skin_data["font_text.size"].value<size_t>());
  }
}

theme::container_skin::container_skin(theme * t, const std::string & skin_name):base_skin(skin_name)
{
  if (!t->get_data().has_key(skin_name)) {
    SDLEx_LogError("container_skin::container_skin - failed to find frame with name \"%s\"", skin_name.c_str());
    throw std::exception("Failed to find container frame");
  }

  data skin_data = t->get_data()[skin_name].value();

  // load container background
  if (skin_data.has_key("color_back")) {
    color_back = skin_data["color_back"].value<color>();
  }
  else {
    color_back = t->color_back;
  }

  // load container borders & corners
  corner_top_left = resources::get_texture(t->get_skin_resource(skin_name, "corner-top-left.png"));
  corner_top_right = resources::get_texture(t->get_skin_resource(skin_name, "corner-top-right.png"));
  corner_bottom_left = resources::get_texture(t->get_skin_resource(skin_name, "corner-bottom-left.png"));
  corner_bottom_right = resources::get_texture(t->get_skin_resource(skin_name, "corner-bottom-right.png"));

  border_top = resources::get_texture(t->get_skin_resource(skin_name, "border-top.png"));
  border_bottom = resources::get_texture(t->get_skin_resource(skin_name, "border-bottom.png"));
  border_left = resources::get_texture(t->get_skin_resource(skin_name, "border-left.png"));
  border_right = resources::get_texture(t->get_skin_resource(skin_name, "border-right.png"));
}

theme::button_skin::button_skin(theme * t, const std::string & skin_name):base_skin(skin_name)
{
  if (!t->get_data().has_key(skin_name)) {
    SDLEx_LogError("label_skin::label_skin - failed to find frame with name \"%s\"", skin_name.c_str());
    throw std::exception("Failed to find label frame");
  }

  data skin_data = t->get_data()[skin_name].value();

  // load button's frame sprites
  left = resources::get_texture(t->get_skin_resource(skin_name, "left.png"));
  right = resources::get_texture(t->get_skin_resource(skin_name, "right.png"));
  center = resources::get_texture(t->get_skin_resource(skin_name, "center.png"));
  
  // try loading optional hovered sprites
  if (t->skin_resource_exists(skin_name, "left-hover.png")) {
    left_hover = resources::get_texture(t->get_skin_resource(skin_name, "left-hover.png"));
    right_hover = resources::get_texture(t->get_skin_resource(skin_name, "right-hover.png"));
    center_hover = resources::get_texture(t->get_skin_resource(skin_name, "center-hover.png"));
  }

  // load button text colors
  if (skin_data.has_key("color_idle")) {
    color_text_idle = skin_data["color_idle"].value<color>();
  }
  else {
    color_text_idle = t->color_idle;
  }
  if (skin_data.has_key("color_highlight")) {
    color_text_highlight = skin_data["color_highlight"].value<color>();
  }
  else {
    color_text_highlight = t->color_highlight;
  }

  // load button font
  if (skin_data.has_key("font_text") && skin_data.has_subkey("font_text.face") && skin_data.has_subkey("font_text.size")) {
    font_text = resources::get_font(skin_data["font_text.face"].value<std::string>(),
                                    skin_data["font_text.size"].value<int>());
    
  }
}

theme::push_button_skin::push_button_skin(theme * t, const std::string & skin_name):base_skin(skin_name)
{
  idle = resources::get_texture(t->get_skin_resource(skin_name, "idle.png"));
  if (t->skin_resource_exists(skin_name, "pressed.png")) {
    pressed = resources::get_texture(t->get_skin_resource(skin_name, "pressed.png"));
  }
  if (t->skin_resource_exists(skin_name, "disabled.png")) {
    disabled = resources::get_texture(t->get_skin_resource(skin_name, "disabled.png"));
  }
}

const data & theme::get_data() const
{
  return _desc;
}

theme::theme(const std::string & theme_name):
  ptr_type(pointer::normal),
  _desc(),
  _name(theme_name)
{
  std::stringstream theme_descr_name;
  theme_descr_name << _name << ".theme.json"; 
  fs::path theme_descriptor = resources::find_file(theme_descr_name.str());
  if (!fs::exists(theme_descriptor) || !fs::is_regular_file(theme_descriptor)) {
    SDLEx_LogError("theme::theme - invalid theme name, descriptor is missing. '%s'",
      theme_descriptor.string().c_str());
    throw std::exception("Invalid theme name, descriptor is missing.");
  }
#ifdef GM_DEBUG_UI
  SDL_Log("theme::theme - loading theme '%s'", theme_descriptor.string().c_str());
#endif

  // load descriptor
  _desc.load(theme_descriptor.string());

  // load common default colors
  if (_desc.has_key("color_back")) {
    color_back = _desc["color_back"].value<color>();
  }
  else {
    color_back = color(127, 127, 127, 255);
  }
  if (_desc.has_key("color_idle")) {
    color_idle = _desc["color_idle"].value<color>();
  }
  else {
    color_idle = color(0, 0, 0, 255);
  }
  if (_desc.has_key("color_highlight")) {
    color_highlight = _desc["color_highlight"].value<color>();
  }
  else {
    color_highlight = color::magenta();
  }
  // load common default font
  if (_desc.has_key("font_text") && _desc.has_subkey("font_text.face") && _desc.has_subkey("font_text.size")) {
    font_text = resources::get_font(_desc["font_text.face"].value<std::string>(),
                                    _desc["font_text.size"].value<int>());
    
  }
  else {
    font_text = new ttf_font("terminus.ttf", 14);
  }

  // load skins
  add_skin(new label_skin(this, "label"));
  add_skin(new label_skin(this, "text_input"));
  add_skin(new label_skin(this, "combo_box"));
  add_skin(new label_skin(this, "list_box"));
  add_skin(new label_skin(this, "scrollbar"));
  add_skin(new container_skin(this, "dialog"));
  add_skin(new container_skin(this, "toolbox"));
  add_skin(new container_skin(this, "group"));
  add_skin(new container_skin(this, "window"));
  add_skin(new button_skin(this, "btn"));
  add_skin(new button_skin(this, "sbtn"));
  add_skin(new label_skin(this, "lbtn"));
  add_skin(new push_button_skin(this, "pushbtn"));

  // load pointer
  //if (_desc.has_key("pointer") && _desc["pointer"].is_value_object()) {
  //  char *norm = NULL, *rsz = NULL, *slt = NULL;
  //  _desc["pointer"].value()->unpack("{s:s s:s s:s}", 
  //    "normal", &norm, "resize", &rsz, "select", &slt);
  //  ptr.tx_normal.load( (_root / fs::path(norm)).string() );
  //  ptr.tx_resize.load( (_root / fs::path(rsz)).string() );
  //  ptr.tx_select.load( (_root / fs::path(slt)).string() );
  //  // disable SDL pointer rendering
  //  SDL_ShowCursor(SDL_DISABLE);
  //}
}

void theme::add_skin(const theme::base_skin * f)
{
  if (_skins.find(f->name) == _skins.end()) {
    _skins.insert(std::make_pair(f->name, f));
  }
}

const theme::base_skin * theme::get_skin(const std::string & name) const
{
  auto it = _skins.find(name);
  if (it == _skins.end()) {
    SDLEx_LogError("theme::get_skin - failed to find frame with name: %s", name.c_str());
    throw std::exception("Failed to find control frame");
  }
  return it->second;
}

resources::resource_id theme::get_skin_resource(const std::string & skin_name, const std::string & skin_res) const
{
  std::stringstream skin_res_id;
  skin_res_id << _name << "-" << skin_name << "-" << skin_res;
  return skin_res_id.str();
}

bool theme::skin_resource_exists(const std::string & skin_name, const std::string & skin_res) const
{
  resources::resource_id skin_res_id = get_skin_resource(skin_name, skin_res);
  fs::path dummy;
  return resources::find_resource(skin_res_id, dummy);
}

resources::resource_id theme::get_resource(const std::string & theme_res) const
{
  std::stringstream theme_res_id;
  theme_res_id << _name << "-" << theme_res;
  return theme_res_id.str();
}

bool theme::resource_exists(const std::string & theme_res) const
{
  resources::resource_id theme_res_id = get_resource(theme_res);
  fs::path dummy;
  return resources::find_resource(theme_res_id, dummy);
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

rect theme::get_container_user_area(const container_skin * f, const rect & control_rect)
{
  return rect(control_rect.x + f->corner_top_left->width(),
              control_rect.y + f->corner_top_left->height(),
              control_rect.w - f->corner_top_left->width() - f->corner_bottom_right->width(),
              control_rect.h - f->corner_top_left->height() - f->corner_bottom_right->height());
              
}

void theme::draw_container_skin(const container_skin * f, SDL_Renderer * r, const rect & dst) const
{
  // draw corners first
  f->corner_top_left->render(r, dst.topleft());
  f->corner_top_right->render(r, point(dst.x + dst.w - f->corner_top_right->width(), dst.y));
  f->corner_bottom_left->render(r, point(dst.x, 
                                         dst.y + dst.h - f->corner_bottom_left->height()));
  f->corner_bottom_right->render(r, point(dst.x + dst.w - f->corner_bottom_left->width(),
                                          dst.y + dst.h - f->corner_bottom_left->height()));

  // draw borders between corners by stretching them
  f->border_top->render(r, rect(dst.x + f->corner_top_left->width(),
                                dst.y,
                                dst.w - (f->corner_top_left->width() + f->corner_top_right->width()),
                                f->border_top->height()));
  f->border_bottom->render(r, rect(dst.x + f->corner_bottom_left->width(),
                                dst.y + dst.h - f->border_bottom->height(),
                                dst.w - (f->corner_bottom_left->width() + f->corner_bottom_right->width()),
                                f->border_bottom->height()));
  f->border_left->render(r, rect(dst.x,
                                 dst.y + f->corner_top_left->height(),
                                 f->border_left->width(),
                                 dst.h - (f->corner_top_left->height() + f->corner_bottom_left->height())));
  f->border_right->render(r, rect(dst.x + dst.w - f->border_right->width(),
                                  dst.y + f->corner_top_right->height(),
                                  f->border_right->width(),
                                  dst.h - (f->corner_top_right->height() + f->corner_bottom_right->height())));
}

void theme::draw_button_skin(const button_skin * f, SDL_Renderer * r, const rect & dst) const
{
  rect ldst(dst.x, dst.y, f->left->width(), dst.h);
  f->left->render(r, ldst);
  rect cdst(dst.x + f->left->width(), dst.y, f->center->width(), dst.h);
  int center = dst.w - (f->right->width() + f->left->width());
  for(int x = 0; x < center; ++x) {
    cdst.x = dst.x + f->left->width() + x;
    f->center->render(r, cdst);
  }
  rect rdst(dst.x + f->left->width() + center, dst.y, f->right->width(), dst.h);
  f->right->render(r, rdst);
}

} //namespace ui