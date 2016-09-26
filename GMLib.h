/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This is the main GMLib header file to be
 * included by client applications.
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
#include <SDLEx.h>

/* GMLib version */
#define GM_LIB_MAJOR   2
#define GM_LIB_MINOR   1
#define GM_LIB_RELESE  "a"
#define GM_LIB_PATCH   4

/** Default log category */
#define SDLEx_LogCategory SDL_LOG_CATEGORY_APPLICATION

/** Error log wrapper */
#define SDLEx_LogError(fmt, ...) SDL_LogError(SDLEx_LogCategory, fmt, __VA_ARGS__ );


/*
 *
 * Core classes framework
 *
 */

/* create SDL resources */
SDL_Surface* GM_CreateSurface(int width, int height);
SDL_Texture* GM_CreateTexture(int width, int height, 
                              SDL_TextureAccess access,
                              uint32_t pixel_format);

/* load SDL resource by file path */
SDL_Surface* GM_LoadSurface(const std::string& file_path);
SDL_Texture* GM_LoadTexture(const std::string& file_path);
TTF_Font*    GM_LoadFont(const std::string& file_path, int ptsize);

/* Resource Interface */
struct iresource 
{
  iresource() {}
  virtual ~iresource() {}
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

/* SDL_Color wrapper */
struct color : SDL_Color {
  color() { r = 0; g = 0; b = 0; a = 0; }
  color(const SDL_Color & clr) { r = clr.r; g = clr.g; b = clr.b; a = clr.a; }
  color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa) { r = rr; g = gg; b = bb; a = aa; }

  void apply(SDL_Renderer* rnd) const;
  void apply() const;

  static color from_string(const std::string & sclr);

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
  rect rect::clip(const rect & other) const
  {
    rect res;
    res.x = max(x, other.x);
    res.w = min(x + w - other.x, other.x + other.w - x);
    res.y = max(y, other.y);
    res.h = min(y + h - other.y, other.y + other.h - y);

    if (w <= 0 || h <= 0) {
      res.w = 0; res.h = 0;
    }
    return res;
  }

  rect scale(const float & fw, const float & fh) const;

  /** Check this rect collides with another rect */
  bool rect::collide_rect(const rect & b) const
  {
    return x + w > b.x && b.x + b.w > x && \
          y + h > b.y && b.y + b.h > y;
  }

  /** Check this rect collides with a point */
  bool rect::collide_point(const point & pnt) const
  {
    return pnt.x >= x && \
            pnt.y >= y && \
            pnt.x < x + w && \
            pnt.y < y + h;
  }
};

/* SDL_Rect & SDL_Point utils */
bool operator== (rect& a, rect& b);
bool operator!= (rect& a, rect& b);

rect operator+ (const rect& a, const rect& b);
rect operator- (const rect& a, const rect& b);

rect operator+= (rect& a, const rect& b);
rect operator-= (rect& a, const rect& b);

rect operator+ (const rect& r, const point& p);
rect operator- (const rect& r, const point& p);

point operator+ (const point& a, const point& b);
point operator- (const point& a, const point& b);

bool operator== (point& a, point& b);
bool operator!= (point& a, point& b);

void operator+= (rect& r, point& p);
void operator-= (rect& r, point& p);

void operator+= (point& a, point& b);
void operator-= (point& a, point& b);

bool operator< (rect& a, rect& b);
bool operator<= (rect& a, rect& b);
bool operator> (rect& a, rect& b);
bool operator>= (rect& a, rect& b);

/* TTF_Font wrapper */
class ttf_font : public iresource {

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

  void load(const std::string & file_path, size_t pts)
  {
    if (_f != nullptr)
      TTF_CloseFont(_f);
    _pts = pts;
    _f = GM_LoadFont(file_path, _pts);
    _fname = file_path;
  }

  size_t pts() const { return _pts; }
  TTF_Font * fnt() const { return _f; }
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
    SDLEx_LogError("SDL Error: %s", _msg);
  }

  virtual const char * what() const
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
  
  template <class T>
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

/* Safe 2D array */
template<class T> 
class safe_array2
{
private:
  T** _arr;

public:
  safe_array2(size_t sz, T & init_value = T()) 
  {
      for (size_t i = 0; i < SIZE; i++)
      {
        _arr[i] = init_value;
      }
  }

  int &operator[](size_t idx)
  {
      if (idx > SIZE)
      {
        SDLEx_LogError("safe_array::operator[] index %d out of bounds", idx); 
        throw std::exception("index out of bounds");
      }
      return arr[i];
  }
};

/*
 *
 * GMLib Core
 *
 */

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
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->update(this);
    }
  };
  
  /* Screen Render */
  virtual void render(SDL_Renderer * r) {
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
    for(; it != _components.end(); ++it) {
      (*it)->render(r, this);
    }
  };

  /* Screen On Event Callback */
  virtual void on_event(SDL_Event* ev) {
    lock_container(_components);
    container<screen::component*>::iterator it = _components.begin();
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