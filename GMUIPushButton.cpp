#include "GMUIPushButton.h"

namespace ui {

push_button::push_button(rect pos, std::string idle, std::string hovered, std::string pressed):
  control("pushbtn", pos),
  _pressed(false),
  _sticky(false)
{
  if (idle.length() > 0)
    set_idle_icon(idle);
  if (hovered.length() > 0)
    set_hoverd_icon(hovered);
  if (pressed.length() > 0)
    set_pressed_icon(pressed);
}

push_button::~push_button()
{
}

void push_button::set_idle_icon(const std::string & res)
{
  _idle.load(res);
}

void push_button::set_idle_icon(SDL_Texture* tex)
{
  _idle.set_texture(tex);
}

void push_button::set_hoverd_icon(const std::string & res)
{
  _hovered.load(res);
}

void push_button::set_hovered_icon(SDL_Texture* tex)
{
  _hovered.set_texture(tex);
}

void push_button::set_pressed_icon(const std::string & res)
{
  _pressed.load(res);
}

void push_button::set_pressed_icon(SDL_Texture* tex)
{
  _pressed.set_texture(tex);
}

void push_button::load(const data & d)
{
  
}

}; //namespace ui