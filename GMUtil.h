#ifndef GM_UTIL_H
#define GM_UTIL_H

#include <SDL.h>

#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif 

#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <exception>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <boost/foreach.hpp>

/* SDL_Color wrapper */
struct color : SDL_Color {
  color() { r = 0; g = 0; b = 0; a = 0; }
  color(const SDL_Color & clr) { r = clr.r; g = clr.g; b = clr.b; a = clr.a; }
  color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa) { r = rr; g = gg; b = bb; a = aa; }

  void apply(SDL_Renderer* rnd) const;
  void apply() const;
};

/* SDL_Point wrapper */
struct point : SDL_Point {
  point() { x = 0; y = 0; }
  point(int _x, int _y) { x = _x; y = _y; }
  point(const point& copy) { x = copy.x; y = copy.y; }

  std::string tostr() const
  { 
    std::stringstream ss;
    ss << "point<" << x << ", " << y << ">";
    return ss.str();
  }
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
    res.w = min(x + w, other.x + other.w) - x;
    res.y = max(y, other.y);
    res.h = min(y + h, other.y + other.h) - y;

    if (w <= 0 || h <= 0) {
      res.w = 0; res.h = 0;
    }
    return res;
  }

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

/** Check point in rect
    Returns SDL_TRUE or SDL_FALSE
*/
inline SDL_bool IsPointInRect(SDL_Rect* rect, int x, int y) {
    return x >= rect->x && x <= rect->x + rect->w && y >= rect->y && y <= rect->y + rect->h ? SDL_TRUE : SDL_FALSE;
}

/** Default log category */
#define SDLEx_LogCategory SDL_LOG_CATEGORY_APPLICATION

/** Error log wrapper */
#define SDLEx_LogError(fmt, ...) SDL_LogError(SDLEx_LogCategory, fmt, __VA_ARGS__ );

/*
 * Implementation of linear gradient
 * color - base color to start gradient
 * start - top left corner to start drawing
 * end - bottom right corner to stop drawing
 * xstep, ystep - gradient direction (i.e vertical growing gradient is xstep = 0, ystep = 1)
 * alpha - drawing alpha value
 */
void RenderVerticalGradient(SDL_Renderer* renderer,  const color & from, const color& to, const point & start, const point & end, uint8_t alpha);

/* SDL_Rect & SDL_Point utils */
bool operator== (rect& a, rect& b);
bool operator!= (rect& a, rect& b);
rect operator+ (const rect& a, const rect& b);
rect operator- (const rect& a, const rect& b);

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

/* Executable path and folder functions */
std::string GM_GetExecutablePath();
std::string GM_GetCurrentPath();
void GM_EnumPath(const std::string& folder, std::vector<std::string>& files, bool recursive);
void GM_EnumPathEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files, bool recursive);

/* SDL_Error exception wrapper */
class sdl_exception : public std::exception {
private:
  const char* _msg;

public:
  sdl_exception():
    std::exception(),
    _msg(SDL_GetError())
  {
    SDLEx_LogError("SDL Error: %s", SDL_GetError());
  }

  virtual const char * what() const
  {
    return _msg;
  }
};

/*
    SDL_Mutex wrapper
*/

class mutex {
public:
    mutex() {
        mtx = SDL_CreateMutex();
    }
    
    virtual ~mutex() {
        SDL_DestroyMutex(mtx);
    }

    void lock() { SDL_LockMutex(mtx); }
    void unlock() { SDL_UnlockMutex(mtx); }
    int try_lock() { return SDL_TryLockMutex(mtx); }

private:
    SDL_mutex* mtx;
};

/*
    Mutex based std::vector
*/
template<class T> class locked_vector: public mutex {
public:
  typedef typename std::vector<T>::iterator iterator;

  locked_vector()
  {}

  virtual ~locked_vector() 
  {}

  void push_back(const T & s) {
    lock();   
    _v.push_back(s);
    unlock();
  }

  void remove(const T & s) {
    lock();
    std::vector<T>::iterator it = std::find(_v.begin(), _v.end(), s);
    if (it != _v.end()) {
        _v.erase(it);
    }
    unlock();
  }

  typename std::vector<T>::iterator erase(typename std::vector<T>::iterator it) {
    return _v.erase(it);
  }

  typename std::vector<T>::iterator begin() { return _v.begin(); }
  typename std::vector<T>::iterator end() { return _v.end(); }
  T& operator[](size_t pos) { return _v[pos]; }
  typename std::vector<T>::iterator find(const T& val) { return std::find(_v.begin(), _v.end(), val); }

  size_t size() { return _v.size(); }
  void clear() { _v.clear(); }
  std::vector<T>& get() { return _v; }

private:
  std::vector<T> _v;
};

/** Utility convertors */

inline uint8_t uint32_to_uint8(uint32_t i) {
    if (i > _UI8_MAX) {
        return _UI8_MAX;
    }
    return (uint8_t)i;
}

inline int32_t uint32_to_int32(uint32_t i) {
    if (i > _I32_MAX) {
        return _I32_MAX;
    }
    return (int32_t)i;
}

inline uint8_t int32_to_uint8(int32_t i) {
    if (i > _UI8_MAX) {
        return _UI8_MAX;
    }
    return (uint8_t)i;
}

inline int32_t int32_to_uint32(int32_t i) {
    if (i > _UI32_MAX) {
        return _UI32_MAX;
    }
    return (int32_t)i;
}

inline int8_t int32_to_int8(int32_t i) {
    if (i > _I8_MAX) {
        return _I8_MAX;
    }
    return (int8_t)i;
}

inline uint8_t int32_to_uint8(uint32_t i) {
  if (i > _I8_MAX) {
        return _I8_MAX;
    }
    return (int8_t)i;
}

inline int32_t str_to_sint32(const char* str) {
    long l = std::strtol(str, nullptr, 10);
    if (l > _I32_MAX) {
        return _I32_MAX;
    }
    return (int32_t)l;
}

inline int32_t double_to_sint32(double d) {
	return d >= 0.0 ? (int32_t)(d+0.5) : (int32_t)(d-0.5);
}

inline int32_t float_to_sint32(float f) {
	return f >= 0.0 ? (int32_t)(f+0.5) : (int32_t)(f-0.5);
}

/* create a new raw string copy for a given */
inline char* copy_string(const char* str, size_t l = 0)
{
  if (l == 0) 
    l = SDL_strlen(str);
  size_t s = sizeof(char) * (l + 1);
  char* c = (char*)SDL_malloc(s);
  memset(c, 0, s);
  SDL_strlcpy(c, str, s);
  return c;
}

/* Get random interger in range */
inline int rand_int(int min, int max) 
{
    int rc = min + (rand() % (int)(max - min + 1));
    return rc;
}

/* Get random ASCII char */
inline char rand_char()
{
    static int ascii_start = 97;
    static int ascii_end = 122;
    int r = rand_int(ascii_start, ascii_end);
    return int32_to_int8(r);
}

#endif //GM_UTIL_H