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

#include "engine.h"
#include "util.h"

/**
   Class texture 
   SDL_Texture wrapper with additional goodies from SDLEx
   Allows pixels manipulations for SDL_TEXTUREACCESS_STREAMING
*/
class texture {
public:

  /**
      Class texture::clip_context
      Temporary setup of the renderer clip area
      with SDL_RenderSetClipRect and reset on exit
      */
  class clip_context {
  private:
    SDL_Renderer *_r;
    rect _orig;

  public:
    clip_context(SDL_Renderer *r, const rect & clip):
      _r(r)
    {
      int rc = SDL_RenderSetClipRect(r, &clip);
      if (rc != 0)
        throw sdl_exception();
    }

    virtual ~clip_context()
    {
      assert(_r);
      int rc = SDL_RenderSetClipRect(_r, NULL);
      if (rc != 0)
        throw sdl_exception();
    }
  };


  /** 
      Class texture::render_context   
      Temporary setup a texture with access = SDL_TEXTUREACCESS_STREAMING
      as a target of SDL_Renderer calls and reset back on destruction
      */
  class render_context {
  public:
    render_context(texture * t, SDL_Renderer * r):
      _r(r), _t(t), _prev(NULL) 
    {
      if (t->access() != SDL_TEXTUREACCESS_TARGET) {
        SDL_Log("%s - unsupported target texture type. it must be SDL_TEXTUREACCESS_TARGET",
          __METHOD_NAME__);
        throw std::runtime_error("Unsupported target texture type for render_context");
      }
      _prev = SDL_GetRenderTarget(_r);
      int ret = SDL_SetRenderTarget(_r, _t->get_texture());
      if (ret != 0) {
        throw sdl_exception();
      }
    }

    virtual ~render_context()
    {
      SDL_RenderPresent(_r);
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

  /* Create an empty texture */
  texture();

  /* Create a new texture of given properties
  */ 
  texture(int w, int h, 
          SDL_TextureAccess access,
          SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
          uint32_t pixel_format = SDL_PIXELFORMAT_RGBA8888);

  /* Create new texture from a file */
  texture(const std::string & file_path);

  /* Create a new texture from external instance of SDL_Texutre.
     Gives owership of a *tx to a new texture instance.
  */
  texture(SDL_Texture * tx,
          SDL_BlendMode bmode = SDL_BLENDMODE_BLEND);
  
  /* Create a new texture from a surface with format conversion */ 
  texture(SDL_Renderer *r,
          SDL_Surface *src,
          SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
          uint32_t pixel_format = SDL_PIXELFORMAT_RGBA8888);

  /* create new SDL_TEXTURE_STREAMING texture with
     pixels loaded from given SDL_Surface
  */
  texture(SDL_Surface *src,
          SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
          bool convert_transparency = true);

  ~texture();

  /* reset this texture to own a new SDL_Texture created with
     specified parameters
  */
  void blank(int w, int h,
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING,
    SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
    uint32_t pixel_format = SDL_PIXELFORMAT_RGBA8888);

  /* check if texture was initialized property */
  bool is_valid() const { return (_texture != NULL); }
  
  /* clean up underlying SDL_Texture */
  void release();

  /* load texture from file (see GM_LoadTexture) */
  void load(const std::string & file_path);

  /* set extra width and height modifier for
     this texture. They are added to rendering and
     all size calculations so the texture is scalled 
   */
  void set_scale(int dw, int dh) { _scale_w = dw; _scale_h = dh; }
  int get_wscale() { return _scale_w; }
  int get_hscale() { return _scale_h; }

  /* create a new copy of this texture's pixels 
     as a new SDL_TEXTUREACCESS_STREAMING texture
   */
  texture * copy_pixels();
  
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
  int width() const { return _width + _scale_w; }
  int height() const { return _height + _scale_h; }
  int base_width() const { return _width; }
  int base_height() const { return _height; }
  bool is_scaled() const { return _scale_w != 0 || _scale_h != 0; } 

  /* get SDL_Texture owned by this texture */
  SDL_Texture* get_texture() const { return _texture; }
  
  /* set this texture owner of a SDL_Texture. The
     SDL_Texture will have blend mode of this texture
  */
  void set_texture(SDL_Texture *tx);

  /* set this texture owner of the new SDL_Texture
     created from a given SDL_Surface with SDL_CreateTextureFromSurface
  */
  void set_surface(SDL_Surface *src, SDL_Renderer * r = NULL);

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
  int _scale_w;
  int _height;
  int _scale_h;
  void* _pixels;
  int _pitch;
  // properties
  uint32_t _format;
  SDL_TextureAccess _access;
  SDL_BlendMode _bmode;

  // raw pixels copy access
  void set_pixels(void *pixels);
  
  // transfer _texture ownership and
  // clone all properties
  void clone(texture * other);
};

#endif //_GM_TEXTURE_H_
