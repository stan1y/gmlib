#ifndef _GM_PYTHON_H_
#define _GM_PYTHON_H_

#include <Python.h>
#include "GMLib.h"
#include "GMData.h"

namespace python {

/* initialize embedded python environment */
void initialize(const config * cfg);
void shutdown();

class script: public iresource {
public:

  class script_exception: public std::exception {
  public:
    script_exception(const char * msg);

  };

  /*
   * Python func call arugments
   * A list or dict based on data
   * with serialization into PyObject
   */
  class arguments: public data {
  public:
    static arguments arg() { return arguments(data::json::array()); }
    static arguments kwargs() { return arguments(data::json::object()); }

    // convert arguments into python
    // representation for func calls
    PyObject * to_python() const;

  private:
    arguments(json * p):data(p) { json_decref(as_json()); };
  };

  // load python script from path
  script(const std::string file_path);
  virtual ~script();

  // call python function and get result as data
  void call_func(data & ret, const std::string & func_name) const;
  void call_func(data & ret, const std::string & func_name, const arguments & args) const;

  // call python function and get result as raw PyObject
  PyObject * call_func_ex(const std::string & func_name) const;
  PyObject * call_func_ex(const std::string & func_name, const arguments & args) const;

  // get module's name. i.e. /path/to/[name].py
  const std::string & name() const { return _name; }

private:
  PyObject * _py_module;
  std::string _name;
};

};

#endif //_GM_PYTHON_H_