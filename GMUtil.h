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

/** Create a new surface compatible with OpenGL & PNG */
SDL_Surface* GM_CreateSurface(int width, int height);

/** Create new texture setup for blended rendering target */
inline SDL_Texture* GM_CreateLayer(SDL_Renderer* r, int w, int h)
{
  SDL_Texture* l = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
  SDL_SetTextureBlendMode(l, SDL_BLENDMODE_BLEND);
  return l;
}

/** Clear given texture */
inline void GM_ClearLayer(SDL_Renderer* r, SDL_Texture* l)
{
    SDL_SetRenderTarget(r, l);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
}

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

/* SDL_Rect utils */
bool operator== (SDL_Rect& a, SDL_Rect& b);
bool operator!= (SDL_Rect& a, SDL_Rect& b);
void operator+= (SDL_Rect& r, SDL_Point& p);

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

 /* SDL_Color class wrapper with assignment constructor */
struct color: public SDL_Color
{
    color()
    { r = 0; g = 0; b = 0; a = 0; }
    color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a)
    { r = _r; g = _g; b = _b; a = _a; }

    void apply(SDL_Renderer* rndr) {
        SDL_SetRenderDrawColor(rndr, r, g, b, a);
    }
};

/*
  Generic dynamic array container with sorting
  */
template<class T> class array_list {
public:
  static const int grow_step = 10;

  array_list()
  {
    _len = 0;
    _array = NULL;
  }
  
  array_list(size_t len)
  {
    _len = len;
    _array = (T*)calloc(_len, sizeof(T));
  }

  virtual ~array_list()
  {
    if (_array) {
      free(_array);
    }
  }

  T& at(size_t idx)
  {
    if (idx >= _len) {
      SDLEx_LogError("array_list::at invalid index: %d, lenght: %d", idx, _len);
      throw std::exception("Invalid array index");
    }
    return _array[idx];
  }

  void set(size_t idx, const T& v)
  {
    if (idx >= _len) {
      SDLEx_LogError("array_list::set invalid index: %d, lenght: %d", idx, _len);
      throw std::exception("Invalid array index");
    }
    _array[idx] = v;
  }

  void append(const T& v)
  {
    if (_len == 0) {
      grow(grow_step);
    }
    size_t i = _len;
    do {
      --i;
      if (_array[i] != T() ) {
        if (i + 1 == _len) {
          grow(grow_step);
        }
        _array[i + 1] = v;
        return;
      }
      if (i == 0) {
        break;
      }
    }
    while(true);
    _array[i] = v;
  }

  size_t find(const T& v)
  {
    for(size_t i = 0; i < _len; ++i) {
      if (_array[i] == v) {
        return i;
      }
    }
    return _UI32_MAX;
  }

  void remove(const T& v) {
    size_t idx = find(v);
    if (idx < _len - 1) {
      for(; idx < _len - 1; ++idx) {
        _array[idx] = _array[idx + 1];
      }
    }
    --_len;
    _array = (T*)realloc(_array, _len * sizeof(T));
  }

  size_t length() { return _len; }

  void clear()
  {
    free(_array);
    _array = NULL;
    _len = 0;
  }

  static void merge(array_list<T>& left, array_list<T>& right, array_list<T>& res)
  {
    res.clear();
    while (left.length() && right.length()) {
      if (left.at(0) <= right.at(0)) {
        res.append(left.at(0));
        left.rshift(1);
      }
      else {
        res.append(right.at(0));
        right.rshift(1);
      }
    }

    while(left.length()) {
      res.append(left.at(0));
      left.rshift(1);
    }
    while(right.length()) {
      res.append(right.at(0));
      right.rshift(1);
    }
  }

  void sort()
  {
    if (length() <= 1)
      return;
    size_t mid = _len / 2;
    array_list<T> left(mid);
    array_list<T> right(mid);

    for(size_t i = 0; i < _len; ++i) {
      if (i <= mid) {
        left.append(at(i));
      }
      else {
        right.append(at(i));
      }
    }

    left.sort();
    right.sort();

    merge(left, right, *this);
  }

  void rshift(size_t num)
  {
    if (num >= _len) {
      clear();
      return;
    }

    for(size_t i = 0; i < _len - num; ++i) {
      _array[i] = _array[i + num];
    }

    _len -= num;
    _array = (T*)realloc(_array, _len * sizeof(T));
  }

private:

  void grow(size_t size)
  {
    _array = (T*)realloc(_array, (_len + size) * sizeof(T));
    memset(_array + (sizeof(T) * _len), 0, size * sizeof(T));
    _len += size;
  }

  T* _array;
  size_t _len;
};

/*
  Generic linked-list container
  */
template<class T> class linked_list {
public:
    template<class T> struct _node {
      _node* next;
      T val;

      _node(_node* n, const T& v):
        next(n), val(v)
      {}

      _node():
        next(NULL)
      {}

      void swap(_node& b) {
        T temp = val;
        this->val = b->val;
        b->val = temp;
      }

    };
    typedef _node<T> node;

    // append new node with given value
    virtual void append(const T& val) {
      node* itm = new node(NULL, val);
      if (_head) {
        itm->next = _head;
        _head = itm;
        _len += 1;
      }
      else {
        _head = itm;
        _len = 1;
      }
    }

    // remove node with given value, returns next node or NULL 
    virtual node* remove(const T& val) {
        node* i = _head;
        node* prev = NULL;
        node* nxt = NULL;
        while(i) {
          nxt = i->next;
          if (i->val == val) {
            //found node to delete, relink first
            if (prev) {
              prev->next = i->next;
            }
            else {
              _head = i->next;
            }
            delete i;
            break;
          }
          prev = i;
          i = nxt;
        }
        return nxt;
    }
    
    //find existing node with given value, returns NULL if not found
    node* find(const T& val) {
      node* ret = NULL;
      node* i = _head;
      while(i) {
        if (i->val == val) {
          ret = i;
          break;
        }
        i = i->next;
      }
      return ret;
    }

    node* at(size_t idx) {
      size_t c = 0;
      node* i = _head;
      node* ret = nullptr;
      while(i) {
        if (c == idx) {
          ret = i;
          break;
        }
        i = i->next;
        c++;
      }
      return ret;
    }

    // list head 
    node* first() { return _head; }

    // list elements count
    size_t size() { return _len; }
    
    //delete all nodes in list
    void clear() {
        node* i = _head;
        while(i) {
            node* nxt = i->next;
            delete i;
            i = nxt;
        }
        _head = NULL;
        _len = 0;
    }

    //create new empty list
    linked_list() {
        _len = 0;
        _head = NULL;
    }

    virtual ~linked_list() {
        clear();
    }

private:
    size_t _len;
    node* _head;
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

private:
    SDL_mutex* mtx;
};

/*
    Mutex based linked_list
*/
template <class T> class locked_linked_list: public linked_list<T>, public mutex {
public:
    virtual void append(const T& val) {
      lock();
      linked_list<T>::append(val);
      unlock();
    }

    virtual node* remove(const T& val) {
      lock();
      node* nxt = linked_list<T>::remove(val);
      unlock();
      return nxt;
    }

    locked_linked_list() 
    {}

    virtual ~locked_linked_list() 
    {}
};

/*
    Mutex based array_list
*/
template <class T> class locked_array_list: public array_list<T>, public mutex {
public:
    virtual void append(const T& val) {
      lock();
      array_list<T>::append(val);
      unlock();
    }

    virtual void remove(const T& val) {
      lock();
      array_list<T>::remove(val);
      unlock();
    }

    locked_array_list()
    {}

    virtual ~locked_array_list() 
    {}
};

/*
    Mutex based std::vector
*/
template<class T> class locked_vector: public std::vector<T>, public mutex {
public:
    locked_vector()
    {}

    virtual ~locked_vector() 
    {}

    void lck_push_back(const T & s) {
        lock();   
        push_back(s);
        unlock();
    }

    void lck_remove(const T & s) {
        lock();
        locked_list<T>::iterator it = std::find(begin(), end(), s);
        if (it != end()) {
            erase(it);
        }
        unlock();  
    }
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

inline int double_to_sint32(double d) {
	return d >= 0.0 ? (int)(d+0.5) : (int)(d-0.5);
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