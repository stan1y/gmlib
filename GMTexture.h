/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This module providers basic "texture" class for all
 * needs of off-screen rendering, blitting and media
 * representation. This is mostly based on SDL_Texture,
 * SDL_Surface and SDL_Render* set of functions. However
 * some features of "texture" are based on SDLEx library and
 * SDLEx_* set of functions in it.
 */

#ifndef _GM_TEXTURE_H_
#define _GM_TEXTURE_H_

#include "GMLib.h"

/**
   Class texture 
   SDL_Texture wrapper with additional goodies from SDLEx
   Allows pixels manipulations for SDL_TEXTUREACCESS_STREAMING
*/
class texture : public iresource {
public:

  /** 
      Class texture::render_context   
      Temporary setup a texture with access = SDL_TEXTUREACCESS_STREAMING
      as a target of SDL_Renderer calls and reset back on destruction
      */
  class render_context {
  public:
    render_context(texture * t, SDL_Renderer * r):_t(t), _r(r) 
    {
      if (t->access() != SDL_TEXTUREACCESS_TARGET) {
        SDLEx_LogError("%s - unsupported target texture type. it must be SDL_TEXTUREACCESS_TARGET",
          __METHOD_NAME__);
        throw std::exception("Unsupported target texture type for render_context");
      }
      _prev = SDL_GetRenderTarget(_r);
      int ret = SDL_SetRenderTarget(_r, _t->get_texture());
      if (ret != 0) {
        throw sdl_exception();
      }
    }

    virtual ~render_context() {
      int ret = SDL_SetRenderTarget(_r, _prev);
      if (ret != 0) {
        throw sdl_exception();
      }
    }

  private:
    SDL_Renderer * _r;
    texture * _t;
    SDL_Texture * _prev;
  };

  /* Create & Initialize the texture object */
  texture();
  texture(const std::string & file_path);
  texture(SDL_Texture * tx, SDL_BlendMode bmode = SDL_BLENDMODE_BLEND);
  texture(int w, int h, 
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING,
    SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
    uint32_t pixel_format = SDL_PIXELFORMAT_ABGR8888);
  texture(SDL_Surface* src, SDL_TextureAccess access, SDL_BlendMode bmode);

  /* Re-initialize as a new clear texture of specified type, access, pixel format*/
  void blank(int w, int h, 
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING,
    SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
    uint32_t pixel_format = SDL_PIXELFORMAT_ABGR8888);

  ~texture();

  /* check if texture was initialized property */
  bool is_valid() const { return (_texture != NULL); }
  
  /* clean up underlying SDL_Texture */
  void release();

  /* load texture from resource (see GM_LoadTexture) */
  void load(const std::string& file_path);
  
  /* load texture data from surface */
  void set_surface(SDL_Surface* src);
  
  /* load from an external surface */
  void convert_surface(SDL_Surface * s);

  /* set raw texture */
  void set_texture(SDL_Texture*);

  /* load texture data by rendering text with font */
  void load_text_solid(const std::string& text, ttf_font const * font, const color & clr);
  void load_text_blended(const std::string& text, ttf_font const * font, const color & clr);
  
  /* calculate rect needed to hold text rendered with font & color */
  static rect get_string_rect(const std::string& text, TTF_Font* font);
  
  /* get/set color modulation */
  void set_color_mod(uint8_t red, uint8_t green, uint8_t blue);
  void set_color_mod(const color & rgb);
  color get_color_mod();
  
  /* get/set blending mode */
  void set_blend_mode(SDL_BlendMode blending);
  SDL_BlendMode get_blend_mode();
  
  /* get/set alpha opacity */
  void set_alpha(uint8_t alpha);
  uint8_t get_alpha();

  /* access texture properties */
  int width() const { return _width; }
  int height() const { return _height; }

  /* raw texture pointer */
  SDL_Texture* get_texture() const { return _texture; }

  /* render it */
  void render(SDL_Renderer* r, const rect & src, const rect & dst, 
              double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) const;
  void render(SDL_Renderer* r, const rect & dst, 
              double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) const;
  void render(SDL_Renderer* r, const point & topleft, 
              double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

  /* pixels access */
  void lock();
  void unlock();
  int get_pitch() { return _pitch; }
  void* get_pixels() { return _pixels; }

  /* replaces one color with another via pixel access (slow) */
  void replace_color(const color & from, const color & to);
  
  /* texture properties */
  const SDL_TextureAccess access() { return _access; }
  const SDL_BlendMode blend_mode() { return _bmode; }
  const uint32_t pixel_format() { return _format; }

private:
  // the texture itself
  SDL_Texture* _texture;
  // pixel access details
  int _width;
  int _height;
  void* _pixels;
  int _pitch;
  // properties
  uint32_t _format;
  SDL_TextureAccess _access;
  SDL_BlendMode _bmode;
};


/**
   Class multi_texture
   This class allows transparent rendering on a 
   texture-like object which is bigger than largest
   texture size supported by SDL.
   The class will initialize a number of sub-textures
   to actually hold parts of the bigger overall texture.
   These textures all will be with access = SDL_TEXTUREACCESS_TARGET
   for rendering onto them via multi_texture::render_texture
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
  void render(SDL_Renderer * r, const point & at = point(0, 0));

  /* render a 'texture' instance onto this multi_texture at given point */
  void render_texture(SDL_Renderer * r, const texture & tx, const point & at = point(0, 0));

private:
  void init(int fragments_w = 0, int fragments_h = 0);

  int _width;
  int _height;
  container<fragment*> _fragments;
};

#endif //_GM_TEXTURE_H_