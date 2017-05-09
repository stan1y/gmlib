/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This is the main GMLib header file to be
 * included by client applications.
 *
 * GMLib supports a number of compilation flags to control
 * debug logging & rendering for components of the library. 
 * Here is the actual list of the flags you can use:
 * - GM_DEBUG              - generic core library components logging
 * - GM_DEBUG_UI           - UI controls, manager, theme, etc logging
 * - GM_DEBUG_MULTITEXTURE - multi_texture class logging & rendering
 */

#ifndef GM_LIB_H
#define GM_LIB_H

#ifdef _WIN32
#include <windows.h>
#endif 

#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <cctype>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <exception>
#include <mutex>

#include <limits.h>

#include <SDL.h>
#include <SDL_config.h>
#include <SDL_render.h> 
#include <SDL_video.h> 
#include <SDL_opengl.h> 
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_log.h>
#include <sdl_ex.h>

#include <json.hpp>
using namespace nlohmann;

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

// list of paths used in some helpers
typedef std::vector<path> paths_list;

/* GMLib version */
#define GM_LIB_MAJOR   2
#define GM_LIB_MINOR   1
#define GM_LIB_RELESE  "a"
#define GM_LIB_PATCH   4

/* __METHOD_NAME__ wrapper for MSVS and GCC */
#ifndef __METHOD_NAME__
  #ifdef __PRETTY_FUNCTION__
    #define __METHOD_NAME__ __PRETTY_FUNCTION__
  #else
    #define __METHOD_NAME__ __FUNCTION__
  #endif
#endif

/* Define if we want to cap fps and for how much */
#ifndef GM_FPS_CAP
#define GM_FPS_CAP 60
#endif


/*
 *
 * Core classes framework
 *
 */

// list of strings for various things everywhere
typedef std::vector<std::string> strings_list;

/* create SDL resources */
SDL_Surface* GM_CreateSurface(int width, int height);
SDL_Texture* GM_CreateTexture(int width, int height, 
                              SDL_TextureAccess access,
                              uint32_t pixel_format);

/* load SDL resource by file path */
SDL_Surface* GM_LoadSurface(const std::string& file_path);
SDL_Texture* GM_LoadTexture(const std::string& file_path);
TTF_Font*    GM_LoadFont(const std::string& file_path, int ptsize);

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

/* SDL_Color wrapper */
struct color : SDL_Color {
  color() { r = 0; g = 0; b = 0; a = 0; }
  color(const SDL_Color & clr) { r = clr.r; g = clr.g; b = clr.b; a = clr.a; }
  color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa) { r = rr; g = gg; b = bb; a = aa; }

  // function chaining
  color & set_red(const uint8_t & v) { r = v; return *this; }
  color & set_green(const uint8_t & v) { g = v; return *this; }
  color & set_blue(const uint8_t & v) { b = v; return *this; }
  color & set_alpha(const uint8_t & v) { a = v; return *this; }

  void apply(SDL_Renderer* rnd) const;
  void apply() const;

  static color from_string(const std::string & sclr);
  static color from_json(const json & d);

  static color red() { return color(255, 0, 0, 255); }
  static color dark_red() { return color(196, 0, 0, 255); }
  static color green() { return color(0, 255, 0, 255); }
  static color dark_green() { return color(0, 196, 0, 255); }
  static color blue() { return color(0, 0, 255, 255); }
  static color dark_blue() { return color(0, 0, 196, 255); }
  static color black() { return color(0, 0, 0, 255); }
  static color white() { return color(255, 255, 255, 255); }
  static color gray() { return color(127, 127, 127, 255); }
  static color light_gray() { return color(205, 205, 205, 255); }
  static color dark_gray() { return color(64, 64, 64, 255); }
  static color yellow() { return color(255, 255, 0, 255); }
  static color magenta() { return color(255, 0, 255, 255); }
  static color cyan() { return color(0, 255, 255, 255); }
};

bool operator== (const color& a, const color& b);
bool operator!= (const color& a, const color& b);

/* SDL_Point wrapper */
struct point : SDL_Point {
  point() { x = 0; y = 0; }
  point(int _x, int _y) { x = _x; y = _y; }
  point(const point& copy) { x = copy.x; y = copy.y; }
  point(const SDL_Point& copy) { x = copy.x; y = copy.y; }

  std::string tostr() const
  { 
    std::stringstream ss;
    ss << "point<" << x << ", " << y << ">";
    return ss.str();
  }

  /* test collision of point and circle */
  bool collide_circle(const point & center, const int radius);

};

/* SDL_Rect wrapper */
struct rect : SDL_Rect {
  rect() { x = 0; y = 0; w = 0; h = 0; }
  rect(int _x, int _y, int _w, int _h) { x = _x; y = _y; w = _w; h = _h; }
  rect(const rect& copy) { x = copy.x; y = copy.y; w = copy.w; h = copy.h; }
  rect(const SDL_Rect& copy) { x = copy.x; y = copy.y; w = copy.w; h = copy.h; }

  point topleft() const { return point(x, y); }
  point bottomright() const { return point(x + w, y + h); }

  std::string tostr() const
  { 
    std::stringstream ss;
    ss << "rect<" << x << ", " << y \
       << ", " << w << ", " << h << ">";
    return ss.str();
  }

  /** Produce a new rect as a clip of this and another rect */
  rect clip(const rect & other) const
  {
    rect out;
    if (SDL_IntersectRect(this, &other, &out) != SDL_TRUE) {
      return other;
    }
    return out;
  }

  rect scale(const float & fw, const float & fh) const;

  rect center(const uint32_t for_w, const uint32_t for_h) const;

  /** Check this rect collides with another rect */
  bool collide_rect(const rect & b) const
  {
    SDL_Rect dummy;
    return (SDL_IntersectRect(this, &b, &dummy) == SDL_TRUE);
  }

  /** Check this rect collides with a point */
  bool collide_point(const point & pnt) const
  {
    return (SDL_PointInRect(&pnt, this) == SDL_TRUE);
  }
};

/* SDL_Rect & SDL_Point utils */
bool operator== (const rect& a, const rect& b);
bool operator!= (const rect& a, const rect& b);

rect operator+ (const rect& a, const rect& b);
rect operator- (const rect& a, const rect& b);

rect operator+= (rect& a, const rect& b);
rect operator-= (rect& a, const rect& b);

rect operator+ (const rect& r, const point& p);
rect operator- (const rect& r, const point& p);

point operator+ (const point& a, const point& b);
point operator- (const point& a, const point& b);

bool operator== (const point& a, const point& b);
bool operator!= (const point& a, const point& b);

void operator+= (rect& r, const point& p);
void operator-= (rect& r, const point& p);

void operator+= (point& a, const point& b);
void operator-= (point& a, const point& b);

bool operator< (rect& a, rect& b);
bool operator<= (rect& a, rect& b);
bool operator> (rect& a, rect& b);
bool operator>= (rect& a, rect& b);

/* TTF_Font wrapper */
class ttf_font {

protected:
  std::string _fname;
  TTF_Font * _f;
  size_t _pts;

public:
  ttf_font():_f(nullptr), _pts(0) {}

  ttf_font(const std::string & file_path, size_t pts):
    _f(nullptr),
    _pts(pts)
  {
    load(file_path, pts);
  }

  void load(const std::string & file_path, size_t pts);

  size_t pts() const { return _pts; }
  TTF_Font * get_font() const { return _f; }
  bool is_loaded() const { return (_f != nullptr); }
  const std::string & filename() const { return _fname; }
  virtual ~ttf_font()
  {
    if (_f != nullptr)
      TTF_CloseFont(_f);
  }

  std::string tostr()
  {
    std::stringstream ss;
    ss << "font< " << _fname \
       << ", pts=" << _pts << ">";
    return ss.str();
  }

  /* text printing */
  SDL_Surface * print_solid(const std::string & text,
                            const color & clr) const;
  SDL_Surface * print_blended(const std::string & text,
                            const color & clr) const;
  rect get_text_rect(const std::string& text) const;
  
};

/* SDL_Error exception wrapper */
class sdl_exception : public std::exception {
private:
  const char* _msg;

public:
  sdl_exception():
    std::exception(),
    _msg(SDL_GetError())
  {
    SDL_Log("SDL Error: %s", _msg);
  }

  virtual const char * what() const _NOEXCEPT
  {
    return _msg;
  }
};

/*
    SDL_Mutex wrapper
*/

class sdl_mutex {
public:
    sdl_mutex() {
        mtx = SDL_CreateMutex();
    }
    
    virtual ~sdl_mutex() {
        SDL_DestroyMutex(mtx);
    }

    void lock() { SDL_LockMutex(mtx); }
    void unlock() { SDL_UnlockMutex(mtx); }
    int try_lock() { return SDL_TryLockMutex(mtx); }

private:
    SDL_mutex* mtx;
};

typedef std::lock_guard<sdl_mutex> mutex_lock;

/*
    Mutex based std::vector
*/
template<class T> class container {
public:
  typedef typename std::vector<T>::const_iterator const_iterator;
  typedef typename std::vector<T>::iterator iterator;
  typedef typename std::vector<T>::reverse_iterator reverse_iterator;

  container()
  {}

  virtual ~container() 
  {}

  void push_back(const T & s) {
    mutex_lock guard(_m);
    _v.push_back(s);
  }

  void remove(const T & s) {
    mutex_lock guard(_m);
    iterator it = find(s);
    if (it != _v.end()) {
      _v.erase(it);
    }
  }

  iterator erase(iterator it) {
    mutex_lock guard(_m);
    return _v.erase(it);
  }

  iterator insert(iterator where_it, T & val) {
    mutex_lock guard(_m);
    return _v.insert(where_it, val);
  }

  iterator begin() { return _v.begin(); }
  iterator end() { return _v.end(); }

  const_iterator begin() const { return _v.begin(); }
  const_iterator end() const { return _v.end(); }

  reverse_iterator rbegin() { return _v.rbegin(); }
  reverse_iterator rend() { return _v.rend(); }

  T& operator[](size_t pos) { return _v[pos]; }
  iterator find(const T& val) { return std::find(_v.begin(), _v.end(), val); }

  size_t size() const { return _v.size(); }
  void clear() { _v.clear(); }
  std::vector<T> & get() { return _v; }
  
  template <T>
  void copy_to(std::vector<T> & output, size_t from = 0, size_t to = -1) {
    std::copy(_v.begin() + from, _v.begin() + (to >= 0 ? to : size() - 1), output);
  }

  sdl_mutex & mutex() { return _m; }

private:
  std::vector<T> _v;
  sdl_mutex _m;
};

/* Define lock helper */
#define lock_container(uv) mutex_lock uv_lock(uv.mutex())

/*
 *
 * GMLib Core
 *
 */

/* Global Config */
class config {
public:
  // load into engine from file path
  static void load(const std::string & cfg_file);

  // get current config instance, if it was loaded before
  static const config & current();

  // get SDL driver index based on settings
  int get_driver_index() const;

  // get SDL window flags based on input
  int get_window_flags() const;

  // get config data
  const json & get_data() const { return _data; }

private:
  config(const std::string cfgpath);

  json _data;
};

/* Init GMLib */
int GM_Init(const std::string & cfg_path, 
            const std::string & name = "GMLib App");

/* Shutdown GMLib */
void GM_Quit();

/* Request graceful shutdown */
void GM_PostQuit();

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

/* Main game loop. Returns only on exit. */
void GM_Loop();

/* Get miliseconds elapsed since start of the current frame */
uint32_t GM_GetFrameTicks();

/* Get average FPS if enabled or 0.0 */
float GM_CurrentFPS();

/* 
 * 
 * Game screen managment
 *
 */

class screen {

  /* Screen private handles */
  SDL_Window * _wnd;

public:

  class component {
  public:
    component(screen * s):_parent_screen(s) {}
    virtual ~component() {};
    virtual void render(SDL_Renderer* r) = 0;
    virtual void on_update(screen *) = 0;
    virtual void on_event(SDL_Event* ev) = 0;
    screen * get_screen() { return _parent_screen; }

  private:
    screen * _parent_screen;
  };

  /* New screen with shared window */
  screen();
  /* New screen with custom window */
  screen(SDL_Window* wnd);

  virtual ~screen();

  /* Current game screen pointer */
  static const screen* current();
  static void set_current(screen* s, bool destroy_on_change = true);

  /* Screen Update */
  virtual void update()
  {
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->on_update(this);
    }
  }
  
  /* Screen Render */
  virtual void render(SDL_Renderer * r) {
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->render(r);
    }
  };

  /* Screen On Event Callback */
  virtual void on_event(SDL_Event* ev) {
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->on_event(ev);
    }
  };

  void add_component(screen::component* c) {
    _components.push_back(c);
  }

  template<class T>
  T* get_component()
  {
    lock_container(_components);
    SDL_Log("get_component - lookup by type [%s]\n", typeid(T).name());
    T * res = NULL;
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      screen::component* c = *it;
      res = dynamic_cast<T*> (c);
      if (res != NULL) break;
    }
    SDL_Log("get_component<%s>() - found instance [%p]\n", typeid(T).name(), res);
    return res;
  }

private:
  container<component*> _components;
};

#endif //GM_LIB_H
