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
  bool calculate_fps;
    
  //SDL settings
  int32_t driver_index;
  uint32_t window_flags;
  uint32_t renderer_flags;

  config();
  int load_file(const char* cfg_file);
};

/* Ticks Timer */
class timer {
public:
  timer();

  void start();
  void stop();
  void pause();
  void unpause();
  uint32_t get_ticks();
  bool is_started() { return _started; }
  bool is_paused() { return _started && _paused; }

private:
  uint32_t _started_ticks;
  uint32_t _paused_ticks;
  bool _started;
  bool _paused;
};

/* Init GMLib */
int GM_Init(const char* name, config* conf);
/* Shutdown GMLib */
void GM_Quit();
/* Get pointer to applied configuration */
const config* GM_GetConfig(); 
/* Get main SDL Window */
SDL_Window* GM_GetWindow();
/* Get rendered attached to main window */
SDL_Renderer* GM_GetRenderer();

/* Start game frame, upate frame ticks and clears renderer */
void GM_StartFrame();
/* Update state, timers and animations */
void GM_UpdateFrame();
/* Draws current screen and presents renderer. */
void GM_RenderFrame();
/* End game frame, update last ticks, delay for fps cap */
void GM_EndFrame();

/* Get miliseconds elapsed since start of the current frame */
uint32_t GM_GetFrameTicks();
/* Get average FPS */
float GM_GetAvgFps();

SDL_Surface* GM_CreateSurface(int width, int height);
SDL_Texture* GM_CreateTexture(int width, int height, SDL_TextureAccess access);

/* load resources from assets path with help of RES_GetFullPath */
SDL_Surface* GM_LoadSurface(const std::string& image);
SDL_Texture* GM_LoadTexture(const std::string& sheet);
TTF_Font*    GM_LoadFont(const std::string& font, int ptsize);
json_t*      GM_LoadJSON(const std::string& file);

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

/*
  Texture wrapper 
  with SDL_TEXTUREACCESS_STREAMING access
  for pixel manipulations
*/
class texture {
public:
  texture();
  ~texture();
  void release();

  /* load texture from resource (see GM_LoadTexture) */
  void load(const std::string& path);
  /* load texture data from surface */
  void load_surface(SDL_Surface* src);
  /* load texture data by rendering text with font */
  void load_text(const std::string& text, TTF_Font* font, SDL_Color& color);
  /* set color modulation */
  void set_color(uint8_t red, uint8_t green, uint8_t blue);
  /* set blending mode */
  void set_blend_mode(SDL_BlendMode blending);
  /* set alpha opacity */
  void set_alpha(uint8_t alpha);

  /* access texture properties */
  int get_width() { return _width; }
  int get_height() { return _height; }
  SDL_Texture* get_texture() { return _texture; }

  /* render it */
  void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);

  /* pixels access */
  bool lock();
  bool unlock();
  int get_pitch() { return _pitch; }
  void* get_pixels() { return _pixels; }
  void replace_color(SDL_Color& from, SDL_Color& to);

private:
  SDL_Texture* _texture;
  int _width;
  int _height;
  void* _pixels;
  int _pitch;
};

/* Sprite */

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
      return (idx == other.idx && w == other.w && h == other.h && flip == other.flip && angle == other.angle && _sheet == other._sheet);
  }
  inline bool operator!= (sprite& other) {
      return !(*this == other);
  }

  /* create new empty sprite */
  sprite();

  /* create new sprite with texture index */
  sprite(size_t tex_idx, int px_w, int px_h, texture* sheet);
  virtual ~sprite() {}

  /* get center point based on rect */
  SDL_Point center();

  /* render sprite with sheet at screen position rect */
  virtual void render(SDL_Point topleft, uint8_t alpha = sprite::alpha_full);

  /* create clip rect for sprite at index in given spritesheet */
  static SDL_Rect clip_rect(size_t idx, int sheet_w, int sheet_h, int sprite_w, int sprite_h);

protected:
  /* sprites sheet*/
  texture* _sheet;
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
    static locked_vector<anim*> running;
    /* animate running items */
    static void update_running();

    /* Sprite(s) to animate */
    sprite** target;
    size_t targets_count;

    /* Animation item task properties */
    bool is_running;
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
    anim(sprite** ss, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);

    virtual ~anim() {};
    void release_targets();
    
    /* start/stop animation task */
    void start(uint32_t repeat = 0);
    void stop();

    /* animate this item */
    void update();

private:
  void init(sprite** s, size_t _count, size_t _base, size_t _from, size_t _to, size_t _step, uint32_t _period_ms, uint32_t _mode);
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