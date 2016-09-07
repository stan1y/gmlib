#ifndef GM_LIB_H
#define GM_LIB_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <cctype>

#include <SDL.h>
#include <SDL_config.h>
#include <SDL_render.h> 
#include <SDL_video.h> 
#include <SDL_opengl.h> 
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_log.h>
#include <SDLEx.h>

#include "GMUtil.h"

/** Default log category */
#define SDLEx_LogCategory SDL_LOG_CATEGORY_APPLICATION

/** Error log wrapper */
#define SDLEx_LogError(fmt, ...) SDL_LogError(SDLEx_LogCategory, fmt, __VA_ARGS__ );

/* Global Config */
struct config {
public:
  const rect          screen_rect() const;
  const std::string   driver_name() const;
  const int32_t       driver_index() const;
  const std::string   assets_path() const;
  const bool          fullscreen() const;
  const bool          calculate_fps() const;
  const int           fps_cap() const;
  const uint32_t      window_flags() const;
  const uint32_t      renderer_flags() const;
  const std::string   python_home() const;

  static int load(const std::string & cfg_file);
};

/* Ticks Timer */
class timer {
public:
  timer();

  void start();
  void stop();
  void pause();
  void unpause();
  uint32_t get_ticks() const;
  bool is_started() const { return _started; }
  bool is_paused() const { return _started && _paused; }

private:
  uint32_t _started_ticks;
  uint32_t _paused_ticks;
  bool _started;
  bool _paused;
};

/* Init GMLib */
int GM_Init(const std::string & cfg_path, 
            const std::string & name = "GMLib App");

/* Shutdown GMLib */
void GM_Quit();

/* Get pointer to applied configuration */
const config* GM_GetConfig(); 

/* Get main SDL Window */
SDL_Window* GM_GetWindow();

/* Get rendered attached to main window */
SDL_Renderer* GM_GetRenderer();

/* Get display rect */
inline SDL_Rect GM_GetDisplayRect() 
{
  int w = 0, h = 0;
  SDL_GetWindowSize(GM_GetWindow(), &w, &h);
  SDL_Rect r = {0, 0, w, h};
  return r;
}

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
float GM_GetAvgFPS();
/* Display & calculate FPS: on/off */
void GM_SetFPS(bool state);

SDL_Surface* GM_CreateSurface(int width, int height);
SDL_Texture* GM_CreateTexture(int width, int height, SDL_TextureAccess access);

/* load resources from assets path with help of RES_GetFullPath */
SDL_Surface* GM_LoadSurface(const std::string& image);
SDL_Texture* GM_LoadTexture(const std::string& sheet);
TTF_Font*    GM_LoadFont(const std::string& font, int ptsize);

/* Game screen */

class screen {

  /* Screen private handles */
  SDL_Window * _wnd;
  SDL_GLContext _glctx;

public:

  class component {
  public:
    component(screen * s):_parent_screen(s) {}
    virtual ~component() {};
    virtual void render(SDL_Renderer* r, screen * src) = 0;
    virtual void update(screen * src) = 0;
    virtual void on_event(SDL_Event* ev, screen * src) = 0;
    screen * get_screen() { return _parent_screen; }

  private:
    screen * _parent_screen;
  };

  /* Active OpenGL context attached to this screen */
  virtual void activate() { SDL_GL_MakeCurrent(_wnd, _glctx); }

  /* Construct new custom screen instance */
  
  /* New screen with shared window and gl context */
  screen();
  /* New screen with custom window and gl content */
  screen(SDL_Window* wnd);

  virtual ~screen() { SDL_GL_DeleteContext(_glctx); }

  /* Current game screen pointer */
  static screen* current();
  static void set_current(screen* s);

  /* Returns instance of the global screen, 
     which can be used to components only
  */
  static screen * global();

  /* Screen Update */
  virtual void update() {
    lock_vector(_components);
    uvector<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->update(this);
    }
  };
  
  /* Screen Render */
  virtual void render(SDL_Renderer * r) {
    lock_vector(_components);
    uvector<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->render(r, this);
    }
  };

  /* Screen On Event Callback */
  virtual void on_event(SDL_Event* ev) {
    lock_vector(_components);
    uvector<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->on_event(ev,this);
    }
  };

  void add_component(screen::component* c) {
    _components.push_back(c);
  }

  template<class T>
  T* get_component()
  {
    lock_vector(_components);
    SDL_Log("get_component - lookup by type [%s]\n", typeid(T).name());
    T * res = NULL;
    uvector<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      screen::component* c = *it;
      res = dynamic_cast<T*> (c);
      if (res != NULL) break;
    }
    SDL_Log("get_component<%s>() - found instance [%p]\n", typeid(T).name(), res);
    return res;
  }

private:
  uvector<component*> _components;
};

#endif //GM_LIB_H