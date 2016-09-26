#include "GMUIPushButton.h"

namespace ui {

push_button::push_button(rect pos,
                         std::string idle_file_path,
                         std::string hovered_file_path,
                         std::string pressed_file_path):
  control("pushbtn", pos),
  _pressed(false),
  _sticky(false)
{
  // load primary idle icon
  if (idle_file_path.length() <= 0) {
    SDLEx_LogError("push_button::push_button - no idle icon specified for push button");
    throw std::exception("No idle icon specified for push button");
  }
  set_idle_icon(idle_file_path);
  // try to load optional icons
  if (hovered_file_path.length() > 0)
    set_hoverd_icon(hovered_file_path);
  if (pressed_file_path.length() > 0)
    set_pressed_icon(pressed_file_path);
}

push_button::~push_button()
{
}

void push_button::set_idle_icon(const std::string & file_path)
{
  _idle.load(file_path);
}

void push_button::set_idle_icon(SDL_Texture* tex)
{
  _idle.set_texture(tex);
}

void push_button::set_hoverd_icon(const std::string & file_path)
{
  _hovered.load(file_path);
}

void push_button::set_hovered_icon(SDL_Texture* tex)
{
  _hovered.set_texture(tex);
}

void push_button::set_pressed_icon(const std::string & file_path)
{
  _pressed.load(file_path);
}

void push_button::set_pressed_icon(SDL_Texture* tex)
{
  _pressed.set_texture(tex);
}

void push_button::load(const data & d)
{
  
}

}; //namespace ui