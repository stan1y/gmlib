#ifndef _GM_PYTHON_H_
#define _GM_PYTHON_H_

#include "engine.h"

namespace python {

/* initialize embedded python environment */
void initialize();
void shutdown();

class script {
public:

  class script_exception: public std::runtime_error {
  public:
    script_exception(const char * msg);

  };

  // load python script from path
  script(const std::string file_path);
  virtual ~script();

  // call python function and get result as data
  void call_func(json & ret, const std::string & func_name) const;
  void call_func(json & ret, const std::string & func_name, const json & args) const;

  // call python function and get result as raw PyObject
  void* call_func_ex(const std::string & func_name) const;
  void* call_func_ex(const std::string & func_name, const json & args) const;

  // get module's name. i.e. /path/to/[name].py
  const std::string & name() const { return _name; }

private:
  void* _py_module;
  std::string _name;
};

} // namespace python

#endif //_GM_PYTHON_H_
