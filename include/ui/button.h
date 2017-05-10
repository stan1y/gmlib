#ifndef _GMUI_BUTTON_H_
#define _GMUI_BUTTON_H_

#include "label.h"
#include "frame.h"

namespace ui {

/**
  UI Standard button. 
  It is using "button" skin from theme
*/
class button : public label, public tileframe {
public:
  button(const rect & pos, 
    const icon_pos & ip = icon_pos::icon_left,
    const h_align & ha = h_align::center, 
    const v_align & va = v_align::middle,
    const padding & pad = padding());
  
  virtual void draw(SDL_Renderer * r, const rect & dst);
  virtual std::string get_type_name() const { return "btn"; }
  virtual void set_user_data(const std::string & usr) { _usr_data = usr; }
  const std::string & get_user_data() const { return _usr_data; }

  virtual void load(const json &);

 private:
  std::string _usr_data;
};

}; //namespace ui

#endif //_GMUI_BUTTON_H_
