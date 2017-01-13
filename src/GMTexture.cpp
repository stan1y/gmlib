#include "GMTexture.h"

/* Texture */

texture::texture():
  _texture(nullptr),
  _pixels(nullptr),
  _pitch(0),
  _width(0),
  _height(0),
  _scale_w(0),
  _scale_h(0),
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
  _scale_w(0),
  _scale_h(0),
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
  _scale_w(0),
  _scale_h(0),
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
  _scale_w(0),
  _scale_h(0),
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
  _scale_w(0),
  _scale_h(0),
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
  _access = access;
  _bmode = bmode;
  _format = pixel_format;
}

void texture::release()
{
  if (_texture != nullptr) {
    SDL_DestroyTexture(_texture);
  }
  _texture = nullptr;
}

texture::~texture()
{
  release();
}

void texture::load(const fs::path & file_path)
{
  SDL_Surface* src = GM_LoadSurface(file_path.string());
  convert_surface(src);

  if (file_path.extension() != ".png") {
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

void texture::load_text_solid(const std::string& text, ttf_font const * font, const color & c)
{
  convert_surface(TTF_RenderText_Solid(font->fnt(), text.c_str(), c));
}

void texture::load_text_blended(const std::string& text, ttf_font const * font, const color & c)
{
  convert_surface(TTF_RenderText_Blended(font->fnt(), text.c_str(), c));
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

texture * texture::copy()
{
  lock();
  texture * cp = new texture(_width, _height, _access, _bmode, _format); 
  cp->set_pixels(_pixels);
  unlock();

  return cp;
}

void texture::resize(int dw, int dh)
{
  _scale_w += dw;
  _scale_h += dh;
}

void texture::set_pixels(void * ptr)
{
  lock();
  _pixels = ptr;
  unlock();
}

void texture::move_texture(texture * other)
{
  other->release();
  other->set_texture(_texture);
  other->_width = _width;
  other->_height = _height;
  other->_bmode = _bmode;
  other->_access = _access;
  other->_format = _format;
  _texture = NULL;
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


/* Class multi_texture implementation */

multi_texture::multi_texture(const rect & size):
  _width(size.w), 
  _height(size.h)
{
  init();
}

multi_texture::multi_texture(const rect & size, int fw, int fh):
  _width(size.w), 
  _height(size.h)
{
  init(fw, fh);
}

multi_texture::multi_texture(int w, int h, int fw, int fh):
  _width(w),
  _height(h)
{
  init(fw, fh);
}

void multi_texture::init(int fw, int fh)
{
  uint32_t fragments_w = 0;
  uint32_t fragments_h = 0;

  if (fw == 0 && fh == 0) {
    fragments_w = fragment::fragment_max_width;
    fragments_h = fragment::fragment_max_height;
    fw = _width / fragments_w;
    if (_width % fragments_w != 0) fw += 1;
    fh = _height / fragments_h;
    if (_height % fragments_h != 0) fh += 1;
  }
  else {
    fragments_w = _width / fw;
    fragments_h = _height / fh;
  }

  if (fragments_w % 2 != 0 || fragments_h % 2 != 0) {
    SDLEx_LogError("%s - fragment size [%d, %d] is not a power of 2",
      __METHOD_NAME__,
      fragments_w,
      fragments_h);
    throw std::exception("Invalid multi_texture:fragment size - not a power of 2");
  }

  for(int ix = 0; ix < fw; ++ix) {
    for(int iy = 0; iy < fh; ++iy) {
      rect fragment_rect(ix * fragments_w, 
                         iy * fragments_h,
                         fragments_w,
                         fragments_h);

      // if this fragment seems to be last in a row/column and too large
      // trim it's size to fit into [width, height] of multi_texture
      if (fragment_rect.x + fragment_rect.w > _width) {
        fragment_rect.w = _width - fragment_rect.x;
      }
      if (fragment_rect.y + fragment_rect.h > _height) {
        fragment_rect.h = _height - fragment_rect.y;
      }
      _fragments.push_back(new fragment(fragment_rect));
    }
  }
#ifdef GM_DEBUG_MULTI_TEXTURE
  SDL_Log("%s - created %d fragments of size (%d, %d), total size is (%d, %d)",
    __METHOD_NAME__,
    _fragments.size(),
    fragments_w,
    fragments_h,
    _width,
    _height);
#endif
}

void multi_texture::render(SDL_Renderer * r, const point & at)
{
  container<fragment*>::iterator it = _fragments.begin();
  for(; it != _fragments.end(); ++it) {
    fragment * f = *it;
    point draw_at = at + f->pos().topleft();
    rect fragment_rect(draw_at.x, draw_at.y, f->pos().w, f->pos().h);
    f->get_texture().render(r, fragment_rect);
#ifdef GM_DEBUG_MULTI_TEXTURE
    color::yellow().apply(r);
    SDL_RenderDrawRect(r, &fragment_rect);
#endif
  }
}

void multi_texture::render_texture(SDL_Renderer * r, const texture & tx, const point & at)
{
  lock_container(_fragments);
  rect texture_collide_rect(at.x, at.y, tx.width(), tx.height());
  container<fragment*>::iterator it = _fragments.begin();
  for(; it != _fragments.end(); ++it) {
    fragment * f = *it;
    const rect & fpos = f->pos();
    // check if texture collides with this fragment and clip it's rect if yes
    if (fpos.collide_rect(texture_collide_rect)) {
      rect clipped = texture_collide_rect.clip(fpos);
      // render into the fragment's texture with clipped rect
      {
        rect src(clipped.x - at.x, clipped.y - at.y, clipped.w, clipped.h);
        rect dst(clipped.x - fpos.x, clipped.y - fpos.y, clipped.w, clipped.h);
        texture::render_context context(&f->get_texture(), r);
        tx.render(r, src, dst);
      }
    }
  }
}

multi_texture::~multi_texture()
{
}