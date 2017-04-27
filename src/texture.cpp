#include "texture.h"

/* Texture */

texture::texture():
  _texture(nullptr),
  _width(0),
  _scale_w(0),
  _height(0),
  _scale_h(0),
  _pixels(nullptr),
  _pitch(0),
  _format(SDL_PIXELFORMAT_RGBA8888),
  _access(SDL_TEXTUREACCESS_STATIC),
  _bmode(SDL_BLENDMODE_BLEND)
{
}

texture::texture(SDL_Texture * tx, SDL_BlendMode bmode):
  _texture(nullptr),
  _width(0),
  _scale_w(0),
  _height(0),
  _scale_h(0),
  _pixels(nullptr),
  _pitch(0),
  _format(SDL_PIXELFORMAT_RGBA8888),
  _access(SDL_TEXTUREACCESS_STATIC),
  _bmode(bmode)
{
  set_texture(tx);
}

texture::texture(SDL_Renderer *r,
                 SDL_Surface* src,
                 SDL_BlendMode bmode,
                 uint32_t pixel_format):
  _texture(nullptr),
  _width(0),
  _scale_w(0),
  _height(0),
  _scale_h(0),
  _pixels(nullptr),
  _pitch(0),
  _format(pixel_format),
  _access(SDL_TEXTUREACCESS_STATIC),
  _bmode(bmode)

{
  set_surface(src, r); 
}


texture::texture(const std::string & file_path):
  _texture(nullptr),
  _width(0),
  _scale_w(0),
  _height(0),
  _scale_h(0),
  _pixels(nullptr),
  _pitch(0),
  _format(SDL_PIXELFORMAT_RGBA8888),
  _access(SDL_TEXTUREACCESS_STATIC),
  _bmode(SDL_BLENDMODE_BLEND)
{
  load(file_path);
}

texture::texture(int w, int h,
                 SDL_TextureAccess access,
                 SDL_BlendMode bmode,
                 uint32_t pixel_format):
  _texture(nullptr),
  _width(0),
  _scale_w(0),
  _height(0),
  _scale_h(0),
  _pixels(nullptr),
  _pitch(0),
  _format(pixel_format),
  _access(access),
  _bmode(bmode)
{
  blank(w, h, access, bmode, pixel_format);
}


texture::texture(SDL_Surface* src,
                 SDL_BlendMode bmode,
                 bool convert_transparency):
    _texture(nullptr),
    _width(0),
    _scale_w(0),
    _height(0),
    _scale_h(0),
    _pixels(nullptr),
    _pitch(0),
    _format(SDL_PIXELFORMAT_RGBA8888),
    _access(SDL_TEXTUREACCESS_STREAMING),
    _bmode(bmode)
{
  blank(src->w, src->h, SDL_TEXTUREACCESS_STREAMING, bmode);
  set_pixels(src->pixels);
  if (convert_transparency) {
    /*uint32_t pixel = SDLEx_GetPixel(src, 0, 0);
    uint8_t r = 0, g = 0, b = 0;
    SDL_GetRGB(pixel, src->format, &r, &g, &b);
    replace_color(color(r, g, b, 255), color(0, 0, 0, 0));*/
  }
}

void texture::set_texture(SDL_Texture *tx)
{
  if (tx == nullptr) {
    SDL_Log("%s: null texture given", __METHOD_NAME__);
    throw std::runtime_error("texture::set_texture - null texture given");
  }

  release();
  _texture = tx;

  SDL_QueryTexture(_texture, &_format, (int*)&_access, &_width, &_height);
  SDL_SetTextureBlendMode(_texture, _bmode);
}

void texture::set_surface(SDL_Surface *src, SDL_Renderer *r)
{
  if (src == NULL) {
    SDL_Log("%s: null surface given", __METHOD_NAME__);
    throw std::runtime_error("texture::texture - null surface given");
  }
  if (r == NULL)
    r = GM_GetRenderer();

  SDL_Surface *converted = SDL_ConvertSurfaceFormat(src, _format, 0);
  SDL_Texture *tx = SDL_CreateTextureFromSurface(r, converted);
  if (tx == NULL) {
    SDL_FreeSurface(converted);
    throw sdl_exception();
  }
  SDL_FreeSurface(converted);
  set_texture(tx);
}

void texture::blank(int w, int h, SDL_TextureAccess access, SDL_BlendMode bmode, uint32_t pixel_format)
{
  release();
  _access = access;
  _format = pixel_format;
  _bmode = bmode;
  _width = w;
  _height = h;

  _texture = GM_CreateTexture(w, h, access, pixel_format);
  if (_texture == NULL) {
    throw sdl_exception();
  }
  SDL_SetTextureBlendMode(_texture, _bmode);
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

void texture::load(const std::string & file_path)
{
  SDL_Surface *loaded = GM_LoadSurface(media_path(file_path));
  set_surface(loaded);
  SDL_FreeSurface(loaded);
}

texture * texture::copy_pixels()
{
  if (_access != SDL_TEXTUREACCESS_STREAMING)
    throw std::runtime_error("texture::copy_pixels - this texture is not SDL_TEXTUREACCESS_STREAMING");

  lock();
  texture * cp = new texture(_width, _height,
    SDL_TEXTUREACCESS_STREAMING,
    _bmode,
    _format);
  cp->set_pixels(_pixels);
  unlock();

  return cp;
}

void texture::set_pixels(void *pixels)
{
  if (_access != SDL_TEXTUREACCESS_STREAMING) {
    throw std::runtime_error("texture::set_pixels - this texture is not SDL_TEXTUREACCESS_STREAMING");
  }
  lock();
  memcpy(_pixels, pixels, _pitch * _height);
  unlock();
}

void texture::clone(texture * other)
{
  other->release();
  other->_bmode = _bmode;
  other->set_texture(_texture);
  _texture = NULL;
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
  if (_format != SDL_PIXELFORMAT_RGBA8888) {
    SDL_Log("%s - unsupported pixel format (%d) for this method. Only SDL_PIXELFORMAT_RGBA8888 is supported.",
      __METHOD_NAME__, _format);
    throw std::runtime_error("Unsupported pixel format for method");
  }
  // SDL_PIXELFORMAT_RGBA8888 is 8 bits long each
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
    SDL_Log("%s - fragment size [%d, %d] is not a power of 2",
      __METHOD_NAME__,
      fragments_w,
      fragments_h);
    throw std::runtime_error("Invalid multi_texture:fragment size - not a power of 2");
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
#ifdef GM_DEBUG
  SDL_Log("%s - created %zu fragments of size (%d, %d), total size is (%d, %d)",
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
#ifdef GM_DEBUG
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
        texture::render_context ctx(&f->get_texture(), r);
        ctx.clear_target();
        tx.render(r, src, dst);
      }
    }
  }
}

multi_texture::~multi_texture()
{
}
