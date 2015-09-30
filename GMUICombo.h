#ifndef _GM_UI_COMBO_H_
#define _GM_UI_COMBO_H_

#include "GMUIBox.h"
#include "GMUITextInput.h"

namespace ui {

class combo: public text_input {
public:

  class area: public box {
  public:
    area(rect pos, const color & clr):box(pos, ui::box::vbox, ui::box::fill, 0), _back(clr)
    {}

    virtual void render(SDL_Renderer* r, const rect & dst);

    void set_back_color(const color & clr) { _back = clr; }
    const color & get_back_color() { return _back; }

  private:
    color _back;
  };

  combo(rect pos,
    int area_maxlen = 100, 
    input_validation valid = (input_validation)alphanum,
    margin pad = margin(),
    icon_pos ip = icon_pos::icon_left,
    h_align ha = label::left, 
    v_align va = label::top);

  virtual ~combo();

  label * get_item(size_t item);
  size_t add_item(const std::string & text, margin pad = margin(0), h_align ha = h_align::left, v_align va = v_align::middle);
  void delete_item(size_t item);
  
  int find_text(const std::string & text);

  virtual void load(const data &);
  virtual void update();
  virtual void render(SDL_Renderer* r, const rect & dst);

  void expand() { _area->set_visible(true); }
  void colapse() { _area->set_visible(false); }
  bool is_expanded() { return _area->visible(); }

  void set_expand_on_hover(bool state) { _expand_on_hover = state; }
  bool get_expand_on_hover() { return _expand_on_hover; }

  area * get_area() { return _area; }

private:
  void resize_area();
  void on_hover(control * target);
  void on_hover_lost(control * target);
  void on_focus(control * target);
  void on_focus_lost(control * target);
  void on_item_mouseup(control * target);

  bool _expand_on_hover;
  int _area_maxlen;
  area * _area;
};

}; //namespace ui

#endif //_GM_UI_COMBO_H_