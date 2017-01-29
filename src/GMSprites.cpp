#include "GMSprites.h"
#include "GMUtil.h"
#include "RESLib.h"

/* helpers */

SDL_Surface* GM_CreateSurface(int width, int height) 
{
  int bpp = 0;
  uint32_t rmask = 0, gmask = 0, bmask = 0, amask = 0;
  SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &rmask, &gmask, &bmask, &amask);
  SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
  if(surface == NULL) {
    SDL_Log("GM_CreateSurface - failed to create surface %dx%d", width, height);
    throw sdl_exception();
  }
  return surface;
}

SDL_Texture* GM_CreateTexture(int width, int height, SDL_TextureAccess access, uint32_t pixel_format)
{
  SDL_Texture * tx = SDL_CreateTexture(GM_GetRenderer(), pixel_format, access, width, height);
  if (tx == NULL) {
    SDL_Log("GM_CreateTexture - failed to create blank texture %dx%d.", width, height);
    throw sdl_exception();
  }
  return tx;
}

/*
  Sprites Sheet 
  */

sprites_sheet::sprites_sheet(const std::string & file_path, uint32_t sprite_w, uint32_t sprite_h)
  :texture(GM_LoadSurface(file_path), SDL_BLENDMODE_BLEND, true)
{
  _cols = width() / sprite_w;
  _rows = height() / sprite_h;
}

rect sprites_sheet::get_sprite_cliprect(size_t idx) const
{
  rect clip;
  int row_idx = 0;
  uint32_t sprite_w = sprite_width();
  uint32_t sprite_h = sprite_height();
  while(idx >= _cols) {
      idx -= _cols;
      row_idx += 1;
  }
  clip.x = idx * sprite_w;
  clip.y = row_idx * sprite_h;
  clip.w = sprite_w;
  clip.h = sprite_h;
  return clip;
}

/*
    Sprite class
*/

sprite::sprite()
{
  w = 0; h = 0;
  idx = sprite::invalid;
  _sheet = nullptr;
  flip = SDL_FLIP_NONE;
  angle = 0;
}

sprite::sprite(size_t tex_idx, int px_w, int px_h, sprites_sheet const* sheet)
{
  idx = tex_idx;
  w = px_w; h = px_h;
  flip = SDL_FLIP_NONE;
  angle = 0;
  _sheet = nullptr;

  if (sheet != nullptr) {
    _sheet = sheet;

    //check sprites sheet
    if ( _sheet->width() % px_w != 0 || _sheet->height() % px_h != 0 ) {
      SDL_Log("Invalid sprite size=%dx%d for sheet size=%dx%d", px_w, px_h, _sheet->width(), _sheet->height());
      throw std::runtime_error("Invalid sprite size");
    }
    size_t total_sprites = (_sheet->width() / px_w) * (_sheet->height() / px_h);
    if (tex_idx >= total_sprites) {
      SDL_Log("Invalid sprite idx=%zu. total sheet length=%zu", tex_idx, total_sprites);
      throw std::runtime_error("Invalid sprite idx");
    }
  }
}

void sprite::render(SDL_Renderer * r, const point & dst_pnt) const
{
  render(r, rect(), dst_pnt); 
}

void sprite::render(SDL_Renderer * r, const rect & dst) const
{
  render(r, rect(), dst);
}

void sprite::render(SDL_Renderer * r, const rect & dsrc, const point & dst_pnt) const
{
  render(r, dsrc, rect( dst_pnt.x, dst_pnt.y, w, h));
}

void sprite::render(SDL_Renderer * r, const rect & dsrc, const rect & dst) const
{
  if (_sheet == nullptr || _sheet->get_texture() == NULL || w == 0 || h == 0) {
      return;
  }
  point cnt(w / 2, h / 2);
  rect src = _sheet->get_sprite_cliprect(idx);
  src += dsrc.topleft();
  if (dsrc.w > 0 && dsrc.h > 0) {
    src.w = dsrc.w;
    src.h = dsrc.h;
  }
  _sheet->render(r, src, dst, angle, &cnt, flip);
}