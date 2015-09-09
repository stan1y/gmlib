#include "GMSprites.h"

/* helpers */

SDL_Surface* GM_CreateSurface(int width, int height) 
{
  int bpp = 0;
  uint32_t rmask = 0, gmask = 0, bmask = 0, amask = 0;
  SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGBA8888, &bpp, &rmask, &gmask, &bmask, &amask);
  SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
  if(surface == NULL) {
    SDLEx_LogError("SDLEx_CreateSurface failed to create surface %dx%d", width, height);
    throw std::exception(SDL_GetError());
  }
  return surface;
}

SDL_Texture* GM_CreateTexture(int width, int height, SDL_TextureAccess access)
{
  return SDL_CreateTexture(GM_GetRenderer(), SDL_PIXELFORMAT_RGBA8888, access, width, height);
}

/*
  Sprites Sheet 
  */

sprites_sheet::sprites_sheet(const std::string & res, uint32_t sprite_w, uint32_t sprite_h)
  :texture(res)
{
  _cols = width() / sprite_w;
  _rows = height() / sprite_h;
}

rect sprites_sheet::get_sprite_cliprect(size_t idx)
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

sprite::sprite(size_t tex_idx, int px_w, int px_h, sprites_sheet* sheet)
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

void sprite::render(SDL_Renderer * r, point & topleft, uint8_t alpha) const
{
  if (_sheet == nullptr || _sheet->get_texture() == NULL || w == 0 || h == 0) {
      return;
  }
  uint32_t fmt = 0;
  int a = 0, sw = 0, sh = 0;
  point cnt(w / 2, h / 2);
  rect src = _sheet->get_sprite_cliprect(idx);
  rect dst = rect(topleft.x, topleft.y, w, h);
  _sheet->set_alpha(alpha);
  _sheet->render(r, src, dst, angle, &cnt, flip); 
}

void sprite::render(SDL_Renderer * r, rect & dst, uint8_t alpha) const
{
  if (_sheet == nullptr || _sheet->get_texture() == NULL || w == 0 || h == 0) {
      return;
  }
  uint32_t fmt = 0;
  int a = 0, sw = 0, sh = 0;
  point cnt(w / 2, h / 2);
  rect src = _sheet->get_sprite_cliprect(idx);
  _sheet->set_alpha(alpha);
  _sheet->render(r, src, dst, angle, &cnt, flip); 
}


/*
    Sprite animation
*/

/* static list of running animations */
locked_vector<anim*> anim::running = locked_vector<anim*>();
static locked_vector<anim*> _GM_anim_tostop = locked_vector<anim*>();
static locked_vector<anim*> _GM_anim_tostart = locked_vector<anim*>();

/* init new animation item */

void anim::init(sprite** ss, size_t _count,  size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode)
{
  is_running = false;
  target = (sprite**)calloc(_count, sizeof(sprite*));
  for(size_t i = 0; i < _count; ++i) {
    target[i] = ss[i];
  }
  targets_count = _count;
  base = _base;
  current = 0;
  from = _from; 
  to = _to;
  step = modifier = _step;
  mode = _mode;
  period_ms = _period_ms; 
  last_updated = 0;
}

anim::anim(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode)
{
    init(&s, 1,  s == nullptr ? 0 : s->idx, _from, _to, _step, _period_ms, _mode);
}

anim::anim(sprite** s, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode)
{
    init(s, _count, _base, _from, _to, _step, _period_ms, _mode);
}

void anim::release_targets()
{
  free(target);
}

void anim::start(uint32_t repeat)
{
    _GM_anim_tostart.lock();
    {
        if (!is_running && from >= 0 && to >= 0 && from < to && modifier != 0 && period_ms && target != nullptr) {
            is_running = true;
            repeats = repeat;
            _GM_anim_tostart.push_back(this);
        }
    }
    _GM_anim_tostart.unlock();
}

void anim::stop()
{
    _GM_anim_tostop.lock();
    {
        if (is_running) {
            is_running = false;
            if(_GM_anim_tostop.find(this) != _GM_anim_tostop.end()) {
                //already pending
                return;
            }
            _GM_anim_tostop.push_back(this);
        }
    }
    _GM_anim_tostop.unlock();
}

void anim::reset()
{
  stop();
  for(size_t target_idx = 0; target_idx < targets_count; ++target_idx) {
    target[target_idx]->idx = from;
  }
}

void anim::update()
{
    if (!is_running) {
        return;
    }

    current += modifier;
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

    for(size_t target_idx = 0; target_idx < targets_count; ++target_idx) {
      target[target_idx]->idx = base + current;
    }
}

void anim::update_running()
{
    //stop pending
    _GM_anim_tostop.lock();
    {
      for(size_t i = 0; i < _GM_anim_tostop.size(); ++i) {
        for(size_t target_idx = 0; target_idx < _GM_anim_tostop[i]->targets_count; ++target_idx) {
          _GM_anim_tostop[i]->target[target_idx]->idx = _GM_anim_tostop[i]->base;
        }
        running.remove(_GM_anim_tostop[i]);
      }
      _GM_anim_tostop.clear();
    }
    _GM_anim_tostop.unlock();

    //run newly added
    _GM_anim_tostart.lock();
    {
      for(size_t i = 0; i < _GM_anim_tostart.size(); ++i) {
        running.push_back(_GM_anim_tostart[i]);
      }
      _GM_anim_tostart.clear();
    }
    _GM_anim_tostart.unlock();

    //update running
    running.lock();
    {
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
    running.unlock();
}