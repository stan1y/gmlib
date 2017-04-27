#include "frame.h"


namespace ui {

//tileframe::tileframe(const std::string & res, int tilesize):
//	_tilesize(tilesize),
//	_theme_tx(nullptr)
//{
//  _theme_tx = resources::get_texture(res);
//}

tileframe::tileframe(const texture * tx, int tilesize):
	_tilesize(tilesize),
	_theme_tx(tx)
{
}

tileframe::~tileframe()
{

}

rect tileframe::get_container_rect(const rect & control_rect) const
{
  return rect(control_rect.x + _tilesize,
              control_rect.y + _tilesize,
              control_rect.w - _tilesize * 2,
              control_rect.h - _tilesize * 2);
              
}

void tileframe::draw_frame(SDL_Renderer * r, 
                       unsigned int row, unsigned int column, 
                       const rect & dst,
                       bool stretch_center, bool stretch_sides) const
{
  int x = dst.x, y = dst.y, w = dst.w, h = dst.h;
	/* fill with background */
    draw_tile_fill(r, row + 1, column + 1,
                 x + _tilesize, y + _tilesize, w - _tilesize * 2, h - _tilesize * 2,
								 stretch_center);
	
	/* fill top */
	draw_tile_fill(r, row + 0, column + 1, 
                 x + _tilesize, y, w - _tilesize * 2, _tilesize, 
								 stretch_sides);
	
	/* fill bottom */
	draw_tile_fill(r, row + 2, column + 1, 
                 x + _tilesize, y + h - _tilesize, w - _tilesize * 2, _tilesize,
								 stretch_sides);
    
  /* fill left */
	draw_tile_fill(r, row + 1, column + 0,
					       x, y + _tilesize, _tilesize, h - _tilesize * 2,
					       stretch_sides);
  
  /* fill right */
	draw_tile_fill(r, row + 1, column + 2,
					       x + (w - _tilesize), y + _tilesize, _tilesize, h - _tilesize * 2,
					       stretch_sides);
	
  /* render top left */
	draw_tile(r, row, column, x, y);
  
  /* render top right */
	draw_tile(r, row, column + 2, x + w - _tilesize, y);
  
  /* render bottom left */
	draw_tile(r, row + 2, column, x, y + h - _tilesize);
  
  /* render bottom right */
	draw_tile(r, row + 2, column + 2, x + (w - _tilesize), y + h - _tilesize);
}

void tileframe::draw_tile(SDL_Renderer * r,
				             unsigned int row, unsigned int column,
				             int x, int y) const
{
	rect clip;
  rect target;
  clip.x = column * _tilesize;
  clip.y = row * _tilesize;
  clip.w = _tilesize;
  clip.h = _tilesize;
  
  target.x = x;
  target.y = y;
  target.w = _tilesize;
  target.h = _tilesize;
  
	
	_theme_tx->render(r, clip, target);
}

void tileframe::draw_tile_fill(SDL_Renderer * r, 
                           unsigned int row, unsigned int column, 
                           int x, int y, int w, int h, 
                           bool stretch) const
{
  int i;
  int j;
  
  rect clip;
  rect target;
  clip.x = column * _tilesize;
  clip.y = row * _tilesize;
  clip.w = _tilesize;
  clip.h = _tilesize;
  
  target.x = x;
  target.y = y;
  target.w = _tilesize;
  target.h = _tilesize;

  if (stretch) {
    target.w = w;
    target.h = h;
    _theme_tx->render(r, clip, target);
    return;
  }

  /* iterate by column */
  for (i = x; i < x + w;) {
    
    /* set the target column */
    target.x = i;
    
    /* reset clipping */
    clip.w = _tilesize;
    
    /* clip the width of the tile if it does not fit */
    if (i + _tilesize > x + w) {
      clip.w = w % _tilesize;
    }
    target.w = clip.w;
    
    /* iterate by lines in a column */
    for (j = y; j < y + h;) {
      /* set target line */
      target.y = j;
      
      /* reset clipping */
      clip.h = _tilesize;
      
      /* clip tile heigh if it does not fit */
      if (j + _tilesize > y + h) {
        clip.h = h % _tilesize;
      }
      target.h = clip.h;
      
      /* finally render it */
      _theme_tx->render(r, clip, target);
      
      j += _tilesize;
    }
    
    i += _tilesize;
  }
}

}
