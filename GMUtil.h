#ifndef GM_UTIL_H
#define GM_UTIL_H

#include <boost/lexical_cast.hpp>

#include "GMLib.h"

/* Define min/max funcs as expected */
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/* Convert booleans to string */
#define YES_NO(val) (val == true ? "yes" : "no")

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
void render_vertical_gradient(SDL_Renderer* renderer,  const color & from, const color& to, const point & start, const point & end, uint8_t alpha);


/* Executable path and folder functions */
//std::string GM_GetExecutablePath();
//std::string GM_GetCurrentPath();
//void GM_EnumPath(const std::string& folder, std::vector<std::string>& files, bool recursive);
//void GM_EnumPathEx(const std::string& folder, const std::string& ext, std::vector<std::string>& files, bool recursive);

static std::vector<std::string> & split_string(const std::string &s, const char delim, std::vector<std::string> &elems) 
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


static std::vector<std::string> split_string(const std::string &s, const char delim) 
{
  std::vector<std::string> elems;
  split_string(s, delim, elems);
  return elems;
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

inline double rand_double(double min, double max)
{
  double f = (double)rand() / RAND_MAX;
  return min + f * (max - min);
}

/* Get random ASCII char */
inline char rand_char()
{
  static int ascii_start = 97;
  static int ascii_end = 122;
  int r = rand_int(ascii_start, ascii_end);
  return int32_to_int8(r);
}

inline std::string rand_string(const size_t len)
{
  char * buf = (char*)malloc(sizeof(char) * (len + 1));
  memset(buf, len + 1, 0);
  for(size_t i = 0; i < len; ++i) {
    buf[i] = rand_char();
  }
  // return a copy
  std::string s(buf);
  free(buf);

  return s;
}

inline std::string sint32_tostr(const int32_t & i) { return boost::lexical_cast<std::string>(i); }
inline std::string uint32_tostr(const uint32_t & i) { return boost::lexical_cast<std::string>(i); }
inline std::string uint8_tostr(const uint8_t & i) { return boost::lexical_cast<std::string>(i); }
inline std::string double_tostr(const double & d) { return boost::lexical_cast<std::string>(d); }

#endif //GM_UTIL_H