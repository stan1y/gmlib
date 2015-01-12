#ifndef GM_LIB_H
#define GM_LIB_H

#include <stdio.h>
#include <direct.h>

#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <exception>
#include <fstream>
#include <limits>
#include <ctime>

#include <SDL.h>
#include <SDL_config.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_log.h>

#include <dirent.h>
#include <jansson.h>

#include "GMUtil.h"

/** Default log category */
#define SDLEx_LogCategory SDL_LOG_CATEGORY_APPLICATION

/** Error log wrapper */
#define SDLEx_LogError(fmt, ...) SDL_LogError(SDLEx_LogCategory, fmt, __VA_ARGS__ );

/* Config */
struct config {
    static const int max_driver_name = 32;

    //Options
    int32_t display_width;
    int32_t display_height;
    uint32_t fps_cap;
    char* driver_name;
    char* assets_path;
    bool fullscreen;
    
    //SDL settings
    int32_t driver_index;
    uint32_t window_flags;
    uint32_t renderer_flags;

    config();
    int load_file(const char* cfg_file);
};

/* Game global state */
struct globals {
    config* conf;

    uint32_t frame_ticks;
    uint32_t last_frame_ticks;
    uint32_t elapsed;
};

globals* GM_GetGlobals();
SDL_Window* GM_GetWindow();
SDL_Renderer* GM_GetRenderer();

/* Init GMLib */
int GM_Init(const char* name, config* conf);
/* Shutdown GMLib */
void GM_Quit();
/* Start game frame, upate frame ticks and clears renderer */
void GM_StartFrame();
/* Update state, timers and animations */
void GM_UpdateFrame();
/* Draws current screen and presents renderer. */
void GM_RenderFrame();
/* End game frame, update last ticks, delay for fps cap */
void GM_EndFrame();

/* GFX */

SDL_Surface* GM_LoadSurface(const char* image);
SDL_Texture* GM_LoadTexture(const char* sheet);
TTF_Font*    GM_LoadFont(const char* font, int ptsize);
json_t*      GM_LoadJSON(const char* file);

/* Game screen */

class screen {
public:
  screen();
  virtual ~screen() {}

  /* Current game state pointer */
  static screen* current();
  static void set_current(screen* s);

  /* Game state render and update API */
  virtual void update() = 0;
  virtual void render() = 0;
  virtual void on_event(SDL_Event* ev) = 0;
};


/* Sprites & Lists */

class sprite {
public:
  /* Sprite constants */
  static const size_t invalid = _UI32_MAX; // invalid sprite index 
  static const uint8_t alpha_full = 255;   // alpha modifiers
  static const uint8_t alpha_half = 127;
  static const uint8_t alpha_none = 0;

  /* sprite index in sheet and size */
  size_t idx;
  uint32_t w;
  uint32_t h;

  /* rotation info */
  SDL_RendererFlip flip;
  int angle;

  inline bool valid() { return idx != invalid; }
  inline bool operator== (sprite& other) {
      return (idx == other.idx && w == other.w && h == other.h && flip == other.flip && angle == other.angle && sheet == other.sheet);
  }
  inline bool operator!= (sprite& other) {
      return !(*this == other);
  }

  /* create new empty sprite */
  sprite();

  /* create new sprite with texture index */
  sprite(size_t tex_idx, int px_w, int px_h, SDL_Texture* sheet);
  virtual ~sprite() {}

  /* get center point based on rect */
  SDL_Point center();

  /* render sprite with sheet at screen position rect */
  virtual void render(SDL_Point topleft, uint8_t alpha = sprite::alpha_full);

  /* create clip rect for sprite at index in given spritesheet */
  static SDL_Rect clip_rect(size_t idx, int sheet_w, int sheet_h, int sprite_w, int sprite_h);

protected:
  /* sprite sheet info */
  SDL_Texture* sheet;
  int sheet_width;
  int sheet_height;

};

/*
    Animations
*/

/* SDL_Timer-based animation index */
class anim {
public:
    /* Animation consts */
    static const uint32_t once     = 0;
    static const uint32_t repeat   = 1;
    static const uint32_t occilate = 2;

    /* list of running animations */
    static locked_array_list<anim> running;
    /* animate running items */
    static void update_running();

    /* Animation item task properties */
    bool is_running;
    sprite* target;
    uint32_t mode;
    uint32_t period_ms;
    uint32_t last_updated;
    uint32_t repeats; // 0 = repeat forever if mode != once

    inline bool operator== (anim other) {
        return (target == other.target && period_ms == other.period_ms && from == other.from && to == other.to && step == other.step && base == other.base);
    }
    inline bool operator!= (anim other) {
        return !(*this == other);
    }

    /* create new animation task */
    anim(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);

    anim();
    virtual ~anim();
    
    /* start/stop animation task */
    void start(uint32_t repeat = 0);
    void stop();

    /* animate this item */
    void update();

private:
  void init(sprite* s, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);
  void reset();

  /* Animated frame info */
  size_t base;
  size_t step;
  int from;
  int to;
  int current; //<current> = base + slide_index. slide_index => [from .. to] 
  int modifier; //animation direction. <next> = slide_index + modifier. modifier => [-1; 1]
};

#endif //GM_LIB_H