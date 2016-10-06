#ifndef _GM_PYTHON_H_
#define _GM_PYTHON_H_

#include "GMLib.h"
#include "GMData.h"

namespace python {

/* initialize embedded python environment */
void initialize(const config * cfg);
void shutdown();

class script: public iresource {
public:
  typedef std::string argument_name;
  typedef std::string argument_value;
  typedef std::map<argument_name, argument_value> arguments;

  // load python script from path
  script(const std::string file_path);
  virtual ~script();

  void call_func(data & ret, const std::string & func_name) const;
  void call_func(data & ret, const std::string & func_name, const arguments & args) const;

  // get access to inner PyObject*
  void * module() const { return _py_module; }
  const std::string & name() const { return _name; }

private:
  void * _py_module;
  std::string _name;
};

};

#endif //_GM_PYTHON_H_