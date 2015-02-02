#ifndef GM_UTIL_H
#define GM_UTIL_H

#include "GMLib.h"
#include <exception>

/** Check point in rect
    Returns SDL_TRUE or SDL_FALSE
*/
inline SDL_bool SDLEx_IsPointInRect(SDL_Rect* rect, int x, int y) {
    return x >= rect->x && x <= rect->x + rect->w && y >= rect->y && y <= rect->y + rect->h ? SDL_TRUE : SDL_FALSE;
}

/** Default log category */
#define SDLEx_LogCategory SDL_LOG_CATEGORY_APPLICATION

/** Error log wrapper */
#define SDLEx_LogError(fmt, ...) SDL_LogError(SDLEx_LogCategory, fmt, __VA_ARGS__ );

/*
 * This is an implementation of the Midpoint Circle Algorithm 
 * found on Wikipedia at the following link:
 *
 *   http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 *
 * The algorithm elegantly draws a circle quickly, using a
 * SDL_RenderDrawPoint function for rendering.
 */
void SDLEx_RenderDrawCircle(SDL_Renderer *renderer, int n_cx, int n_cy, int radius);

/*
 * Implementation of linear gradient
 * color - base color to start gradient
 * start - top left corner to start drawing
 * end - bottom right corner to stop drawing
 * xstep, ystep - gradient direction (i.e vertical growing gradient is xstep = 0, ystep = 1)
 * alpha - drawing alpha value
 */
void SDLEx_RenderVerticalGradient(SDL_Renderer* renderer, SDL_Color& from, SDL_Color& to, SDL_Point& start, SDL_Point& end, uint8_t alpha);

/* SDL_Rect & SDL_Point utils */
bool operator== (SDL_Rect& a, SDL_Rect& b);
bool operator!= (SDL_Rect& a, SDL_Rect& b);
bool operator== (SDL_Point& a, SDL_Point& b);
bool operator!= (SDL_Point& a, SDL_Point& b);
void operator+= (SDL_Rect& r, SDL_Point& p);

void operator+= (SDL_Point& a, SDL_Point& b);
void operator-= (SDL_Point& a, SDL_Point& b);

bool operator< (SDL_Rect& a, SDL_Rect& b);
bool operator<= (SDL_Rect& a, SDL_Rect& b);
bool operator> (SDL_Rect& a, SDL_Rect& b);
bool operator>= (SDL_Rect& a, SDL_Rect& b);

/* Executable path and folder functions */
std::string GM_GetExecutablePath();
std::string GM_GetCurrentPath();
std::string GM_GetPathOfFile(const std::string& filepath);
std::string GM_JoinPaths(const std::string& root, const std::string& relative);
void GM_EnumPath(const std::string& folder, std::vector<std::string>& files, int d_type);
void GM_EnumPathFiles(const std::string& folder, std::vector<std::string>& files);
void GM_EnumPathFilesEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files);
void GM_EnumPathFolders(const std::string& folder, std::vector<std::string>& folders);

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

private:
    SDL_mutex* mtx;
};

/*
    Mutex based std::vector
*/
template<class T> class locked_vector: public mutex {
public:
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

inline uint32_t jint_to_uint32(json_int_t i) {
    if (i > _UI32_MAX) {
        return _UI32_MAX;
    }
    return (uint32_t)i;
}

inline int32_t jint_to_sint32(json_int_t i) {
    if (i > _I32_MAX) {
        return _I32_MAX;
    }
    return (int32_t)i;
}

inline uint8_t jint_to_uint8(json_int_t i) {
    if (i > _UI8_MAX) {
        return _UI8_MAX;
    }
    return (uint8_t)i;
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

inline int32_t str_to_sint32(const char* str) {
    long l = std::strtol(str, nullptr, 10);
    return jint_to_uint32(l);
}

inline char* sint32_to_str(int32_t i) {
    static char buf[12];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%d", i);
    return buf;
}

inline int32_t double_to_sint32(double d) {
	return d >= 0.0 ? (int32_t)(d+0.5) : (int32_t)(d-0.5);
}

inline int32_t float_to_sint32(float f) {
	return f >= 0.0 ? (int32_t)(f+0.5) : (int32_t)(f-0.5);
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