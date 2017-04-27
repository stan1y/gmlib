#ifndef _GM_UI_FRAME_H_
#define _GM_UI_FRAME_H_

#include "engine.h"

#include "texture.h"

namespace ui {

class tileframe {
  public:
    //tileframe(const std::string & tileset, int tilesize = 8);
    tileframe(const texture * tileset, int tilesize = 8);

    virtual ~tileframe();

    rect get_container_rect(const rect & control_rect) const;

    void draw_frame(SDL_Renderer * r, 
                       unsigned int row, unsigned int column, 
                       const rect & dst,
                       bool stretch_center, bool stretch_sides) const;

  private:
    int _tilesize;
    const texture * _theme_tx;

    void draw_tile(SDL_Renderer * r,
                     unsigned int row, unsigned int column,
                     int x, int y) const;

    void draw_tile_fill(SDL_Renderer * r, 
                           unsigned int row, unsigned int column, 
                           int x, int y, int w, int h, 
                           bool stretch) const;

};

}; // namespace ui

#endif //_GM_UI_FRAME_H_
