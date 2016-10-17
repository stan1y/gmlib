#ifndef _GMUI_BUTTON_H_
#define _GMUI_BUTTON_H_

#include "GMUILabel.h"

namespace ui {

/**
  UI Button base class.
  Does not render any frames, see theme specific classes
  such as ui::btn, ui::sbtn, ui::tbtn
*/
class button : public label {
public:
  virtual ~button() {}

  /* toggle this button on or off */
  bool checked() { return _checked; }
  void set_checked(bool c) { _checked = c; }

  virtual void load(const data &d) { label::load(d); }
  virtual std::string get_type_name() const = 0;

protected:
  /* Protected constructor for button sub-classes. 
     No direct 'button' instances allowed this way.
  */
  button(const rect & pos,
    const icon_pos & ip = icon_pos::icon_left,
    const h_align & ha = h_align::center, 
    const v_align & va = v_align::middle,
    const padding & pad = padding());

  /* get button's frame by it's type */
  const theme::button_skin * get_btn_skin() const { 
    auto f = current_theme().get_skin(get_type_name());
    auto cf = dynamic_cast<const theme::button_skin*>(f); 
    return cf;
  }

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    // render button frame & back
    current_theme().draw_button_skin(get_btn_skin(), r, dst);
  
    // render label contents on top
    label::render(r, dst);
  }

  /* button can be togged on/off */
  bool _checked;
};

/**
  UI Standard size button. 
  It is using "btn" frame from theme
*/
class btn : public button {
public:
  btn(const rect & pos, 
    const icon_pos & ip = icon_pos::icon_left,
    const h_align & ha = h_align::center, 
    const v_align & va = v_align::middle,
    const padding & pad = padding()):
  button(pos, ip, ha, va, pad)
  {
    set_font(get_btn_skin()->font_text);
    set_idle_color(get_btn_skin()->color_text_idle);
    set_highlight_color(get_btn_skin()->color_text_highlight);

    // change pos to reflect height of the frame sprites
    size_t body_height = max(get_btn_skin()->left->height(),
      get_btn_skin()->right->height());
    
    set_pos(rect(pos.x, pos.y, pos.w, body_height));
  }

  virtual std::string get_type_name() const { return "btn"; }
};

/**
  UI Small size button.
  It is using "sbtn" frame from theme
*/
class sbtn : public button {
public:
  sbtn(const rect & pos,
    const icon_pos & ip = icon_pos::icon_left,
    const h_align & ha = h_align::left, 
    const v_align & va = v_align::middle,
    const padding & pad = padding()):
  button(pos, ip, ha, va, pad)
  {
    set_font(get_btn_skin()->font_text);
    set_idle_color(get_btn_skin()->color_text_idle);
    set_highlight_color(get_btn_skin()->color_text_highlight);

    // change pos to reflect height of the frame sprites
    size_t body_height = max(get_btn_skin()->left->height(),
      get_btn_skin()->right->height());
    set_pos(rect(pos.x, pos.y, pos.w, body_height));
  }

  virtual std::string get_type_name() const { return "sbtn"; }
};

class shape {
public:
  typedef enum {
    none      = 0,
    rectangle = 1,
    rounded   = 2,
    prism     = 3
  } shape_type;

  static void render(SDL_Renderer * r, const rect & dst, shape_type stype) {
    switch(stype) {
      case shape::rectangle:
        SDL_RenderDrawRect(r, &dst);
        break;
      case shape::rounded:
        SDLEx_RenderDrawRoundedRect(r,
          dst.x, dst.y, dst.x + dst.w, dst.y + dst.h, 3);
        break;
      case shape::prism:
        break;

      default:
      case shape::none:
        // nothing to do for none
        break;
    }
  }
};

/**
  UI Small size button.
  It is rendered with privitive rect, roundedrect, and so on
*/
class lbtn : public button {
public:
  lbtn(const rect & pos, 
    const icon_pos & ip = icon_pos::icon_left,
    const h_align & ha = h_align::center, 
    const v_align & va = v_align::middle,
    const padding & pad = padding()):
  button(pos, ip, ha, va, pad)
  {
    // lbtn can have any rect size it wants
    // no need to update this->pos() in any way
    set_font(get_btn_skin()->font_text);
    set_idle_color(get_btn_skin()->color_idle);
    set_highlight_color(get_btn_skin()->color_highlight);
  }

  virtual std::string get_type_name() const { return "lbtn"; }

  const theme::label_skin * get_btn_skin() { 
    return dynamic_cast<const theme::label_skin*>(current_theme().get_skin("lbtn")); 
  }

  void set_btn_shape(shape::shape_type s) { _btn_shape = s; }
  shape::shape_type get_btn_shape() { return _btn_shape; }

  virtual void render(SDL_Renderer * r, const rect & dst)
  {
    if (is_hovered()) {
      get_hightlight_color().apply(r);
    }
    else {
      get_idle_color().apply(r);
    }
    shape::render(r, dst, _btn_shape);
    label::render(r, dst);
  }

  virtual void load(const data &);

private:
  shape::shape_type _btn_shape;
};

}; //namespace ui

#endif //_GMUI_BUTTON_H_