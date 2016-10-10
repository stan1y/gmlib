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
    SDLEx_LogError("GM_CreateSurface - failed to create surface %dx%d", width, height);
    throw sdl_exception();
  }
  return surface;
}

SDL_Texture* GM_CreateTexture(int width, int height, SDL_TextureAccess access, uint32_t pixel_format)
{
  SDL_Texture * tx = SDL_CreateTexture(GM_GetRenderer(), pixel_format, access, width, height);
  if (tx == NULL) {
    SDLEx_LogError("GM_CreateTexture - failed to create blank texture %dx%d.", width, height);
    throw sdl_exception();
  }
  return tx;
}

/*
  Sprites Sheet 
  */

sprites_sheet::sprites_sheet(const std::string & resource, uint32_t sprite_w, uint32_t sprite_h)
  :texture(GM_LoadSurface(resources::find(resource)),
           SDL_TEXTUREACCESS_STREAMING,
           SDL_BLENDMODE_BLEND)
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
      SDLEx_LogError("Invalid sprite size=%dx%d for sheet size=%dx%d", px_w, px_h, _sheet->width(), _sheet->height());
      throw std::exception("Invalid sprite size");
    }
    size_t total_sprites = (_sheet->width() / px_w) * (_sheet->height() / px_h);
    if (tex_idx >= total_sprites) {
      SDLEx_LogError("Invalid sprite idx=%d. total sheet length=%d", tex_idx, total_sprites);
      throw std::exception("Invalid sprite idx");
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
  uint32_t fmt = 0;
  int a = 0, sw = 0, sh = 0;
  point cnt(w / 2, h / 2);
  rect src = _sheet->get_sprite_cliprect(idx);
  src += dsrc.topleft();
  if (dsrc.w > 0 && dsrc.h > 0) {
    src.w = dsrc.w;
    src.h = dsrc.h;
  }
  _sheet->render(r, src, dst, angle, &cnt, flip); 
}


/*
    Animations
*/

/* static list of running animations */
container<anim*> anim::running = container<anim*>();
static container<anim*> g_anim_tostop = container<anim*>();
static container<anim*> g_anim_tostart = container<anim*>();

/* init new animation item */

anim::anim(size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode) :
  identifier(rand_int(1, 999)),
  is_running(false),
  mode(_mode), period_ms(_period_ms),
  last_updated(0), repeats(0),
  base(_base), step(_step),
  from(_from), to(_to),
  current(0), modifier(_step)
{
}

void anim::start(uint32_t repeat)
{
    if (!is_running && from >= 0 && to >= 0 && modifier != 0 && period_ms &&
      ( (from == 0 && to == 0) || from != to) ) {
        lock_container(g_anim_tostart);
        is_running = true;
        repeats = repeat;
        g_anim_tostart.push_back(this);
    }
}

void anim::stop()
{
    if (is_running) {
      lock_container(g_anim_tostop);
      is_running = false;
      if(g_anim_tostop.find(this) != g_anim_tostop.end()) {
        //already pending
        return;
      }
      g_anim_tostop.push_back(this);
    }
}

void anim::update()
{
    if (!is_running) {
        return;
    }

    current += modifier;
    if (from == 0 && to == 0) {
      // continue calling animation forever
      animate();
      return;
    }

    if (current <= from && modifier < 0) {
      //reached start going backwards

      //revert direction for occilate mode
      if (mode == anim::occilate) {
        if (repeats > 0 && repeats-- == 0) {
          reset();
          return;
        }
        modifier = step;
      }
      //rewind to end for repeat mode
      if (mode == anim::repeat) {
        current = to;
      }
      //done for once mode
      if (mode == anim::once) {
        reset();
        return;
      }
    }
    if (current >= to && modifier > 0) {
      //reached end going forward

      //revert direction for occilate mode
      if (mode == anim::occilate) {
        if (repeats > 0 && repeats-- == 0) {
          reset();
          return;
        }
        modifier = -1 * modifier;
      }
      //rewind to start for repeat mode
      if (mode == anim::repeat) {
        if (repeats > 0 && repeats-- == 0) {
          reset();
          return;
        }
        current = from;
      }
      //done for once mode
      if(mode == anim::once) {
        reset();
        return;
      }
    }

    if (current < 0) {
        SDLEx_LogError("current index is less than zero. base=%d step=%d modifier=%d", 
            base, step, modifier);
        throw std::exception("Next animation index is less than zero");
    }

    // call sub-class implementation
    animate();
}

void anim::update_running()
{
    // stop pending
    {
      mutex_lock guard(g_anim_tostop.mutex());
      for(size_t i = 0; i < g_anim_tostop.size(); ++i) {
        g_anim_tostop[i]->reset();
        running.remove(g_anim_tostop[i]);
      }
      g_anim_tostop.clear();
    }

    // run newly added
    {
      mutex_lock guard(g_anim_tostart.mutex());
      for(size_t i = 0; i < g_anim_tostart.size(); ++i) {
        running.push_back(g_anim_tostart[i]);
      }
      g_anim_tostart.clear();
    }

    // update running
    {
      mutex_lock guard(running.mutex());
      uint32_t ms = SDL_GetTicks();
      for(size_t i = 0; i < running.size(); ++i) {
        //check if time to run
        uint32_t delta = ms - running[i]->last_updated;
        if ( delta >= running[i]->period_ms) {
            running[i]->update();
            running[i]->last_updated = ms;
        }
      }
    }
}

/**
  Sprite Animation
  */

void sprite_anim::init(sprite** ss, size_t _count)
{
  is_running = false;
  target = (sprite**)calloc(_count, sizeof(sprite*));
  for (size_t i = 0; i < _count; ++i) {
    target[i] = ss[i];
  }
  targets_count = _count;
}

sprite_anim::sprite_anim(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode):
  anim(0, _from, _to, _step, _period_ms, _mode)
{
  init(&s, 1);
}

sprite_anim::sprite_anim(sprite** s, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode):
  anim(0, _from, _to, _step, _period_ms, _mode)
{
  init(s, _count);
}

void sprite_anim::release_targets()
{
  free(target);
}

void sprite_anim::reset()
{
  anim::reset();
  // reset indexes of target(s)
  for (size_t target_idx = 0; target_idx < targets_count; ++target_idx) {
    target[target_idx]->idx = from;
  }
}

void sprite_anim::animate()
{
  // apply index to target(s)
  for (size_t target_idx = 0; target_idx < targets_count; ++target_idx) {
    target[target_idx]->idx = base + current;
  }
}