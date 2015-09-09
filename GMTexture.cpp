#include "GMTexture.h"

/* Texture */

texture::texture()
{
  _texture = NULL;
  _pixels = NULL;
  _pitch = 0;
  _width = 0;
  _height = 0;
}

texture::texture(SDL_Texture * tx)
{
  set_texture(tx);
}

texture::texture(const std::string & resource_path)
{
  load(resource_path);
}

texture::texture(SDL_Surface* src, SDL_TextureAccess access, SDL_BlendMode bmode)
{
  load_surface(src, access, bmode);
}

texture::texture(int w, int h, SDL_TextureAccess access, SDL_BlendMode bmode)
{
  blank(w, h, access, bmode);
}

void texture::set_texture(SDL_Texture * tx)
{
  release();
  _texture = tx;
  uint32_t fmt = 0;
  int access = 0;
  SDL_QueryTexture(tx, &fmt, &access, &_width, &_height);
}

void texture::blank(int w, int h, SDL_TextureAccess access, SDL_BlendMode bmode)
{
  release();
  _texture = GM_CreateTexture(w, h, access);
  SDL_SetTextureBlendMode(_texture, bmode);
  
  if (_texture == NULL) {
    SDLEx_LogError("texture::blank - Failed to create blank texture.");
    throw std::exception("Failed to create blank texture");
  }
  _width = w; _height = h;
}

void texture::release()
{
  if (_texture) 
    SDL_DestroyTexture(_texture);
  _texture = NULL;
}

texture::~texture()
{
  release();
}

void texture::load(const std::string& path)
{
  SDL_Surface* src = GM_LoadSurface(path);
  load_surface(src, SDL_TEXTUREACCESS_STREAMING, SDL_BLENDMODE_BLEND);
  
  if (path.find(".png") == std::string::npos) {
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

void texture::apply_text(SDL_Surface * s)
{
  if (s == NULL) {
    SDLEx_LogError("texture::load_text Failed to render text: %s", TTF_GetError());
    throw std::exception("Failed to render text");
  }
  SDL_Surface* converted = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA8888, 0);
  load_surface(converted, SDL_TEXTUREACCESS_STREAMING, SDL_BLENDMODE_BLEND);
  SDL_FreeSurface(converted);
  SDL_FreeSurface(s);
}

void texture::load_text_solid(const std::string& text, TTF_Font* font, const color & c)
{
  apply_text(TTF_RenderText_Solid(font, text.c_str(), c));
}

void texture::load_text_blended(const std::string& text, TTF_Font* font, const color & c)
{
  apply_text(TTF_RenderText_Blended(font, text.c_str(), c));
}

void texture::load_surface(SDL_Surface* src, SDL_TextureAccess access, SDL_BlendMode bmode)
{
  blank(src->w, src->h, access, bmode);
  //streaming texture has user-memory pixels
  if (access == SDL_TEXTUREACCESS_STREAMING) {
    lock();
    memcpy(_pixels, src->pixels, src->pitch * src->h);
    //apply _pixles to the texture
    unlock();
  }
}

void texture::lock()
{
  if (_pixels != NULL) {
    //already locked
    return;
  }
  if (SDL_LockTexture(_texture, NULL, &_pixels, &_pitch) != 0) {
    SDLEx_LogError("texture::lock - failed to lock texture: %s", SDL_GetError());
    throw std::exception("failed to lock texture");
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

void texture::replace_color(SDL_Color& from, SDL_Color& to)
{
  lock();
  SDL_PixelFormat* fmt = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
  uint32_t f = SDL_MapRGBA(fmt, from.r, from.g, from.b, from.a);
  uint32_t t = SDL_MapRGBA(fmt, to.r, to.g, to.b, to.a);
  SDL_FreeFormat(fmt);

  uint32_t* pixels = (uint32_t*)_pixels;
  int pixel_count = ( _pitch / 4 ) * _height;
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
  SDL_RenderCopyEx(r, _texture, &src, &dst, angle, center, flip );
}

void texture::set_color(uint8_t red, uint8_t green, uint8_t blue)
{
  if (!_texture) return;
  SDL_SetTextureColorMod(_texture, red, green, blue);
}

void texture::set_blend_mode(SDL_BlendMode blending)
{
  if (!_texture) return;
  SDL_SetTextureBlendMode(_texture, blending);
}

void texture::set_alpha(uint8_t alpha)
{
  if (!_texture) return;
  SDL_SetTextureAlphaMod(_texture, alpha);
}
