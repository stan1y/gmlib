#include <Python.h>
#include "script.h"
#include "util.h"

/* Check if list or tuple contain an object */
bool PyObject_Contains(PyObject * obj, PyObject * needle)
{
  if (!PyList_Check(obj) && !PyTuple_Check(obj)) {
    return false;
  }

  // iterate container
  PyObject *iterator = PyObject_GetIter(obj);
  PyObject *item;

  if (iterator == NULL) {
    return false;
  }
  // compare items
  bool found = false;
  while ( (item = PyIter_Next(iterator)) ) {
    if (PyObject_RichCompareBool(item, needle, Py_EQ)) {
      found = true;
      break;
    }
  }
  Py_DECREF(iterator);
  return found;
}

/* Add a path to a folder to sys.path list if it is not present */
void Py_AddModulePath(const std::string & folder)
{
  PyObject* module_path = PyUnicode_FromString(folder.c_str());
  PyObject* py_sys_path = PySys_GetObject((char*)"path");
  if (!PyObject_Contains(py_sys_path, module_path)) {
    SDL_Log("adding %s to python modules path", folder.c_str());
    PyList_Append(py_sys_path, module_path);
  }
  Py_DECREF(module_path);
}

/* create PyObject from json data */
static PyObject * PyObject_FromJSON(const json & json_data)
{
  if (json_data.is_number_integer()) {
    return PyLong_FromLong(json_data.get<long>());
  }
  if (json_data.is_number_float()) {
    return PyFloat_FromDouble(json_data.get<float>());
  }
  if (json_data.is_string()) {
    return PyUnicode_FromString(json_data.get<std::string>().c_str());
  }
  if (json_data.is_array()) {
    size_t sz = json_data.size();
    PyObject * arr = PyList_New(sz);
    for(size_t i = 0; i < sz; ++i) {
      PyObject * item = PyObject_FromJSON(json_data.at(i));
      PyList_SetItem(arr, i, item);
      Py_DECREF(item);
    }
    return arr;
  }
  if (json_data.is_object()) {
    PyObject * dict = PyDict_New();
    json::const_iterator i = json_data.begin();
    for(; i != json_data.end(); ++i) {
      PyObject * key = PyUnicode_FromString(i->get<std::string>().c_str());
      PyObject * val = PyObject_FromJSON(*i);
      PyDict_SetItem(dict, key, val);
      Py_DECREF(key);
      Py_DECREF(val);
    }
    return dict;
  }

  throw python::script::script_exception("Failed to convert data instance into python.");
}

namespace python {


/*
 * Python exception
 * collects python's error details
 */
script::script_exception::script_exception(const char * msg):
  runtime_error(msg)
{
  if (!PyErr_Occurred()) {
    return;
  }

  // Get python error traceback
  PyErr_Print();
  PyErr_Clear();
}


void initialize()
{
  Py_NoSiteFlag = 1;
  const config & cfg = config::current();
  path python_path(cfg.get_data()["python_path"].get<std::string>());
  wchar_t * wpython_path = Py_DecodeLocale(python_path.string().c_str(), NULL);
  Py_SetPath(wpython_path);
  
  Py_Initialize();
  SDL_Log("python - initialized %s", Py_GetVersion());
}

void shutdown()
{
  Py_Finalize();
}


script::script(const std::string file_path):
  _py_module(NULL)
{
  path script_path(media_path(file_path));
  _name = script_path.stem().string();

  // append path to the module to sys.path list
  Py_AddModulePath(script_path.parent_path().string());

  // load module object
  PyObject* module_name = PyUnicode_FromString(_name.c_str());
  _py_module = PyImport_Import(module_name);
  Py_DECREF(module_name);

  if (_py_module == NULL) {
    if (PyErr_Occurred())
      PyErr_Print();

    SDL_Log("%s - failed to load python script from %s",
      __METHOD_NAME__, script_path.string().c_str());
    throw python::script::script_exception("Failed to load python script");
  }
}

script::~script()
{
  if (_py_module) {
    Py_DECREF(_py_module);
  }
}

/* 
 * Convert PyObject to a value of json type
 */
static json PyObject_AsJSON(PyObject * py)
{
  // number is interger
  if (PyLong_Check(py)) {
    return json(PyLong_AsLong(py));
  }

  // number is double
  if (PyFloat_Check(py)) {
    return json(PyFloat_AsDouble(py));
  }

  // boolean
  if (PyBool_Check(py)) {
    return json(PyObject_IsTrue(py) == 1);
  }
  
  // unicode string
  if (PyUnicode_Check(py)) {
    Py_ssize_t sz = 0;
    char * utf8 = PyUnicode_AsUTF8AndSize(py, &sz);
    if (sz != 0) {
      return json(utf8);
    }
  }

  // tuple or list into array
  if (PyTuple_Check(py) || PyList_Check(py)) {
    PyObject *iterator = PyObject_GetIter(py);
    PyObject *item;

    if (iterator == NULL) {
      // proparate array error
      return NULL;
    }
    // init new array
    json p = json::array();
    while ( (item = PyIter_Next(iterator)) ) {
      // prepare and set child item
      p.push_back(PyObject_AsJSON(item));
      Py_DECREF(item);
    }
    Py_DECREF(iterator);
    return p;
  }

  // dict as object
  if (PyDict_Check(py)) {
    PyObject *iterator = PyObject_GetIter(py);
    PyObject *key = NULL, *value = NULL;

    if (iterator == NULL) {
      // proparate dict error
      SDL_Log("PyObject_AsJSON - failed to get iterator");
      throw std::runtime_error("failed to get iterator");
    }
    // init new dict
    json p = json::object();
    while ( (key = PyIter_Next(iterator)) ) {
      
      // key must be a unicode string for 
      // compatibility with JSON 
      if (!PyUnicode_Check(key)) {
        SDL_Log("%s - cannot convert non-string key in object",
          __METHOD_NAME__);
        return NULL;
      }

      // get value
      value = PyObject_GetItem(py, key);
      if (value == NULL) {
        // proparage key error
        SDL_Log("%s - failed to find value by key \"%s\"",
          __METHOD_NAME__, PyUnicode_AsUTF8(key));
        throw std::runtime_error("failed to find key in python object");
      }

      // convert key & value
      char * key_str = PyUnicode_AsUTF8(key);
      p[key_str] = PyObject_AsJSON(value);
      Py_DECREF(value);
    }
    Py_DECREF(iterator);
    return p;
  }

  SDL_Log("PyObject_AsJSON - unknown python object type, cannot convert to json.");
  throw python::script::script_exception("unknown python type");
}

/*
 * python::script object interface
 */

void script::call_func(json & ret, const std::string & func_name) const
{
  call_func(ret, func_name, json::object());
}

/*
 * callfunc and call_func_ex
 * Generic interface to call a function from
 * loaded python module
 */
void* script::call_func_ex(const std::string & func_name, const json & args) const
{
  if (_py_module == nullptr) {
    SDL_Log("%s - module %s is not initalized. cannot call \"%s\"",
      __METHOD_NAME__, _name.c_str(), func_name.c_str());
    throw script_exception("Cannot call function. Module object is not initialized.");
  }

  if (!args.is_object()) {
    SDL_Log("%s - python function arguments must be a dictionary. cannot call \"%s\"",
      __METHOD_NAME__, func_name.c_str());
    throw script_exception("Cannot call function. Arguments object is not a dictionary.");
  }

  PyObject * func = PyObject_GetAttrString((PyObject*)_py_module, func_name.c_str());
  if (func == NULL) {
    SDL_Log("%s - failed to find module attribute %s",
      __METHOD_NAME__, func_name.c_str());
    throw script_exception("Failed to find python module attribute");
  }

  if (!PyCallable_Check(func)) {
    Py_XDECREF(func);

    SDL_Log("%s - python module attribute %s is not a function",
      __METHOD_NAME__, func_name.c_str());
    throw script_exception("Called python module attribute is not a function");
  }

  PyObject * kwargs = PyObject_FromJSON(args);

  // call python
  PyObject * targs = PyTuple_New(0);
  PyObject * ret = PyObject_Call(func, targs, kwargs);
  Py_DECREF(targs);
  Py_DECREF(kwargs);
  if (ret == NULL) {
    Py_XDECREF(ret);

    SDL_Log("%s - python function call failed. %s:%s",
      __METHOD_NAME__, _name.c_str(), func_name.c_str());
    throw script::script_exception("Python function call failed");
  }

  return ret;
}

void script::call_func(json & ret, const std::string & func_name, const json & args) const
{
  PyObject* py = (PyObject*)call_func_ex(func_name, args);
  ret = PyObject_AsJSON(py);
  Py_XDECREF(py);
  if (ret == NULL) {
    SDL_Log("%s - failed to convert returned value of %s:%s",
      __METHOD_NAME__, _name.c_str(), func_name.c_str());
    throw script_exception("Failed to convert value returned from Python");
  }
}

} // namespace python
