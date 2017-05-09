#ifndef MULTI_TEXTURE_H
#define MULTI_TEXTURE_H

#include "engine.h"
#include "util.h"
#include "sprite.h"

/**
 * @brief the multi_texture offscreen rendering cache
 * This class allows offscreen rendering on a texture-like object which is bigger than largest
 * texture size supported by SDL. The class will initialize a number of sub-textures (fragments)
 * to actually hold parts of the bigger overall texture. These textures all will be with
 * access = SDL_TEXTUREACCESS_TARGET for rendering onto them via multi_texture::render_texture
 */
class multi_texture {
public:

  /**
     Class multi_texture::fragment
     A fragment of the multi_texture bounded by a rect
  */
  class fragment {
  public:
     // define min/max sizes of texture fragments
    static const int fragment_max_width = 4096;
    static const int fragment_max_height = 4096;

    // create a new fragment of given size at position
    fragment(rect & pos):
      _pos(pos), _tx(pos.w, pos.h, SDL_TEXTUREACCESS_TARGET)
    {}

    ~fragment() {}
    // get fragment size and position on multi_texture
    const rect & pos() { return _pos; }
    // get underlying fragment's texture
    texture & get_texture() { return _tx; }

  private:
    rect _pos;
    texture _tx;
  };

  /* create a new multi_texture instance */
  multi_texture(const rect & size);
  multi_texture(int width, int height, int fragments_w = 0, int fragments_h = 0);
  multi_texture(const rect & size, int fragments_w = 0, int fragments_h = 0);
  ~multi_texture();

  /* get total width/height of the multi_texture */
  const int width() { return _width; }
  const int height() { return _height; }

  /* render this multi_texture with given renderer at given screen point */
  void render(SDL_Renderer *, const point & at = point(0, 0));

  void render(SDL_Renderer *, const rect & src, const rect & dst);

  /* render 'texture' instance onto this multi_texture at given point */
  void render_texture(SDL_Renderer *, const texture & tx, const point & at = point(0, 0));

  /* render 'sprite' instance onto this multi_texture at given point */
  void render_sprite(SDL_Renderer *, const sprite & s, const point & at = point(0, 0));

  void render_draw_rect(SDL_Renderer *, const rect &);
  void render_fill_rect(SDL_Renderer *, const rect &);
  void render_clear(SDL_Renderer *);

private:
  void init(int fragments_w = 0, int fragments_h = 0);

  int _width;
  int _height;
  container<fragment*> _fragments;
};

#endif // MULTI_TEXTURE_H
