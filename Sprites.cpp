#include "GMLib.h"

/*
    Load surfaces and sprite sheets 
*/

SDL_Surface* GM_LoadSurface(const char* image)
{
    SDL_Surface *s = IMG_Load(image);

    if (!s) {
      SDLEx_LogError("LoadSurface: failed to load %s: %s", image, SDL_GetError());
      return nullptr;
    }
    return s;
}

SDL_Texture* GM_LoadTexture(const char* sheet)
{
    SDL_Surface* s = GM_LoadSurface(sheet);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(GM_GetRenderer(), s);
    SDL_FreeSurface(s);
    return tex;
}

/*
    Sprite class
*/

sprite::sprite()
{
  w = 0; h = 0;
  idx = sprite::invalid;
  sheet = nullptr;
  flip = SDL_FLIP_NONE;
  angle = 0;
}

sprite::sprite(size_t tex_idx, int px_w, int px_h, SDL_Texture* _sheet)
{
    idx = tex_idx;
    w = px_w; h = px_h;
    flip = SDL_FLIP_NONE;
    angle = 0;
    sheet = nullptr;
    sheet_width = 0; sheet_height = 0;

    if (_sheet != nullptr) {
        sheet = _sheet;

        //query sprites sheet
        uint32_t fmt = 0; int access = 0;
        SDL_QueryTexture(sheet, &fmt, &access, &sheet_width, &sheet_height);
        if ( sheet_width % px_w != 0 || sheet_height % px_h != 0 ) {
            SDLEx_LogError("Invalid sprite size=%dx%d for sheet size=%dx%d", px_w, px_h, sheet_width, sheet_height);
            throw std::exception("Invalid sprite size");
        }
        size_t total_sprites = (sheet_width / px_w) * (sheet_height / px_h);
        if (tex_idx >= total_sprites) {
            SDLEx_LogError("Invalid sprite idx=%d. total sheet length=%d", tex_idx, total_sprites);
            throw std::exception("Invalid sprite idx");
        }
    }
}

SDL_Rect sprite::clip_rect(size_t idx, int sheet_w, int sheet_h, int sprite_w, int sprite_h) {
    SDL_Rect clip;
    size_t columns = sheet_w / sprite_w;
    int rows = sheet_h / sprite_h;
    int row_idx = 0;
    while(idx >= columns) {
        idx -= columns;
        row_idx += 1;
    }
    clip.x = idx * sprite_w;
    clip.y = row_idx * sprite_h;
    clip.w = sprite_w;
    clip.h = sprite_h;
    return clip;
}

void sprite::render(SDL_Point topleft, uint8_t alpha) 
{
    if (sheet == nullptr || w == 0 || h == 0) {
        return;
    }
    uint32_t fmt = 0;
    int a = 0, sw = 0, sh = 0;
    SDL_Point cnt = {
        w / 2, h / 2
    };
    SDL_Rect clip = clip_rect(idx, sheet_width, sheet_height, w, h);
    SDL_SetTextureAlphaMod(sheet, alpha);
    SDL_Rect rect = { topleft.x, topleft.y, w, h };
    SDL_RenderCopyEx(GM_GetRenderer(), sheet, &clip, &rect, angle, &cnt, flip);
}

/*
    Sprite animation
*/

/* static list of running animations */
locked_array_list<anim> anim::running = locked_array_list<anim>();
static locked_array_list<anim> _GM_anim_tostop = locked_array_list<anim>();
static locked_array_list<anim> _GM_anim_tostart = locked_array_list<anim>();

/* init new animation item */
void anim::init(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode)
{
  is_running = false;
  target = s;
  base = s == nullptr ? 0 : s->idx;
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
    init(s, _from, _to, _step, _period_ms, _mode);
}

anim::anim()
{
    init(nullptr, 0, 0, 0, 0, 0);
}

anim::~anim()
{
}

void anim::start(uint32_t repeat)
{
    _GM_anim_tostart.lock();
    {
        if (!is_running && from >= 0 && to >= 0 && from < to && modifier != 0 && period_ms && target != nullptr) {
            is_running = true;
            repeats = repeat;
            _GM_anim_tostart.append(*this);
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
            if(_GM_anim_tostop.find(*this) != NULL) {
                //already pending
                return;
            }
            _GM_anim_tostop.append(*this);
        }
    }
    _GM_anim_tostop.unlock();
}

void anim::reset()
{
  stop();
  target->idx = from;
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

    target->idx = base + current;
}

void anim::update_running()
{
    globals* gm = GM_GetGlobals();

    //stop pending
    _GM_anim_tostop.lock();
    {
      for(size_t i = 0; i < _GM_anim_tostop.length(); ++i) {
        running.remove(_GM_anim_tostop.at(i));
      }
      _GM_anim_tostop.clear();
    }
    _GM_anim_tostop.unlock();

    //run newly added
    _GM_anim_tostart.lock();
    {
      for(size_t i = 0; i < _GM_anim_tostart.length(); ++i) {
        running.append(_GM_anim_tostart.at(i));
      }
      _GM_anim_tostart.clear();
    }
    _GM_anim_tostart.unlock();

    //update running
    running.lock();
    {
      for(size_t i = 0; i < running.length(); ++i) {
        //check if time to run
        uint32_t delta = gm->frame_ticks - running.at(i).last_updated;
        if ( delta >= running.at(i).period_ms) {
            running.at(i).update();
            running.at(i).last_updated = gm->frame_ticks;
        }
      }
    }
    running.unlock();
}