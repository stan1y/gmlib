#include "GMTexture.h"

/* Texture */

texture::texture():
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _format(SDL_PIXELFORMAT_ABGR8888),
  _access(SDL_TEXTUREACCESS_STREAMING),
  _bmode(SDL_BLENDMODE_BLEND)
{
}

texture::texture(SDL_Texture * tx, SDL_BlendMode bmode):
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _format(SDL_PIXELFORMAT_ABGR8888),
  _access(SDL_TEXTUREACCESS_STREAMING),
  _bmode(bmode)
{
  set_texture(tx);
}

texture::texture(const std::string & file_path):
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _format(SDL_PIXELFORMAT_ABGR8888),
  _access(SDL_TEXTUREACCESS_STATIC),
  _bmode(SDL_BLENDMODE_BLEND)
{
  load(file_path);
}

texture::texture(SDL_Surface* src, SDL_TextureAccess access, SDL_BlendMode bmode):
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _format(SDL_PIXELFORMAT_ABGR8888),
  _access(access),
  _bmode(bmode)

{
  convert_surface(src);
}

texture::texture(int w, int h, SDL_TextureAccess access, SDL_BlendMode bmode, uint32_t pixel_format):
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _format(pixel_format),
  _access(access),
  _bmode(bmode)
{
  blank(w, h, access, bmode);
}

void texture::set_texture(SDL_Texture * tx)
{
  release();
  
  _texture = tx;
  SDL_SetTextureBlendMode(_texture, _bmode);

  uint32_t fmt = 0;
  int access = 0;
  SDL_QueryTexture(tx, &fmt, &access, &_width, &_height);
}

void texture::blank(int w, int h, SDL_TextureAccess access, SDL_BlendMode bmode, uint32_t pixel_format)
{
  SDL_Texture * tx = GM_CreateTexture(w, h, access, pixel_format);
  set_texture(tx); 
}

void texture::release()
{
  if (_texture) {
    SDL_DestroyTexture(_texture);
  }
  _texture = NULL;
}

texture::~texture()
{
  release();
}

void texture::load(const std::string& file_path)
{
  _resource = file_path;
  SDL_Surface* src = GM_LoadSurface(file_path);
  convert_surface(src);

  if (file_path.find(".png") == std::string::npos) {
    // convert pixels to transparent for non-PNG files
    uint32_t pixel = SDLEx_GetPixel(src, 0, 0);
    uint8_t r = 0, g = 0, b = 0;
    SDL_GetRGB(pixel, src->format, &r, &g, &b);
    replace_color(color(r, g, b, 255), color(0, 0, 0, 0));
  }
  SDL_FreeSurface(src);
}

rect texture::get_string_rect(const std::string& text, TTF_Font* font)
{
  rect r(0, 0, 0, 0);
  TTF_SizeText(font, text.c_str(), &r.w, &r.h);
  return r;
}

void texture::convert_surface(SDL_Surface * s)
{
  if (s == NULL) {
    SDLEx_LogError("%s - NULL surface to convert", __METHOD_NAME__);
    throw std::exception("NULL surface to convert");
  }
  SDL_Surface* converted = SDL_ConvertSurfaceFormat(s, _format, 0);
  set_surface(converted);
  SDL_FreeSurface(converted);
}

void texture::load_text_solid(const std::string& text, TTF_Font* font, const color & c)
{
  convert_surface(TTF_RenderText_Solid(font, text.c_str(), c));
}

void texture::load_text_blended(const std::string& text, TTF_Font* font, const color & c)
{
  convert_surface(TTF_RenderText_Blended(font, text.c_str(), c));
}

void texture::set_surface(SDL_Surface* src)
{
  // we expect this surface to to be converted into our
  // desired pixel format for current renderer

  if (_access == SDL_TEXTUREACCESS_STREAMING) {
    // init empty new texture canvas
    blank(src->w, src->h, _access, _bmode);
    // streaming texture has user-memory pixels
    // copy them into the "_pixels" array
    lock();
    memcpy(_pixels, src->pixels, src->pitch * src->h);
    //apply _pixles to the texture
    unlock();
  }
  else if (_access == SDL_TEXTUREACCESS_STATIC) {
    set_texture(SDL_CreateTextureFromSurface(GM_GetRenderer(), src));
  }
  else if (_access == SDL_TEXTUREACCESS_TARGET) {
    SDLEx_LogError("%s - texture with access type SDL_TEXTUREACCESS_TARGET cannot be loaded from surface",
      __METHOD_NAME__);
    throw std::exception("Cannot load SDL_TEXTUREACCESS_TARGET texture from surface");
  }
}

void texture::lock()
{
  if (_pixels != NULL) {
    //already locked
    return;
  }
  if (SDL_LockTexture(_texture, NULL, &_pixels, &_pitch) != 0) {
    throw sdl_exception();
  }
}

void texture::unlock()
{
  if (_pixels == NULL) {
    //not locked
    return;
  }

  SDL_UnlockTexture(_texture);
  _pixels = NULL;
  _pitch = 0;
}

void texture::replace_color(const color & from, const color & to)
{
  if (_format != SDL_PIXELFORMAT_ABGR8888) {
    SDLEx_LogError("%s - unsupported pixel format (%d) for this method. Only SDL_PIXELFORMAT_ABGR8888 is supported.", 
      __METHOD_NAME__, _format);
    throw std::exception("Unsupported pixel format for method");
  }
  // SDL_PIXELFORMAT_ABGR8888 is 8 bits long each
  static const size_t pixel_size = 8;

  lock();
  SDL_PixelFormat* fmt = SDL_AllocFormat(_format);
  if (fmt == NULL)
    throw sdl_exception();
  uint32_t f = SDL_MapRGBA(fmt, from.r, from.g, from.b, from.a);
  uint32_t t = SDL_MapRGBA(fmt, to.r, to.g, to.b, to.a);
  SDL_FreeFormat(fmt);

  uint32_t* pixels = (uint32_t*)_pixels;
  int pixel_count = ( _pitch / pixel_size ) * _height;
  for( int i = 0; i < pixel_count; ++i ) {
      if( pixels[i] == f ) {
          pixels[i] = t;
      }
  }
  unlock();
}

void texture::render(SDL_Renderer* r, const point & topleft, 
                     double angle, SDL_Point* center, SDL_RendererFlip flip
                     ) const
{
  render(r, 
    rect(0, 0,  _width, _height), 
    rect(topleft.x, topleft.y,  _width, _height), 
    angle, center, flip);
}

void texture::render(SDL_Renderer* r, const rect & dst, 
                     double angle, SDL_Point* center, SDL_RendererFlip flip
                     ) const
{
  render(r, 
    rect(0, 0,  _width, _height), 
    dst, 
    angle, center, flip);
}

void texture::render(SDL_Renderer* r, const rect & src, const rect & dst, 
                     double angle, SDL_Point* center, SDL_RendererFlip flip
                     ) const
{
  if (_texture == NULL) {
    return;
  }
  //Render to screen
  if (SDL_RenderCopyEx(r, _texture, &src, &dst, angle, center, flip ) != 0)
    throw sdl_exception();
}

void texture::set_color_mod(const color & rgb)
{
  set_color_mod(rgb.r, rgb.g, rgb.b);
}

color texture::get_color_mod()
{
  color clr;
  clr.a = 255;
  if (SDL_GetTextureColorMod(_texture, &clr.r, &clr.g, &clr.b) != 0) {
    throw sdl_exception();
  }
  return clr;
}

void texture::set_color_mod(uint8_t red, uint8_t green, uint8_t blue)
{
  if (!_texture) return;
  if (SDL_SetTextureColorMod(_texture, red, green, blue) != 0)
    throw sdl_exception();
}

void texture::set_blend_mode(SDL_BlendMode blending)
{
  if (!_texture) return;
  if (SDL_SetTextureBlendMode(_texture, blending) != 0)
    throw sdl_exception();
}

SDL_BlendMode texture::get_blend_mode()
{
  if (!_texture) return SDL_BLENDMODE_NONE;
  SDL_BlendMode mode;
  if (SDL_GetTextureBlendMode(_texture, &mode) != 0)
    throw sdl_exception();
  return mode;
}

void texture::set_alpha(uint8_t alpha)
{
  if (!_texture) return;
  if (SDL_SetTextureAlphaMod(_texture, alpha) != 0)
    throw sdl_exception();
}

uint8_t texture::get_alpha()
{
  if (!_texture) return 0;
  uint8_t a = 0;
  if (SDL_GetTextureAlphaMod(_texture, &a) != 0)
    throw sdl_exception();
  return a;
}
