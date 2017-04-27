#ifndef _GM_UI_COMBO_H_
#define _GM_UI_COMBO_H_

#include "box.h"
#include "text.h"
#include "frame.h"

namespace ui {

class combo: public text_input {
public:

  class area: public box, public tileframe {
  public:
    area(const rect & pos):
      box(pos, ui::box::vbox, h_align::left, v_align::middle, 0),
      tileframe(ui::current_theme())
    {}

    virtual void draw(SDL_Renderer* r, const rect & dst);
  };

  combo(const rect & pos,
        const int max_box_height = 100,
        const int item_height = 32,
        const input_validation & valid = (input_validation)alphanum,
        const icon_pos & ip = icon_pos::icon_left,
        const h_align & ha = h_align::left, 
        const v_align & va = v_align::middle,
        const padding & pad = padding(2));

  virtual std::string get_type_name() const { return "combo"; }

  virtual ~combo();

  label * get_item(size_t item);
  size_t add_item(const std::string & text,
                  const padding & pad = padding(0),
                  const h_align & ha = h_align::left,
                  const v_align & va = v_align::middle);
  void delete_item(size_t item);
  
  int find_text(const std::string & text);

  virtual void load(const json &);
  virtual void update();
  virtual void draw(SDL_Renderer* r, const rect & dst);

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
  void on_focused(control * target);
  void on_focus_lost(control * target);
  void on_item_mouseup(control * target);

  bool _expand_on_hover;
  int _max_box_height;
  int _item_height;
  area * _area;
};

}; //namespace ui

#endif //_GM_UI_COMBO_H_
