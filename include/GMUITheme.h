#ifndef _GMUI_THEME_H_
#define _GMUI_THEME_H_

#include "GMTexture.h"
#include "GMData.h"
#include "RESLib.h"

namespace ui {

/**
  UI Theme class.
  Encapsulates theme-based control frames rendering
*/
class theme {
public:

  /* Themed pointer information */
  struct pointer {
    enum pointer_type {
      normal = 0,
      resize_topleft = 1,
      resize_topright = 2,
      resize_bottomleft = 3,
      resize_bottomright = 4,
      select = 5
    };
    texture tx_normal;
    texture tx_resize;
    texture tx_select;
  };

  /* UI Theme's font cache 
  class font : public ttf_font {
  public:
    typedef enum {
      solid = 0,
      blended = 1
    } font_style;
    
    // load from file, managed mode
    font(const std::string & file_path, size_t pts, font_style st);
    font(const std::string & file_path, size_t pts, const std::string & style = "blended");
    
    void print(texture & target, const std::string & text, const color & c) const;
    
    font_style style() const { return _fs; }
    void set_style(font_style & s) { _fs = s; }
    void set_style(const std::string & fs)
    {
      if (fs == "solid") {
        _fs = font::solid;
      }
      if (fs == "blended") {
        _fs = font::blended;
      }
    }

  private:
    font_style _fs;
  };
  */
  struct base_skin {
    /* frame's name, the control's type */
    std::string name;
    base_skin(const std::string & n):name(n) {}
    virtual ~base_skin() {}
  };

  /* label frame, the most basic one */
  struct label_skin: public base_skin {

    label_skin(theme * t, const std::string & skin_name);

    /* frame specific colors & fonts */
    color color_back;
    color color_idle;
    color color_highlight;
    const ttf_font * font_text;
  };

  /* Frames declarations */
  struct container_skin : public base_skin {
     const texture * corner_top_left;
     const texture * corner_top_right;
     const texture * corner_bottom_left;
     const texture * corner_bottom_right;
     const texture * border_top;
     const texture * border_bottom;
     const texture * border_left;
     const texture * border_right;
     color color_back;

     container_skin(theme * t, const std::string & skin_name);

     static const container_skin * dummy() { return nullptr; }
   };

   class button_skin: public base_skin {
   public:
     const texture * left;
     const texture * right;
     const texture * center;

     const texture * left_hovered;
     const texture * right_hovered;
     const texture * center_hovered;

     const texture * left_pressed;
     const texture * right_pressed;
     const texture * center_pressed;

     ttf_font const * font_text;
     color color_text_idle;
     color color_text_highlight;
     
     button_skin(theme * t, const std::string & skin_name);
   };

   class push_button_skin: public base_skin {
   public:
     const texture * idle;
     const texture * disabled;
     const texture * pressed;

     push_button_skin(theme * t, const std::string & skin_name);
   };

  /** Common default colors and fonts */
  color color_back;
  color color_idle;
  color color_highlight;
  const ttf_font * font_text;

  /* Pointer */
  pointer ptr;
  pointer::pointer_type ptr_type;

  /* Create a new theme based folder in ui resources */
  theme(const std::string & theme_name);

  /* Get path to theme resource */
  const data & get_data() const;
  
  std::string get_skin_resource(const std::string & skin_name, const std::string & skin_res) const;
  bool skin_resource_exists(const std::string & skin_name, const std::string & skin_res) const;

  std::string get_resource(const std::string & theme_res) const;
  bool resource_exists(const std::string & skin_res) const;

  /** Theme Drawing APIs */
  static rect get_container_user_area(const container_skin * f, const rect & control_rect);
  void draw_container_skin(const container_skin * f, SDL_Renderer * r, const rect & dst) const;
  void draw_button_skin(const button_skin * f, SDL_Renderer * r, const rect & dst, bool hovered = false, bool pressed = false) const;
  void draw_pointer(SDL_Renderer* r, const rect & dst);

  const data & get_desc() const { return _desc; }

  /** Theme Setup APIs */
  void add_skin(const base_skin *);
  const base_skin * get_skin(const std::string & name) const;

private:
  /* theme's privates */
  data _desc;
  std::string _name;

  std::map<std::string, const base_skin*> _skins;
};

}; //namespace ui

#endif //_GMUI_THEME_H_