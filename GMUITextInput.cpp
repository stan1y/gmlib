#include "GMUITextInput.h"
#include <math.h>

namespace ui {

text_input::text_input(rect pos,
                       input_validation valid,
                       margin pad,
                       icon_pos ip,
                       h_align ha, 
                       v_align va):
label(pos, pad, ip, ha, va),
  _valid(valid),
  _cursor(0),
  _cursor_alpha(255),
  _blink_phase(-1),
  _readonly(false),
  _draw_frame(true),
  _timer()
{
  kbd_up += boost::bind(&text_input::on_kbd_up, this, _1);
  focus += boost::bind(&text_input::on_focus, this, _1);
}

text_input::~text_input()
{
  
}

void text_input::load(const data & d)
{
  if (d.has_key("validation")) {
    if (d["validation"].is_string()) {
      std::string sval = d["validation"].as<std::string>();
      input_validation v;
      if (sval == std::string("whitespace")) {
        v = input_validation::whitespace;
      }
      if (sval == std::string("alpha")) {
        v = input_validation::alpha;
      }
      if (sval == std::string("numbers")) {
        v = input_validation::numbers;
      }
      if (sval == std::string("alphanum")) {
        v = (input_validation)alphanum;
      }
      if (sval == std::string("everything")) {
        v = (input_validation)everything;
      }
      SDL_Log("text_input: allow %s (%d)", sval.c_str(), v);
      set_validation(v);
    }
  }

  if (d.has_key("readonly")) {
    _readonly = d["readonly"].as<bool>();
  }

  if (d.has_key("frame")) {
    _draw_frame = d["frame"].as<bool>();
  }

  label::load(d);
}

void text_input::on_kbd_up(control * target)
{
  // ignore input in readonly
  if (_readonly)
    return;

  const uint8_t * kbdstate = SDL_GetKeyboardState(NULL);
  const SDL_KeyboardEvent & kbd = manager::instance()->current_event()->key;
  if (
    (kbd.keysym.sym >= SDLK_a && kbd.keysym.sym <= SDLK_z) ||
    (kbd.keysym.sym >= SDLK_0 && kbd.keysym.sym <= SDLK_9) ||
    (kbd.keysym.sym == SDLK_SPACE) ||
    (kbd.keysym.sym == SDLK_EQUALS) ||
    (kbd.keysym.sym == SDLK_MINUS) ||
    (kbd.keysym.sym == SDLK_SLASH) ||
    (kbd.keysym.sym == SDLK_BACKSLASH) ||
    (kbd.keysym.sym == SDLK_COMMA) ||
    (kbd.keysym.sym == SDLK_STOP)
  ) {
    std::string s = translate_sym(kbdstate, kbd.keysym.sym);
    for(size_t i = 0; i < s.length(); ++i) {
      if ( (std::isalpha(s[i]) | std::isspace(s[i])) && !(_valid & input_validation::alpha))
        return;
      if (std::isdigit(s[i]) && !(_valid & input_validation::numbers))
        return;
      if ( (std::iscntrl(s[i]) || std::ispunct(s[i])) && !(_valid & input_validation::whitespace) )
        return;
    }
    insert_at(_cursor, s);
    _cursor += 1;
  }
  switch(kbd.keysym.sym) {
  case SDLK_LEFT:
    set_cursor(_cursor - 1);
    break;
  case SDLK_RIGHT:
    set_cursor(_cursor + 1);
    break;
  case SDLK_HOME:
    set_cursor(0);
    break;
  case SDLK_END:
    set_cursor(_text.length());
    break;
  case SDLK_DELETE:
    erase_at(_cursor);
    break;
  case SDLK_BACKSPACE:
    erase_at(_cursor - 1);
    break;
  };
}

void text_input::on_focus(control * target)
{
  if (_readonly)
    return;

  set_cursor(_text.length());
}

void text_input::on_focus_lost(control * target)
{
  if (_readonly)
    return;
}

void text_input::update()
{
  // don't blink cursor if in read-only mode
  if (_readonly)
    return;

  blink_cursor();
}

void text_input::blink_cursor()
{
  // start/stop cursor blink timer
  if (manager::instance()->get_focused_control() != this) {
    if (_timer.is_started())
      _timer.stop();
    return;
  }

  if (!_timer.is_started()) {
    // the timer wasn't running, so this is first time update
    // call after (during) which the contol gained text focus.
    // start timer and render next frame.
    _timer.start();
    return;
  }

  // this is the target of text focus, render cursor blinks
  uint32_t ticked = _timer.get_ticks();
  //deplect 1 every 3 ms
  uint32_t elapsed = ticked / 3;
  if (elapsed >= 255) {
    // restart in other direction
    _blink_phase = -_blink_phase;
    _timer.stop();
    _timer.start();
    return;
  }
  int i_elapsed = _blink_phase * elapsed;
  int a = 0;
  if (i_elapsed < 0)
    a = 255 + i_elapsed;
  else
    a = i_elapsed;
  _cursor_alpha = int32_to_uint8(a);
}

void text_input::set_cursor(size_t c)
{
  if (c < 0)
    c = 0;
  if (c > _text.length())
    c = _text.length();

  _cursor = c;
}

void text_input::erase_at(size_t c)
{
  std::stringstream ss;
  std::string before = _text.substr(0, c);
  ss << before;
  int chars_left = _text.length() - c - 1;
  if (chars_left > 0) {
    std::string after = _text.substr(c + 1, int32_to_uint32(chars_left));
    ss << after;
  }
  _text = ss.str();
  if (_cursor > _text.length())
    _cursor = _text.length();
  _dirty = true;
}

void text_input::insert_at(size_t c, const std::string & val)
{
  std::string before = _text.substr(0, c);
  std::string after = _text.substr(c, _text.length() - c);
  std::stringstream ss;
  ss << before;
  ss << val;
  ss << after;
  _text = ss.str();
  _dirty = true;
}

// render text & cursor
void text_input::render(SDL_Renderer * r, const rect & dst)
{
  if (_draw_frame) {
    // draw input frame
    const theme & th = UI_GetTheme();
    th.draw_button_frame(th.input, r, dst);
  }
  // debug blue frame for text input rect
  if (manager::instance()->get_flags() & manager::ui_debug && \
    manager::instance()->get_focused_control() == this) 
  {
    static color blue(0, 0, 255, 255);
    blue.apply(r);
    rect cdst(dst.x + 1, dst.y + 1, dst.w - 2, dst.h - 2);
    SDL_RenderDrawRect(r, &cdst);
  }

  label::render(r, dst);

  // cursor
  if (!_readonly && manager::instance()->get_focused_control() == this) {
    point cur = get_cursor_pos(dst) + dst.topleft();
    color clr(UI_GetTheme().color_highlight);
    clr.a = _cursor_alpha;
    clr.apply(r);
    SDL_RenderDrawLine(r, cur.x, cur.y, cur.x, cur.y + _text_tx.height());
  }
}

point text_input::get_cursor_pos(const rect & dst)
{
  std::string sub = _text.substr(0, _cursor);
  rect txt_rect = texture::get_string_rect(sub, _font->ptr());
  return point (_text_offset.x + txt_rect.w, _text_offset.y);
}

}; //namespace ui