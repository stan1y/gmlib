#ifndef _GM_UI_INPUT_H_
#define _GM_UI_INPUT_H_

#include "GMUILabel.h"

namespace ui {

class text_input: public label {
public:

  typedef enum {
    whitespace = 1,
    alpha      = 2,
    numbers    = 4
  } input_validation;

  static const int alphanum = (input_validation::alpha | input_validation::numbers);
  static const int everything = (input_validation::alpha | input_validation::numbers | input_validation::whitespace);

  text_input(rect pos,
    input_validation valid = (input_validation)alphanum,
    margin pad = margin(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  virtual ~text_input();

  const size_t cursor() const { return _cursor; }
  void set_cursor(size_t c);

  virtual void render(SDL_Renderer * r, const rect & dst);
  virtual void load(const data &);
  virtual void update();

  void set_readonly(bool state) { _readonly = state; }
  bool is_readonly() { return _readonly; }

  const input_validation & get_validation() { return _valid; }
  void set_validation(const input_validation & v) { _valid = v; }

protected:
  void blink_cursor();
  point get_cursor_pos(const rect & dst);
  void erase_at(size_t c);
  void insert_at(size_t c, const std::string & val);
  std::string translate_sym(const uint8_t * kbdstate, const SDL_Keycode & sym);
  void on_focus(control * target);
  void on_focus_lost(control * target);
  void on_kbd_up(control * target);

  bool _readonly;

  size_t _cursor;
  input_validation _valid;
  uint8_t _cursor_alpha;
  timer _timer;
  int _blink_phase; // +1 or -1
};


}; //namespace ui

#endif //_GM_UI_INPUT_H_