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

/* Utility conterters */
inline uint32_t jint_to_uint32(json_int_t i) {
    if (i > UINT32_MAX) {
      return UINT32_MAX;
    }
    return (uint32_t)i;
}

inline int32_t jint_to_sint32(json_int_t i) {
    if (i > INT32_MAX) {
      return INT32_MAX;
    }
    return (int32_t)i;
}

inline uint8_t jint_to_uint8(json_int_t i) {
    if (i > UINT8_MAX) {
      return UINT8_MAX;
    }
    return (uint8_t)i;
}

inline uint64_t jint_to_uint64(json_int_t i) {
  return (uint64_t)i;
}

inline size_t jint_to_size(json_int_t i) {
  // long long < unsigned long long
  return (size_t)i;
}

/** class data
    A JSON json_t opaque data storage for 
    structured data in files and memory.
**/
class data: public iresource {
public:

  /*
   * Data Exception details class with
   * optional parsing error details from jansson lib
   */
  class json_exception: public std::runtime_error {
  public:
    json_exception(const char * msg, json_error_t * err = nullptr)
      :runtime_error(msg)
    {
      if (err == nullptr)
        return;
      // copy error details
      line = err->line;
      column = err->column;
      position = err->position;
      SDL_strlcpy(source, err->source, JSON_ERROR_SOURCE_LENGTH);
      SDL_strlcpy(text, err->text, JSON_ERROR_TEXT_LENGTH);
    }

    /*
     * JSON exception details
     */
    int line;
    int column;
    int position;
    char source[JSON_ERROR_SOURCE_LENGTH];
    char text[JSON_ERROR_TEXT_LENGTH];
  };

  /*
   * Class data::json
   * A wrapper over json_t without ownership
   * Providers plain types to/from 
   * conversion template functions
   */
  struct json : public json_t {
    
    /*
     * Create new instances
     */    
    static data::json * array() { return (data::json *) json_array(); }
    static data::json * object() { return (data::json *) json_object(); }
    static data::json * integer(json_int_t ival) { return (data::json *) json_integer(ival); }
    static data::json * real(double dval) { return (data::json *) json_real(dval); }
    static data::json * boolean(bool bval) { return (data::json *) json_boolean(bval); }
    static data::json * string(const char * sval) { return (data::json *) json_string(sval); }
    
    // copy is in fact read-only
    static data::json * copy(const json_t * other) { return (data::json *) json_deep_copy(other); }
    static data::json * copy(const data::json * other) { return (data::json *) json_deep_copy(other); }

    /* Unpack contained JSON with provided format */
    void unpack(const char *fmt, ...) const;

    bool is_true() const
    {
      if (!json_is_boolean(this)) {
        throw json_exception("instance is not a boolean");
      }
      return json_is_true(this);
    }
    /* 
     * Get this json value as plain type value.
     * Template methods to convert into custom type 
     */
    template<typename T> T as() const;
  };


  /*
   * Class data::object_iterator
   * std::iterator implementation for
   * traversing json objects with string key
   * by key name. Instance is an equvivalent 
   * of a key object.
   */
  class object_iterator: public std::iterator<
                         std::input_iterator_tag, // iterator_category
                         const data,              // value_type
                         int,                     // difference_type
                         const data*,             // pointer -> a value of the data
                         void*>                   // reference -> a string key in dictionary
  {
  public:
    // new iterator with given values
    explicit object_iterator(data::json * p, void * i): 
      _p(p),
      _i(i)
    {}
    // new iterator from the fist key of the dict
    explicit object_iterator(const data & d): 
      _p(*d),
      _i(json_object_iter(_p))
    {}

    // end iterator
    explicit object_iterator():
      _p(nullptr),
      _i(nullptr)
    {}

    ~object_iterator() {}

    // iterate over keys of the json object
    object_iterator& operator++() { _i = json_object_iter_next(_p, _i); return *this; }
    object_iterator operator++(int) { object_iterator retval = *this; ++(*this); return retval; }

    // compare iterator to others and end() operator(_i == nullptr)
    bool operator==(object_iterator other) const { return _i == other._i; }
    bool operator!=(object_iterator other) const { return !(*this == other); }

    // get referece to a real operator type -> (void *)
    object_iterator::reference operator*() const {
      return _i;
    }

    // get dict key -> string
    const char * key() const { return json_object_iter_key(_i); }

     // get dict value converted to into supported type
    template<class T> T value() const { return value()->as<T>(); }

    // get dict value -> data::json
    data::json * value() const {
      return (data::json *) json_object_iter_value(_i);
    }

    // assign json_t value to this key
    // the instance of json_t is expected to be a newly created one
    void set_value(data::json * val)
    {
      json_object_iter_set(_p, _i, val);
    }

    /*
     * "operator=" implementation
     * Assign type specific value to this key
     */
    template<class T>
    void operator=(const T& val);

    // query value type helpers
    bool is_value_number() { return json_is_number(value()); }
    bool is_value_integer() { return json_is_integer(value()); }
    bool is_value_real() { return json_is_real(value()); }
    bool is_value_string() { return json_is_string(value()); }
    bool is_value_object() { return json_is_object(value()); }
    bool is_value_array(size_t of_size = 0) 
    { 
      data::json * val = value();
      return (json_is_array(val) && (of_size > 0 ? json_array_size(val) == of_size : true) ); 
    }

  private:
    data::json * _p;
    void * _i;
  };


  /*
   * Class data::array_iterator
   * std::iterator implementation for
   * traversing json arrays with size_t indexes.
   * Iterator instance holds index in the array.
   */
  class array_iterator: public std::iterator<
                         std::input_iterator_tag, // iterator_category
                         const data,              // value_type
                         int,                     // difference_type
                         const data*,             // pointer -> a value of the data
                         const data::json *>      // reference -> a const value of data::json
  {
  public:
    // new iterator with given values
    explicit array_iterator(data::json * p, size_t i): 
      _p(p),
      _i(i)
    {}
    // new iterator from the fist key of the dict
    explicit array_iterator(const data & d): 
      _p(*d),
      _i(0)
    {}

    // end iterator
    explicit array_iterator():
      _p(nullptr),
      _i(0)
    {}

    ~array_iterator() {}

    // assign json_t value to this key
    // the instance of json_t is expected to be a newly created one
    void set(data::json * val)
    {
      json_array_set(_p, _i, val);
    }

    // iterate over keys of the json object
    array_iterator& operator++() { ++_i; return *this; }
    array_iterator operator++(int) { array_iterator retval = *this; ++(*this); return retval; }

    // compare iterator to others and end() operator(_i == nullptr)
    bool operator==(array_iterator other) const { return (_p == other._p && _i == other._i); }
    bool operator!=(array_iterator other) const { return !(*this == other); }

    // get pointer to the underlying data::json
    array_iterator::reference operator*() const {
      return item();
    }

    // get pointer to the underlying data::json
    array_iterator::reference item() const {
      data::json * item = (data::json *) json_array_get(_p, _i);
      return item;
    }

    /* return underlying index in array */
    size_t index() { return _i; }

    /*
     * "operator=" implementation
     * Assign type specific value to this key
     */
    template<class T>
    void operator=(const T& val);

    // query value type helpers
    bool is_value_number() { return json_is_number(item()); }
    bool is_value_integer() { return json_is_integer(item()); }
    bool is_value_real() { return json_is_real(item()); }
    bool is_value_string() { return json_is_string(item()); }
    bool is_value_object() { return json_is_object(item()); }
    bool is_value_array(size_t of_size = 0) 
    { 
      const data::json * val = item();
      return (json_is_array(val) && (of_size > 0 ? json_array_size(val) == of_size : true) ); 
    }

  private:
    data::json * _p;
    size_t _i;
  };


  /**********************************************
   * Data Class - Opaque JSON storage with
   * automatic C++ refcount support of inner
   * json_t type. The type json_t is a type
   * from jansson library.
   */

  /* Assign a new instance of data with given json_t *
     This instance will take ownership by incref-ing
     the refcount of passed data::json
   */
  data(data::json * p):_json(nullptr), _f(0)
  {
    assign(p);
  }

  /* Assign a new instance of data with given json_t *
     This instance will take ownership by incref-ing
     the refcount of passed data::
   */
  data(json_t * p):_json(nullptr), _f(0)
  {
    assign( (data::json *) p);
  }

  /* Create a new instance with full copy of given const data */
  data(const data & d):_json(nullptr), _f(0)
  {
    json_t * j = json_deep_copy( *d );
    assign( (data::json *) j);
    json_decref(j);
  }
  
  /* Create a new instance with full copy of given const json */
  data(const data::json * p):_json(nullptr), _f(0)
  {
    data::json * j = (data::json *) json_deep_copy(p);
    assign(j);
    json_decref(j);
  }

  /* null constructor */
  data():_json(nullptr), _f(0)
  {}

  /* Assign this instance with given json and incref it*/
  void assign(data::json * p)
  {
    if (p == nullptr)
      return;

    
    if (_json != nullptr) {
      // release owned data
      json_decref(_json);
    }

    // assign new data
    _json = p;
    json_incref(_json);
  }

  /* parse json string */
  data(const std::string & json_string, uint32_t parser_flags);

  /* serialize this data as json string */
  std::string tostr(int ident = 2) const;
  
  /* load from a file at some given path */
  data(const std::string & json_file);

  /* Load json data from a file given by path */
  virtual void load(const std::string & json_file);

  /* Get access to the data::json inner value */
  data::json * operator*() const {
    if (!_json)
      throw json_exception("Access to a data object with no json pointer");
    return _json;
  }

  data::json & value() const {
    if (!_json)
      throw json_exception("Access to a data object with no json pointer");
    return *_json;
  }

  /*
   * Iterate object via keys 
   */
  object_iterator object_begin() const { return object_iterator(_json, json_object_iter(_json)); }
  object_iterator object_end() const { return object_iterator(_json, nullptr); }

  /*
   * Check if this data dictionary contains given string key
   */
  bool has_key(const std::string & key) const
  {
    if (!_json || !json_is_object(_json))
      throw json_exception("data is not a object");

    json_t * j = json_object_get(_json, key.c_str());
    return (j != NULL);
  }

  /*
    * Check if this data of nested dictionaries contains given subset
    * if string keys in format "key.subkey.subsubkey"
    */
  bool has_subkey(const std::string & key) const
  {
    if (!_json || !json_is_object(_json)) 
      throw json_exception("data is not a object");

    data::json * val = nullptr;
    void * it = find_iter_recursive(key.c_str(), val);
    if (it == nullptr) {
      // not found
      return false;
    }
    if (val == _json) {
      // found on this level
      return false;
    }
    return true;
  }

  ~data()
  {
    if (_json != NULL) {
      json_decref(_json);
      _json = NULL;
    }
  }

  /*
   * Set dictionary pair with string key
   */
  void set(const char* key, json_t * val)
  {
    if (!_json || !json_is_object(_json)) {
      SDL_Log("data::set - instance is not an object");
      throw json_exception("instance is not an object");
    }
    json_object_set(_json, key, val);
  }

  template<typename T>
  void set(const char* key, const T & val);

  /* Get dictionary item by string key */
  data::json * get(const char* key) const
  {
    if (!_json || !json_is_object(_json)) {
      SDL_Log("data::get - instance is not an object");
      throw json_exception("instance is not a object");
    }
    return (data::json *) json_object_get(_json, key);
  }
  
  /* Get dictionary item by string key */
  template<typename T>
  T get(const char* key, T def_value = T()) const
  {
    data::json * p = get(key);
    if (p == NULL) 
      return def_value;
    return p->as<T>();
  }

  /*
   *  Get dictionary item iterator by string key
   */
  object_iterator operator[](const std::string& key) const
  {
    if (!_json || !json_is_object(_json)) {
      SDL_Log("data::operator[std::string] - instance is not an object");
      throw json_exception("instance is not a object");
    }

    void * it = NULL;
    if (key.find(".") == UINT32_MAX) {
      // there is no dots in key, so check this level only
      it = json_object_iter_at(_json, key.c_str()); 
    }
    else {
      // check sublevels
      data::json * dummy = nullptr;
      it = find_iter_recursive(key.c_str(), dummy);
    }

    if (it == NULL) {
      SDL_Log("%s - key \"%s\" does not exist",
        __METHOD_NAME__, key.c_str());
      throw json_exception("Key does not exist");
    }
    return object_iterator(_json, it);
  }


  /*
   * Add array item to at the back
   */
  void push_back(json_t * val)
  {
    if (!_json || !json_is_array(_json)) {
      SDL_Log("%s - instance is not an array",
        __METHOD_NAME__);
      throw json_exception("instance is not an array");
    }

    json_array_append(_json, val);
  }

  /*
   * Add array item at index
   */
  void insert(size_t idx, json_t * val)
  {
    if (!_json || !json_is_array(_json)) {
      SDL_Log("%s - instance is not an array",
        __METHOD_NAME__);
      throw json_exception("instance is not an array");
    }

    json_array_insert(_json, idx, val);
  }
  
  /*
   * Iterate arrays via indexes 
   */
  array_iterator array_begin() const { return array_iterator(_json, 0); }
  array_iterator array_end() const { return array_iterator(_json, json_array_size(_json) ); }

  /*
   * Get array item by index
   */
  template<typename T>
  T at(size_t idx, T def_value = T()) const
  {
    if (!_json || !json_is_array(_json)) {
      SDL_Log("%s - instance is not an array",
        __METHOD_NAME__);
      throw json_exception("instance is not an array");
    }
    data item(json_array_get(_json, idx));
    if (item.is_null()) 
      return def_value;
    return item.as<T>();
  }

  /*
   *  Get array item as data by index
   */
  data operator[](size_t idx) const
  {
    if (!is_array()) {
      SDL_Log("data::operator[size_t] - instance is not an array");
      throw json_exception("data is not an array");
    }
    if (idx >= json_array_size(_json)) {
      SDL_Log("data::operator[size_t] - index is out of range");
      throw json_exception("index out of data array range");
    }
    return data( (data::json *) json_array_get(_json, idx));
  }

  /*
    * Check if this data is empty
    */
  bool is_null() const
  {
    return (!_json || json_is_null(_json));
  }

  /*
    * Check if this data is dictionary
    */
  bool is_object() const
  {
    return (_json && json_is_object(_json));
  }

  /*
    * Check if this data is array
    */
  bool is_array(const size_t of_size = 0) const
  {
    return (_json && json_is_array(_json) && ( of_size == 0 ? true : json_array_size(_json) == of_size ));
  }

  /*
    * Check if this data is string
    */
  bool is_string() const
  {
    return (_json && json_is_string(_json));
  }

  bool is_number() const
  {
    return json_is_number(_json);
  }

  bool is_real() const
  {
    return json_is_real(_json);
  }

  bool is_integer() const
  {
    return json_is_integer(_json);
  }

  /*
    * Get length of an indexable item
    * such as a string, an object or an array
    */
  size_t length() const
  {
    if (!_json)
      throw json_exception("Null value does not have length");

    if (is_string()) {
      return SDL_strlen(json_string_value(_json));
    }
    if (is_array())
    {
      return json_array_size(_json);
    }

    if (is_object())
    {
      return json_object_size(_json);
    }
    throw json_exception("data is not indexable");
  }

  /* 
  * Get this data as plain type value.
  * Template methods to convert into custom type 
  */
  template<typename T> T as() const
  {
    if (!is_null())
      throw json_exception("Cannot convert null value");

    return _json->as<T>();
  }

  /* Unpack contained JSON with provided format */
  void unpack(const char *fmt, ...) const;
  
protected:
  // lookup in dictionary
  void * find_iter_recursive(const char* key, data::json *& found) const;

  data::json * _json;
  uint32_t _f;
};

/* data::array_iterator template specialization */
template<> inline 
void data::array_iterator::operator=(const data& val) 
{
  // assign a copy of val
  json_array_set_new(_p, _i, json_deep_copy( *val ));
}

template<> inline 
void data::array_iterator::operator=(const bool & bval)
{
  data::json * val = data::json::boolean(bval);
  set(val);
  json_decref(val);
}

template<> inline 
void data::array_iterator::operator=(const std::string & sval)
{
  data::json * val = data::json::string(sval.c_str());
  set(val);
  json_decref(val);
}

template<> inline 
void data::array_iterator::operator=(const int& ival)
{
  data::json * val = data::json::integer(ival);
  set(val);
  json_decref(val);
}

/* data::object_iterator template specialization */

template<> inline 
void data::object_iterator::operator=(const data& val) 
{
  // assign a copy of val
  json_object_iter_set_new(_p, _i, json_deep_copy(*val));
}

template<> inline 
void data::object_iterator::operator=(const bool & bval)
{
  data::json * val = data::json::boolean(bval);
  set_value(val);
  json_decref(val);
}

template<> inline 
void data::object_iterator::operator=(const std::string & sval)
{
  data::json * val = data::json::string(sval.c_str());
  set_value(val);
  json_decref(val);
}

template<> inline 
void data::object_iterator::operator=(const int& ival)
{
  data::json * val = data::json::integer(ival);
  set_value(val);
  json_decref(val);
}

/* data::json template specializations */

template<> inline 
void data::set(const char* key, const std::string & val)
{
  set(key, json_string(val.c_str()));
}

template<> inline 
void data::set(const char* key, const int & val)
{
  set(key, json_integer(val));
}

template<> inline size_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("data object is not an integer");

  json_int_t i = json_integer_value(this);
  return jint_to_size(i);
}

template<> inline int32_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("data object is not an integer");

  json_int_t i = json_integer_value(this);
  return jint_to_sint32(i);
}

template<> inline uint32_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("data object is not an integer");

  json_int_t i = json_integer_value(this);
  return jint_to_uint32(i);
}

template<> inline uint8_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("object is not an integer");

  json_int_t i = json_integer_value(this);
  return jint_to_uint8(i);
}

template<> inline uint64_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("object is not an integer");

  json_int_t i = json_integer_value(this);
  return jint_to_uint64(i);
}

template<> inline int64_t data::json::as() const
{
  if (!json_is_integer(this))
    throw json_exception("object is not an integer");

  return json_integer_value(this);
}

template<> inline point data::json::as() const
{
  if (!json_is_array(this) || json_array_size(this) != 2) {
    throw json_exception("invald object or array lenght for a point array");
  }
  point p;
  unpack("[ii]", &p.x, &p.y);
  return p;  
}

template<> inline rect data::json::as() const
{
  if (!json_is_array(this) || json_array_size(this) != 4) {
    throw json_exception("invald object or array lenght for a rect array");
  }
  rect r;
  unpack("[iiii]", &r.x, &r.y, &r.w, &r.h);
  return r;
}

template<> inline color data::json::as() const
{
  if (!json_is_array(this) || json_array_size(this) != 4) {
    throw json_exception("invald object or array lenght for a color array");
  }
  color c;
  json_int_t r = 0, g = 0, b = 0, a = 0;
  unpack("[iiii]", &r, &g, &b, &a);
  c.r = jint_to_uint8(r);
  c.g = jint_to_uint8(g);
  c.b = jint_to_uint8(b);
  c.a = jint_to_uint8(a);
  return c;
}

template<> inline const char * data::json::as() const
{
  if (!json_is_string(this)) throw json_exception("object is not a string");
  const char * s = json_string_value(this);
  return s;
}

template<> inline std::string data::json::as() const
{
  return std::string(as<const char*>());
}

template<> inline std::pair<int,int> data::json::as() const
{
  if (!json_is_array(this) || json_array_size(this) != 2) {
    throw json_exception("invald object or array lenght for a std::pair array");
  }
  int f = 0, s = 0;
  unpack("[ii]", &f, &s);
  return std::make_pair(f, s);
}

template<> inline bool data::json::as() const
{
  return (json_typeof(this) == JSON_TRUE);
}

/* Load json object from resources */
data::json *      GM_LoadJSON(const std::string& file);

#endif //_GM_DATA_H_