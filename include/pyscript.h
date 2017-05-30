#ifndef _GM_PYTHON_H_
#define _GM_PYTHON_H_

#include <Python.h>
#include "engine.h"
#include "sprite.h"
#include "texture.h"

/* Python module helpers */

#define METHOD(n, c, a)		{ n, (PyCFunction)c, a, NULL }
#define GETTER(n, g)		{ n, (getter)g, NULL, NULL, NULL }
#define SETTER(n, s)		{ n, NULL, (setter)g, NULL, NULL }
#define GETSET(n, g, s)		{ n, (getter)g, (setter)s, NULL, NULL }


namespace python {

/**
 * @brief setup
 * setup python runtime flags and built-in
 * modules path (Py_SetPath, Py_NoSiteFlag)
 */
void setup();

/**
 * @brief initialize
 * initialize embedded python runtime
 * call this method after extensions registration
 */
void initialize();

/**
 * @brief shutdown
 * shutdown embedded python runtime
 */
void shutdown();

/**
 * @brief append_path
 * @param path
 * append item to list of sys.path
 */
void append_path(const char *path);

/**
 * @brief list_contains
 * @return true or false
 * check of list contains given object
 */
bool list_contains(PyObject*);

/**
 * @brief from_json
 * @param json data
 * @return new python object
 * create new PyObject from json
 */
PyObject * from_json(const json &);

/**
 * @brief to_json
 * @param python object
 * @return json data
 * create json representation of PyObject
 */
json to_json(PyObject *);

/**
 * @brief register_const
 * register named constant integer in python runtime
 */
void register_const(PyObject *, const char *, long);

/**
 * @brief register_type
 * register named class type in python runtime
 */
void register_type(const char *, PyObject *, PyTypeObject *);


/*
 * Core GMLib Types python versions
 * rect
 * point
 * color
 * sprite
 * texture
 */

PyObject   *pypoint_alloc(const point &);
PyObject   *pyrect_alloc(const rect &);
PyObject   *pycolor_alloc(const color &);
PyObject   *pysprite_alloc(const sprite &);
PyObject   *pytexture_alloc(const texture *);

/**
 * @brief The script class
 * The python module api
 */
class script {
public:

  /**
   * @brief The script_error class
   * Generic python runtime error
   */
  class script_error: public std::exception {
  public:
    script_error(const std::stringstream &);
    script_error(const char *);
    script_error();

    virtual const char* what() const _NOEXCEPT;

  private:
    std::string _msg;
    void collect();
  };

  // load python script from path
  script(const std::string file_path);
  virtual ~script();

  // call python function and get result as data
  void call_func(json & ret, const std::string & func_name) const;
  void call_func(json & ret, const std::string & func_name, const json & kwargs) const;

  // call python function and get result as raw PyObject
  PyObject* call_func_ex(const std::string & func_name, PyObject * tuple) const;

  // check if module exports callable with name
  bool has_func(const std::string & func_name) const;

  // get module's name. i.e. /path/to/[name].py
  const std::string & name() const { return _name; }

private:
  PyObject* _py_module;
  std::string _name;
};

} // namespace python

#endif //_GM_PYTHON_H_
