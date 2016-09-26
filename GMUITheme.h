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
  struct base_frame {
    /* frame's name, the control's type */
    std::string name;
    base_frame(const std::string & n):name(n) {}
    virtual ~base_frame() {}
  };

  /* label frame, the most basic one */
  struct label_frame: public base_frame {

    label_frame(theme * t, const std::string & frame_name);

    /* frame specific colors & fonts */
    color color_back;
    color color_idle;
    color color_highlight;
    ttf_font const * font_text;
  };

  /* Frames declarations */
  struct container_frame : public base_frame {
     texture corner_top_left;
     texture corner_top_right;
     texture corner_bottom_left;
     texture corner_bottom_right;
     texture border_top;
     texture border_bottom;
     texture border_left;
     texture border_right;
     color color_back;

     container_frame(theme * t, const std::string & frame_name);
   };

   class button_frame: public base_frame {
   public:
     texture left;
     texture right;
     texture center;

     texture left_hover;
     texture right_hover;
     texture center_hover;

     ttf_font const * font_text;
     color color_text_idle;
     color color_text_highlight;
     
     button_frame(theme * t, const std::string & frame_name);
   };

   class push_button_frame: public base_frame {
   public:
     texture idle;
     texture selected;
     texture disabled;
     texture hovered;

     push_button_frame(theme * t, const std::string & frame_name);
   };

  /** Common default colors and fonts */
  color color_back;
  color color_idle;
  color color_highlight;
  ttf_font const * font_text;

  /* Pointer */
  pointer ptr;
  pointer::pointer_type ptr_type;

  /* Create a new theme based folder in ui resources */
  theme(const std::string & theme_name);

  /* Get path to theme resource */
  const data & get_data() const;
  std::string get_root() const;
  
  std::string get_frame_resource(const std::string & frame_name, const std::string & frame_res) const;
  bool frame_resource_exists(const std::string & frame_name, const std::string & frame_res) const;

  std::string get_resource(const std::string & theme_res) const;
  bool resource_exists(const std::string & frame_res) const;

  /** Theme Drawing APIs */
  void draw_container_frame(const container_frame * f, SDL_Renderer * r, const rect & dst) const;
  void draw_button_frame(const button_frame * f, SDL_Renderer * r, const rect & dst) const;
  void draw_pointer(SDL_Renderer* r, const rect & dst);

  const data & get_desc() const { return _desc; }

  /** Theme Setup APIs */
  void add_frame(const base_frame *);
  const base_frame * get_frame(const std::string & name) const;

private:
  /* theme's privates */
  data _desc;
  std::string _name;

  std::map<std::string, const base_frame*> _frames;
};

}; //namespace ui

#endif //_GMUI_THEME_H_