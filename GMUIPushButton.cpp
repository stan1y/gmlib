#include "GMUIPushButton.h"
#include "RESLib.h"

namespace ui {

push_btn::push_btn(std::string idle_icon_file_path,
                         std::string disabled_icon_file_path,
                         std::string pressed_icon_file_path):
control("push_btn", rect()),
  _is_pressed(false),
  _sticky(false)
{
  // load primary idle icon
  if (idle_icon_file_path.length() <= 0) {
    SDLEx_LogError("push_btn::push_btn - no idle icon specified for push button");
    throw std::exception("No idle icon specified for push button");
  }
  set_idle_icon(idle_icon_file_path);

  // update own size to reflect idle frame sprite
  const texture & body = get_skin()->idle;
  set_pos(rect(pos().x, pos().y, body.width(), body.height()));

  // try to load optional icons
  if (disabled_icon_file_path.length() > 0)
    set_disabled_icon(disabled_icon_file_path);
  if (pressed_icon_file_path.length() > 0)
    set_pressed_icon(pressed_icon_file_path);

  mouse_up += boost::bind( &push_btn::on_mouse_up, this, _1 );
  mouse_down += boost::bind( &push_btn::on_mouse_down, this, _1 );
}

push_btn::push_btn(rect pos,
                         std::string idle_file_path,
                         std::string disabled_file_path,
                         std::string pressed_file_path):
  control("push_btn", pos),
  _is_pressed(false),
  _sticky(false)
{
  // load primary idle icon
  if (idle_file_path.length() <= 0) {
    SDLEx_LogError("push_btn::push_btn - no idle icon specified for push button");
    throw std::exception("No idle icon specified for push button");
  }
  set_idle_icon(idle_file_path);
  // try to load optional icons
  if (disabled_file_path.length() > 0)
    set_disabled_icon(disabled_file_path);
  if (pressed_file_path.length() > 0)
    set_pressed_icon(pressed_file_path);

  mouse_up += boost::bind( &push_btn::on_mouse_up, this, _1 );
  mouse_down += boost::bind( &push_btn::on_mouse_down, this, _1 );
}

push_btn::~push_btn()
{
}

void push_btn::on_mouse_down(control * target)
{
  _is_pressed = true;
}

void push_btn::on_mouse_up(control * target)
{
  _is_pressed = false;
}

void push_btn::set_idle_icon(const std::string & file_path)
{
  _idle_icon.load(file_path);
}

void push_btn::set_idle_icon(SDL_Texture* tex)
{
  _idle_icon.set_texture(tex);
}

void push_btn::set_disabled_icon(const std::string & file_path)
{
  _disabled_icon.load(file_path);
}

void push_btn::set_disabled_icon(SDL_Texture* tex)
{
  _disabled_icon.set_texture(tex);
}

void push_btn::set_pressed_icon(const std::string & file_path)
{
  _pressed_icon.load(file_path);
}

void push_btn::set_pressed_icon(SDL_Texture* tex)
{
  _pressed_icon.set_texture(tex);
}

void push_btn::load(const data & d)
{
  if (!d.has_key("icon_idle")) {
    SDLEx_LogError("push_btn::load - no idle icon specified");
    throw std::exception("No idle icon specified for push_btn");
  }
  set_idle_icon(resources::find(d["icon_idle"].value<std::string>()));

  if (d.has_key("icon_disabled"))
    set_disabled_icon(resources::find(d["icon_disabled"].value<std::string>()));

  if (d.has_key("icon_pressed"))
    set_pressed_icon(resources::find(d["icon_pressed"].value<std::string>()));

  if (d.has_key("sticky") && d["sticky"].value<bool>()) {
    set_sticky(true);
  }
}

void push_btn::render(SDL_Renderer* r, const rect & dst)
{
  if (disabled()) {
    if(get_skin()->disabled.is_valid()) 
      get_skin()->disabled.render(r, dst);
    if(_disabled_icon.is_valid())
      _disabled_icon.render(r, dst.center(_disabled_icon.width(), _disabled_icon.height()));
    else
      _idle_icon.render(r, dst.center(_idle_icon.width(), _idle_icon.height()));
  }
  else {
    if (_is_pressed) { 
      if(get_skin()->pressed.is_valid()) 
        get_skin()->pressed.render(r, dst);
      if(_pressed_icon.is_valid())
        _pressed_icon.render(r, dst.center(_pressed_icon.width(), _pressed_icon.height()));
      else
        _idle_icon.render(r, dst.center(_idle_icon.width(), _idle_icon.height()));
    } else {
      get_skin()->idle.render(r, dst);
      _idle_icon.render(r, dst.center(_idle_icon.width(), _idle_icon.height()));
    }
  }
}

}; //namespace ui