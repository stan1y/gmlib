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
  std::string _res_root;

public:

  /* UI Theme's font cache */
  class font {
  public:
    typedef enum {
      solid = 0,
      blended = 1
    } font_style;

    font();
    font(const std::string & font_res, uint32_t pt_size, font_style st);
    void load(const std::string & font_res, uint32_t pt_size, font_style st);
    void load(const std::string & font_res, uint32_t pt_size, const std::string st);
    void print(texture & target, const std::string & text, const color & c) const;

    TTF_Font* ptr() const { return _f; }
    font_style style() const { return _fs; }

  private:
    TTF_Font * _f;
    font_style _fs;
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

     color font_color;
     color font_hover_color;

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
  font font_text_norm;
  font font_text_bold;
  font font_text_ital;

  /* Create new frame based on resources folder */
  theme(const std::string & res_folder);

  /* Get path to theme resource */
  std::string get_theme_respath(const std::string & frame_name, const std::string & frame_res) const;
  bool theme_respath_exists(const std::string & frame_name, const std::string & frame_res) const;

  /** Theme Drawing APIs */
  void draw_container_frame(const container_frame & f, SDL_Renderer * r, const rect & dst) const;
  void draw_button_frame(const button_frame & f, SDL_Renderer * r, const rect & dst) const;

  const data & get_desc() const { return _desc; }
};

}; //namespace ui

#endif //_GMUI_THEME_H_