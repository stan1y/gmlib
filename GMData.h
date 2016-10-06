/* 
 * GMLib - The GMLib library.
 * Copyright Stanislav Yudin, 2014-2016
 *
 * This header defines opaque data type used across the GMLib
 * to represent various structured data, from config to ui and
 * animations. The serialization is based on JSON via jansson
 * library providing raw json_t type.
 */

#ifndef _GM_DATA_H_
#define _GM_DATA_H_

#include <jansson.h>

#include "GMLib.h"
#include "GMUtil.h"

/* Load json object from resources */
json_t*      GM_LoadJSON(const std::string& file);

/* Utility conterters */
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

/** pebble::data
    A JSON json_t opaque data storage for 
    structured data in files and memory.
**/
class data: public iresource {
public:

  bool valid() { return (_p != nullptr); }

  data():_p(nullptr), _f(0)
  {}

  data(json_t* p):_p(nullptr), _f(0)
  {
    set_owner_of(p);
  }

  /* parse json string */
  data(const std::string & json, uint32_t parser_flags);

  /* product json string */
  std::string tostr() const;
  
  /* load from a resource */
  data(const std::string & json_res);

  /* returns string representation of the
  std::string tostr(int ident = 2) const
  {
    char * s = json_dumps(_p, JSON_INDENT(ident) );
    return std::string(s); // return a copy
  }

  /* Load json data from a file given by path */
  virtual void load(const std::string & json_file);

  /* Unpack contained JSON with provided format */
  virtual void unpack(const char *fmt, ...) const;

  /* Set this instance of data a owner of ref to the 
     specified json_t by incref-ing it */
  void set_owner_of(json_t* p)
  {
    if (p == NULL) return;
    if (_p != NULL) {
      json_decref(_p);
      _p = NULL;
    }
    _p = p;
    json_incref(_p);
  }

  ~data()
  {
    if (_p != NULL) {
      json_decref(_p);
      _p = NULL;
    }
  }

  bool is_object() const
  {
    return json_is_object(_p);
  }

  bool has_key(const std::string & key) const
  {
    if (!is_object()) throw std::exception("data is not a object");
    json_t* j = json_object_get(_p, key.c_str());
    return (j != NULL);
  }

  bool has_subkey(const std::string & key) const
  {
    if (!is_object()) throw std::exception("data is not a object");
    std::vector<std::string> subkeys = split_string(key, '.');
    auto ikey = subkeys.begin();
    json_t* current = _p;
    for(; ikey != subkeys.end(); ++ikey) {
      current = json_object_get(current, ikey->c_str());
      if (current == NULL) {
        return false;
      }
    }
    return (current != NULL && current != _p);
  }

  bool is_array(const size_t of_size = 0) const
  {
    return json_is_array(_p) && ( of_size == 0 ? true : json_array_size(_p) == of_size );
  }

  bool is_string() const
  {
    return json_is_string(_p);
  }

  size_t length() const
  {
    if (is_string()) {
      const char* s = json_string_value(_p);
      return SDL_strlen(s);
    }
    if (is_array())
    {
      return json_array_size(_p);
    }
    throw std::exception("data is not indexable");
  }

  bool is_number() const
  {
    return json_is_number(_p);
  }

  /* Template methods to convert into cutom type */
  template<typename T> T as() const;
  
  template<> int32_t as() const
  {
    if (!json_is_integer(_p)) throw std::exception("data object is not an integer");
    json_int_t i = json_integer_value(_p);
    return jint_to_sint32(i);
  }

  template<> uint32_t as() const
  {
    if (!json_is_integer(_p)) throw std::exception("data object is not an integer");
    json_int_t i = json_integer_value(_p);
    return jint_to_uint32(i);
  }

  template<> uint8_t as() const
  {
    if (!json_is_integer(_p)) throw std::exception("object is not an integer");
    json_int_t i = json_integer_value(_p);
    return jint_to_uint8(i);
  }

  template<> point as() const
  {
    if (!json_is_array(_p)) {
      throw std::exception("object is not a point array");
    }
    if (length() != 2) {
      throw std::exception("invalid object array length for a point array");
    }
    point p;
    unpack("[ii]", &p.x, &p.y);
    return p;  
  }

  template<> rect as() const
  {
    if (!json_is_array(_p)){
      SDLEx_LogError("data::as - json is not a rect array");
      throw std::exception("json is not a rect array");
    }
    if (length() != 4) {
      SDLEx_LogError("data::as - invalid object array length for a rect array");
      throw std::exception("invalid object array length for a rect array");
    }
    rect r;
    unpack("[iiii]", &r.x, &r.y, &r.w, &r.h);
    return r;
  }

  template<> color as() const
  {
    if (!json_is_array(_p)) throw std::exception("object is not a color array");
    if (length() != 4) throw std::exception("invalid object array length for a color array");
    color c;
    json_int_t r = 0, g = 0, b = 0, a = 0;
    unpack("[iiii]", &r, &g, &b, &a);
    c.r = jint_to_uint8(r);
    c.g = jint_to_uint8(g);
    c.b = jint_to_uint8(b);
    c.a = jint_to_uint8(a);
    return c;
  }

  template<> const char * as() const
  {
    if (!json_is_string(_p)) throw std::exception("object is not a string");
    const char * s = json_string_value(_p);
    return s;
  }

  template<> std::string as() const
  {
    return std::string(as<const char*>());
  }

  template<> std::pair<int,int> as() const
  {
    if (!json_is_array(_p)) {
      SDLEx_LogError("data::as() - instance is not a pair");
      throw std::exception("instance is not a pair array");
    }
    if (length() != 2) {
      throw std::exception("invalid array length for a pair");
      SDLEx_LogError("data::as() - invalid array length for a pair");
    }
    int f = 0, s = 0;
    unpack("[ii]", &f, &s);
    return std::make_pair(f, s);
  }

  template<> bool as() const
  {
    return (json_typeof(_p) == JSON_TRUE);
  }

  bool is_true() const
  {
    if (!json_is_boolean(_p)) {
      SDLEx_LogError("data::is_true - instance is not a boolean");
      throw std::exception("instance is not a boolean");
    }
    return json_is_true(_p);
  }

  void set(const char* key, json_t * val)
  {
    if (!is_object()) {
      SDLEx_LogError("data::set - instance is not an object");
      throw std::exception("instance is not an object");
    }
    json_object_set(_p, key, val);
  }

  template<typename T>
  void set(const char* key, const T & val);

  template<>
  void set(const char* key, const std::string & val)
  {
    set(key, json_string(val.c_str()));
  }

  template<>
  void set(const char* key, const int & val)
  {
    set(key, json_integer(val));
  }

  template<typename T>
  T get(const char* key, T def_value = T()) const
  {
    if (!is_object()) {
      SDLEx_LogError("data::get - instance is not an object");
      throw std::exception("instance is not a object");
    }
    json_t * p = json_object_get(_p, key);
    if (p == NULL) 
      return def_value;
    return data(p).as<T>();
  }

  data operator[](const std::string& key) const
  {
    if (!is_object()) {
      SDLEx_LogError("data::operator[std::string] - instance is not an object");
      throw std::exception("instance is not a object");
    }

    json_t * p = NULL;
    if (key.find(".") == UINT32_MAX) {
      p = json_object_get(_p, key.c_str());
    }
    else {
      std::vector < std::string> tokens = split_string(key, '.');
      for (size_t i = 0; i < tokens.size(); ++i) {
        p = json_object_get(p == NULL ? _p : p, tokens[i].c_str());
      }
    }
    if (p == NULL) {
      SDLEx_LogError("data::operator[std::string] - object has no key ");
      throw std::exception("data object has no key");
    }
    return data(p);
  }

  data operator[](size_t idx) const
  {
    if (!is_array()) {
      SDLEx_LogError("data::operator[size_t] - instance is not an array");
      throw std::exception("data is not an array");
    }
    if (idx >= json_array_size(_p)) {
      SDLEx_LogError("data::operator[size_t] - index is out of range");
      throw std::exception("index out of data array range");
    }
    return data(json_array_get(_p, idx));
  }
  
  json_t * ptr() const { return _p; }

private:
  json_t* _p;
  uint32_t _f;
};

/* Get config as data object */
const data & GM_GetConfigData();

#endif //_GM_DATA_H_