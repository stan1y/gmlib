#include <Python.h>
#include "script.h"

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
#ifdef GM_DEBUG
    SDL_Log("%s - adding %s to python modules path",
      __METHOD_NAME__, folder.c_str());
#endif
    PyList_Append(py_sys_path, module_path);
  }
  Py_DECREF(module_path);
}

/* create PyObject from json data */
static PyObject * PyObject_FromJSON(const json & json_data)
{/*
   FIXME: json to python
  if (json_is_integer(json_data)) {
    return PyLong_FromDouble(json_number_value(json_data));
  }
  if (json_is_real(json_data)) {
    return PyFloat_FromDouble(json_number_value(json_data));
  }
  if (json_is_string(json_data)) {
    return PyUnicode_FromString(json_string_value(json_data));
  }
  if (json_is_array(json_data)) {
    size_t sz = json_array_size(json_data);
    PyObject * arr = PyList_New(sz);
    for(size_t i = 0; i < sz; ++i) {
      PyObject * item = PyObject_FromJSON(json_array_get(json_data, i));
      PyList_SetItem(arr, i, item);
      Py_DECREF(item);
    }
    return arr;
  }
  if (json_is_object(json_data)) {
    PyObject * dict = PyDict_New();
    void * it = json_object_iter((json_t *) json_data);
    for(; it != nullptr; it = json_object_iter_next((json_t *) json_data, it)) {
      PyObject * key = PyUnicode_FromString(json_object_iter_key(it));
      PyObject * val = PyObject_FromJSON(json_object_iter_value(it));
      PyDict_SetItem(dict, key, val);
      Py_DECREF(key);
      Py_DECREF(val);
    }
    return dict;
  }*/

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
  SDL_Log("%s: initialized python runtime %s",
          __METHOD_NAME__,
          Py_GetVersion());
}

void shutdown()
{
  Py_Finalize();
}


script::script(const std::string file_path):
  _py_module(NULL)
{
  path script_path(file_path);
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
      __METHOD_NAME__, file_path.c_str());
    throw script_exception("Failed to load python script");
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
static json * PyObject_AsJSON(PyObject * py)
{
  json * p = nullptr;
  /*
   * FIXME: python to json
   *
  // number is interger
  if (PyLong_Check(py)) {
    p = data::json::integer(PyLong_AsLong(py));
  }

  // number is double
  if (PyFloat_Check(py)) {
    p = data::json::real(PyFloat_AsDouble(py));
  }

  // boolean
  if (PyBool_Check(py)) {
    p = data::json::boolean(PyObject_IsTrue(py) == 1);
  }
  
  // unicode string
  if (PyUnicode_Check(py)) {
    Py_ssize_t sz = 0;
    char * utf8 = PyUnicode_AsUTF8AndSize(py, &sz);
    if (sz != 0) {
      p = data::json::string(utf8);
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
    p = data::json::array();
    while ( (item = PyIter_Next(iterator)) ) {
      // prepare and set item  
      data::json * jitem = PyObject_AsJSON(item);
      Py_DECREF(item);
      if (jitem == NULL) {
        // proparate error
        return NULL;
      }
      json_array_append(p, jitem);
    }
    Py_DECREF(iterator);
  }

  // dict as object
  if (PyDict_Check(py)) {
    PyObject *iterator = PyObject_GetIter(py);
    PyObject *key = NULL, *value = NULL;

    if (iterator == NULL) {
      // proparate dict error
      return NULL;
    }
    // init new dict
    p = data::json::object();
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
        return NULL;
      }

      // convert key & value
      char * key_str = PyUnicode_AsUTF8(key);
      data::json * jval = PyObject_AsJSON(value);
      Py_DECREF(value);
      if (jval == NULL) {
        // proparage value error
        return NULL;
      }
      json_object_set(p, 
                      key_str,
                      jval);
    }
    Py_DECREF(iterator);
  }
  */
  return p;
}

/*
 * python::script object interface
 */

void script::call_func(json * ret, const std::string & func_name) const
{
  arguments args = arguments::kwargs();
  call_func(ret, func_name, args);
}

/*
 * callfunc and call_func_ex
 * Generic interface to call a function from
 * loaded python module
 */
void* script::call_func_ex(const std::string & func_name, const script::arguments & args) const
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

void script::call_func(json * ret, const std::string & func_name, const arguments & args) const
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



};
