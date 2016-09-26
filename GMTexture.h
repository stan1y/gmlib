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

/*
  Texture wrapper 
  with SDL_TEXTUREACCESS_STREAMING access
  for pixel manipulations
*/
class texture : public iresource {
public:

  /** Texture render context
      Temporary setup a texture (SDL_TEXTUREACCESS_STREAMING)
      as a target of SDL_Renderer calls
      and reset back on destruction
      */
  class render_context {
  public:
    render_context(texture * t, SDL_Renderer * r):_t(t), _r(r) 
    {
      _prev = SDL_GetRenderTarget(_r);
      int ret = SDL_SetRenderTarget(_r, _t->get_texture());
      if (ret != 0) {
        SDLEx_LogError("render_context: failed to set target to texture. %s", SDL_GetError());
        //throw std::exception("failed to set render target to texture");
      }
    }

    virtual ~render_context() {
      int ret = SDL_SetRenderTarget(_r, _prev);
      if (ret != 0) {
        SDLEx_LogError("render_context: failed to set target to texture. %s", SDL_GetError());
        //throw std::exception("failed to set render target to texture");
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

  void blank(int w, int h, 
    SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING,
    SDL_BlendMode bmode = SDL_BLENDMODE_BLEND,
    uint32_t pixel_format = SDL_PIXELFORMAT_ABGR8888);

  ~texture();

  bool is_valid() const { return (_texture != NULL); }
  
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
  void load_text_solid(const std::string& text, TTF_Font* font, const color & clr);
  void load_text_blended(const std::string& text, TTF_Font* font, const color & clr);
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
  SDL_Texture* get_texture() const { return _texture; }

  /* render it */
  void render(SDL_Renderer* r, const rect & src, const rect & dst, 
    double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE
    ) const;
  void render(SDL_Renderer* r, const rect & dst, 
    double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE
    ) const;
  void render(SDL_Renderer* r, const point & topleft, 
    double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE
    ) const;

  /* pixels access */
  void lock();
  void unlock();
  int get_pitch() { return _pitch; }
  void* get_pixels() { return _pixels; }
  void replace_color(const color & from, const color & to);

  /* name of the loaded resource */
  const std::string resource_name() const { return _resource; }

private:

  std::string _resource;
  SDL_Texture* _texture;
  int _width;
  int _height;
  void* _pixels;
  int _pitch;
  uint32_t _format;
  SDL_TextureAccess _access;
  SDL_BlendMode _bmode;
};


#endif //_GM_TEXTURE_H_