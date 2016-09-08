#ifndef _GMUI_THEME_H_
#define _GMUI_THEME_H_

#include "GMTexture.h"
#include "GMData.h"

namespace ui {

/**
  UI Theme class.
  Encapsulates theme-based control frames rendering
*/
class theme {
private:
  data _desc;
  std::string _name;

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

  /* UI Theme's font cache */
  class font : public ttf_font {
  public:
    typedef enum {
      solid = 0,
      blended = 1
    } font_style;

    font() {}
    
    font(const std::string & file_path, size_t pts, font_style st);
    
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
    texture _pointer;
    texture _pointer_resize;
  };

  class frame {
  public:
    virtual void load(theme * t, const std::string & frame_name) = 0;
  };

  /* Frames declarations */
  class container_frame : public frame {
  public:
     texture corner_top_left;
     texture corner_top_right;
     texture corner_bottom_left;
     texture corner_bottom_right;
     texture border_top;
     texture border_bottom;
     texture border_left;
     texture border_right;
     virtual void load(theme * t, const std::string & frame_name);
   };

   class button_frame {
   public:
     texture left;
     texture right;
     texture center;

     texture left_hover;
     texture right_hover;
     texture center_hover;

     virtual void load(theme * t, const std::string & frame_name);
   };

   class push_button_frame {
   public:
     texture idle;
     texture hovered;
     texture pressed;

     virtual void load(theme * t, const std::string & frame_name);
   };

  /** Available Frames */
  container_frame dialog;
  container_frame toolbox;
  container_frame group;
  button_frame btn;
  button_frame sbtn;
  button_frame input;

  /** Available colors & fonts **/
  color color_front;
  color color_back;
  color color_highlight;
  color color_text;
  color color_toolbox;
  font font_text_norm;
  font font_text_bold;
  font font_text_ital;

  /* Pointer */
  pointer ptr;
  pointer::pointer_type ptr_type;

  /* Create new frame based on resources folder */
  theme(const std::string & theme_name);

  /* Get path to theme resource */
  std::string get_root() const;
  std::string get_frame_resource(const std::string & frame_name, const std::string & frame_res) const;
  bool frame_resource_exists(const std::string & frame_name, const std::string & frame_res) const;

  /** Theme Drawing APIs */
  void draw_container_frame(const container_frame & f, SDL_Renderer * r, const rect & dst) const;
  void draw_button_frame(const button_frame & f, SDL_Renderer * r, const rect & dst) const;
  void draw_pointer(SDL_Renderer* r, const rect & dst);

  const data & get_desc() const { return _desc; }
};

}; //namespace ui

#endif //_GMUI_THEME_H_