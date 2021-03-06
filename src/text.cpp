#include "text.h"
#include <math.h>

namespace ui {

text_input::text_input(const rect & pos,
                       const input_validation & filter,
                       const icon_pos & ip,
                       const h_align & ha, 
                       const v_align & va,
                       const padding & pad):

  label(pos, ip, ha, va, pad),
  tileframe(ui::current_theme_sprites()),
  _readonly(false),
  _draw_frame(true),
  _filter(filter),
  _cursor(0),
  _cursor_alpha(255),
  _timer(),
  _blink_phase(-1)
{
  enable_hightlight_on_focus();
  kbd_up += boost::bind(&text_input::on_kbd_up, this, _1);
  focused += boost::bind(&text_input::on_focused, this, _1);

  set_font(ui::manager::instance()->get_font("input"));
  set_idle_color(ui::manager::instance()->get_idle_color("input"));
  set_highlight_color(ui::manager::instance()->get_highlight_color("input"));
}


text_input::~text_input()
{
  
}

void text_input::load(const json & d)
{
  if (d.find("validation") != d.end()) {
    if (d["validation"].is_string()) {
      std::string sval = d["validation"];
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
      set_validation(v);
    }
  }

  if (d.find("readonly") != d.end()) {
    _readonly = d["readonly"].get<bool>();
  }

  if (d.find("frame") != d.end()) {
    _draw_frame = d["frame"].get<bool>();
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
      if ( (std::isalpha(s[i]) | std::isspace(s[i])) && !(_filter & input_validation::alpha))
        return;
      if (std::isdigit(s[i]) && !(_filter & input_validation::numbers))
        return;
      if ( (std::iscntrl(s[i]) || std::ispunct(s[i])) && !(_filter & input_validation::whitespace) )
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
    set_cursor(get_text().length());
    break;
  case SDLK_DELETE:
    erase_at(_cursor);
    break;
  case SDLK_BACKSPACE:
    erase_at(_cursor - 1);
    break;
  };
}

void text_input::on_focused(control * target)
{
  if (_readonly)
    return;

  set_cursor(get_text().length());
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
  if (c > get_text().length())
    c = get_text().length();

  _cursor = c;
}

void text_input::erase_at(size_t c)
{
  const std::string & txt = get_text();
  std::stringstream ss;
  std::string before = txt.substr(0, c);
  ss << before;
  int chars_left = txt.length() - c - 1;
  if (chars_left > 0) {
    std::string after = txt.substr(c + 1, int32_to_uint32(chars_left));
    ss << after;
  }
  set_text(ss.str());
  if (_cursor > txt.length())
    _cursor = txt.length();
  mark_dirty();
}

void text_input::insert_at(size_t c, const std::string & val)
{
  const std::string & txt = get_text();
  std::string before = txt.substr(0, c);
  std::string after = txt.substr(c, txt.length() - c);
  std::stringstream ss;
  ss << before;
  ss << val;
  ss << after;
  set_text(ss.str());
}

// render text & cursor
void text_input::draw(SDL_Renderer * r, const rect & dst)
{
  if (_draw_frame) {
    int row = 0;
    if (is_focused()) {
      row = 3;
    }
    draw_frame(r, row, 6, dst, false, false);
  }

  label::draw(r, dst);

  // cursor
  if (!_readonly && manager::instance()->get_focused_control() == this) {
    point cur = get_cursor_pos(dst) + dst.topleft();
    color clr(get_hightlight_color());
    clr.a = _cursor_alpha;
    clr.apply(r);
    SDL_RenderDrawLine(r, cur.x, cur.y, cur.x, cur.y + get_text_texture().height());
  }
}

point text_input::get_cursor_pos(const rect & dst)
{
  std::string sub = get_text().substr(0, _cursor);
  rect txt_rect = get_font()->get_text_rect(sub);
  return point (get_text_offset().x + txt_rect.w, get_text_offset().y);
}

} //namespace ui
