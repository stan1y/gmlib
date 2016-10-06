#include "GMPython.h"
#include "Python.h"

#include <boost/filesystem.hpp>

namespace python {

void initialize(const config * cfg) 
{
  auto python_home = boost::filesystem::current_path();
  wchar_t * wpython_home = Py_DecodeLocale(python_home.string().c_str(), NULL);
  Py_SetPythonHome(wpython_home);

  auto python_lib = python_home / "lib";
  wchar_t * wpython_lib = Py_DecodeLocale(python_lib.string().c_str(), NULL);
  Py_SetPath(wpython_lib);

  Py_SetStandardStreamEncoding("utf-8", "utf-8");
  Py_Initialize();
#ifdef GM_DEBUG_PYTHON
  SDL_Log("python::initialize - done %s", Py_GetVersion());
#endif
}

void shutdown()
{
  Py_Finalize();
}

script::script(const std::string file_path):
  _py_module(NULL)
{
  boost::filesystem::path script_path(file_path);
  std::string folder = script_path.parent_path().string();
  _name = script_path.stem().string();

  // append path to the module to sys.path list
  PyObject* module_path = PyUnicode_FromString(folder.c_str());
  PyObject* py_sys_path = PySys_GetObject((char*)"path");
  PyList_Append(py_sys_path, module_path);
  Py_DECREF(module_path);

  // load module object
  PyObject* module_name = PyUnicode_FromString(_name.c_str());
  _py_module = PyImport_Import(module_name);
  Py_DECREF(module_name);

  if (_py_module == NULL) {
    if (PyErr_Occurred())
      PyErr_Print();

    SDLEx_LogError("%s - failed to load python script from %s",
      __METHOD_NAME__, file_path.c_str());
    throw std::exception("Failed to load python script");
  }
}

script::~script()
{
  if (_py_module) {
    Py_DECREF(_py_module);
  }
}

/*
 * call_func_ex
 * Generic interface to call a function from
 * loaded python module
 */


PyObject * call_func_ex(const script & src, const std::string & func_name, const script::arguments & args)
{
  PyObject * func = PyObject_GetAttrString((PyObject*)src.module(), func_name.c_str());
  if (func == NULL) {
    if (PyErr_Occurred())
      PyErr_Print();

    SDLEx_LogError("%s - failed to find module attribute %s",
      __METHOD_NAME__, func_name.c_str());
    throw std::exception("Failed to find python module attribute");
  }

  if (!PyCallable_Check(func)) {
    Py_XDECREF(func);

    SDLEx_LogError("%s - python module attribute %s is not a function",
      __METHOD_NAME__, func_name.c_str());
    throw std::exception("Called python module attribute is not a function");
  }

  // build key-value dict with args
  PyObject * kwargs = PyDict_New();
  PyObject * targs = PyTuple_New(0);
  script::arguments::const_iterator it = args.begin();
  for(; it != args.end(); ++it) {
    PyObject * v = PyUnicode_FromString(it->second.c_str());
    PyDict_SetItemString(kwargs, it->first.c_str(), v);
    Py_DECREF(v);
  }

  // call python
  PyObject * ret = PyObject_Call(func, targs, kwargs);
  Py_DECREF(targs);
  Py_DECREF(kwargs);
  if (ret == NULL) {
    Py_XDECREF(ret);
    if (PyErr_Occurred())
      PyErr_Print();

    SDLEx_LogError("%s - python function call failed. %s:%s",
      __METHOD_NAME__, src.name().c_str(), func_name.c_str());
    throw std::exception("Python function call failed");
  }

  return ret;
}

/*
 * python::script object interface
 */

void script::call_func(data & ret, const std::string & func_name) const
{
  arguments args;
  call_func(ret, func_name, args);
}

void script::call_func(data & ret, const std::string & func_name, const arguments & args) const
{
  PyObject * py = call_func_ex(*this, func_name, args);
  json_t * p = nullptr;
  // convert PyObject to a value of json type
  if (PyNumber_Check(py)) {
    PyObject * exc = nullptr;
    Py_ssize_t val = PyNumber_AsSsize_t(py, exc);
    p = json_integer(val);
  }
  if (PyUnicode_Check(py)) {
    Py_ssize_t sz = 0;
    char * utf8 = PyUnicode_AsUTF8AndSize(py, &sz);
    if (sz != 0) {
      p = json_string(utf8);
    }
  }

  if (p != nullptr) {
    ret.set_owner_of(p);
    json_decref(p);
  }

  Py_DECREF(py);
}

};