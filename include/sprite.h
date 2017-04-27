/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This module implements sprites rendering via "texture" and
 * its raw SDL_Texture representations. The module provides
 * several classes for sprites and sprite sheets management.
 */

#ifndef _GM_SPRITES_H_
#define _GM_SPRITES_H_

#include "engine.h"
#include "texture.h"

/* Sprites Sheet */

class sprites_sheet : public texture {
public:
  sprites_sheet(const std::string & file_path, uint32_t sprite_w, uint32_t sprite_h);

  inline bool operator== (sprites_sheet & other) {
    return (get_texture() == other.get_texture() && _rows == other._rows && _cols == other._cols);
  }
  inline bool operator!= (sprites_sheet & other) {
    return !(*this == other);
  }

  rect get_sprite_cliprect(size_t idx) const;
  uint32_t sprite_width() const { return width() / _cols; }
  uint32_t sprite_height() const { return height() / _rows; }
  uint32_t rows() const { return _rows; }
  uint32_t cols() const { return _cols; }

private:
  uint32_t _cols;
  uint32_t _rows;
};

/* Sprite */

class sprite {
public:
  /* Sprite constants */
  static const size_t invalid = UINT32_MAX; // invalid sprite index 
  static const uint8_t alpha_full = 255;   // alpha modifiers
  static const uint8_t alpha_half = 127;
  static const uint8_t alpha_none = 0;

  /* sprite index in sheet and size */
  size_t idx;
  uint32_t w;
  uint32_t h;

  /* rotation info */
  SDL_RendererFlip flip;
  int angle;

  inline bool valid() { return idx != invalid; }
  inline bool operator== (sprite& other) {
      return (idx == other.idx && w == other.w && h == other.h && flip == other.flip && angle == other.angle && _sheet == other._sheet);
  }
  inline bool operator!= (sprite& other) {
      return !(*this == other);
  }

  /* create new empty sprite */
  sprite();

  /* create new sprite with texture index */
  sprite(size_t tex_idx, int px_w, int px_h, sprites_sheet const * sheet);
  virtual ~sprite() {}

  /* get center point based on rect */
  SDL_Point center();

  /* render sprite at abosulute renderer position */
  void render(SDL_Renderer * r, const point & dst_pnt) const;
  void render(SDL_Renderer * r, const rect & dst) const;

  /* render a sprite with altered src rect at absolute renderer position.
     dsrc's x and y are added to the sprites's src rect.
     dsrc's w and h are copied to src if w and h both > 0.
  */
  void render(SDL_Renderer * r, const rect & dsrc, const rect & dst) const;
  void render(SDL_Renderer * r, const rect & dsrc, const point & dst_pnt) const;

protected:
  /* sprites sheet*/
  const sprites_sheet* _sheet;
};

#endif //_GM_SPRITES_H_
