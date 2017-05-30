#include "pyscript.h"
#include "util.h"

namespace python {

/* Check if list or tuple contain an object */
bool list_contains(PyObject * obj, PyObject * needle)
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

void append_path(const char *path)
{
  PyObject	*mpath, *spath;

  if ((mpath = PyUnicode_FromString(path)) == NULL)
    throw script::script_error("python::append_path - PyUnicode_FromString failed");

  if ((spath = PySys_GetObject("path")) == NULL)
    throw script::script_error("python::append_path - PySys_GetObject failed");

  PyList_Append(spath, mpath);
  Py_DECREF(mpath);
}

void script::script_error::collect()
{
  PyObject	*type = NULL,
            *value = NULL,
            *traceback = NULL;
  PyObject	*stype = NULL,
            *svalue = NULL,
            *straceback = NULL;

  if (!PyErr_Occurred() || PyErr_ExceptionMatches(PyExc_StopIteration))
    return;

  PyErr_Fetch(&type, &value, &traceback);
  _msg = std::string("unknown exception");

  if (type == NULL || value == NULL || traceback == NULL) {
    SDL_Log("script::python - unknown exception");
    return;
  }
  if (type != NULL) {
    stype = PyObject_Str(type);
    if (stype == NULL) {
      SDL_Log("script::python - failed to collect exception type");
      return;
    }
  }
  if (value != NULL) {
    svalue = PyObject_Str(value);
    if (svalue == NULL) {
      SDL_Log("script::python - failed to collect exception type");
      return;
    }
  }
  if (traceback != NULL) {
    straceback = PyObject_Str(traceback);
    if (straceback == NULL) {
      SDL_Log("script::python - failed to collect exception type");
      return;
    }
  }

  char *tp    = PyUnicode_AsUTF8(stype),
       *val   = PyUnicode_AsUTF8(svalue);

  std::stringstream ss;
  ss << "error: ";
  if (tp)
    ss << " " << tp << "";
  if (val)
    ss << " \"" << val << "\"";

  Py_DECREF(type);
  Py_DECREF(value);
  Py_DECREF(traceback);

  _msg = ss.str();
}

script::script_error::script_error(const char * msg)
{
  collect();
  _msg  = _msg + ". " + msg;
}

script::script_error::script_error(const std::stringstream & ss)
{
  collect();
  _msg  = _msg + ". " + ss.str();
}

script::script_error::script_error()
{
  collect();
}

const char* script::script_error::what() const _NOEXCEPT
{
  return _msg.c_str();
}

script::script(const std::string file_path):
  _py_module(NULL)
{
  path script_path(media_path(file_path));
  _name = script_path.stem().string();

  // append path to the module to sys.path list
  append_path(script_path.parent_path().string().c_str());

  // load module object
  _py_module = PyImport_ImportModule(_name.c_str());

  if (_py_module == NULL || PyErr_Occurred()) {
    SDL_Log("%s - failed to load python script from %s",
      __METHOD_NAME__, script_path.string().c_str());
    throw script_error();
  }
  SDL_Log("python::script - loaded module '%s'", _name.c_str());
}

script::~script()
{
  if (_py_module) {
    Py_DECREF(_py_module);
  }
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
PyObject* script::call_func_ex(const std::string & func_name, PyObject * args_tuple) const
{
  PyErr_Clear();

  if (_py_module == nullptr) {
    SDL_Log("python::script - no module \"%s\"", _name.c_str());
    throw script_error("module is not loaded");
  }

  PyObject * func = PyObject_GetAttrString(_py_module, func_name.c_str());
  if (func == NULL || PyErr_Occurred()) {
    SDL_Log("python::script - failed to find callable attribute %s",
      func_name.c_str());
    throw script_error();
  }

  if (!PyCallable_Check(func) || PyErr_Occurred()) {
    Py_XDECREF(func);

    SDL_Log("%s - python module attribute %s is not a function",
      __METHOD_NAME__, func_name.c_str());
    throw script_error("Called python module attribute is not a function");
  }

  // call python
  PyObject *ret = PyObject_Call(func, args_tuple, NULL);
  if (ret == NULL) {
    throw script::script_error();
  }

  return ret;
}

void script::call_func(json & ret, const std::string & func_name, const json & kwargs) const
{
  PyObject* pykwargs = from_json(kwargs);
  PyObject* py = (PyObject*)call_func_ex(func_name, pykwargs);
  ret = to_json(py);
  Py_XDECREF(py);
  Py_XDECREF(pykwargs);
}

bool script::has_func(const std::string & func_name) const
{
  PyErr_Clear();
  PyObject * func = PyObject_GetAttrString(_py_module, func_name.c_str());
  if (func == NULL) {
    return false;
  }

  if (!PyCallable_Check(func)) {
    Py_DECREF(func);
    return false;
  }

  Py_DECREF(func);
  return true;
}

void
register_const(PyObject *module, const char *name, long value)
{
  int ret;
  if ((ret = PyModule_AddIntConstant(module, name, value)) == -1)
    throw script::script_error(
        (std::stringstream() << "python::register_const - failed to add " << name));
}

void
register_type(const char *name, PyObject *module, PyTypeObject *type)
{
  if (PyType_Ready(type) == -1)
    throw script::script_error(
        (std::stringstream() << "python::register_type - failed to ready " << name));

  Py_INCREF(type);

  if (PyModule_AddObject(module, name, (PyObject *)type) == -1)
    throw script::script_error(
        (std::stringstream() << "python::register_type - failed to add " << name));
}

/****** Serialization ******/

json to_json(PyObject * py)
{
  // none
  if (py == Py_None) {
    return json();
  }

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
      p.push_back(to_json(item));
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
      p[key_str] = to_json(value);
      Py_DECREF(value);
    }
    Py_DECREF(iterator);
    return p;
  }

  SDL_Log("PyObject_AsJSON - unknown python object type, cannot convert to json.");
  throw python::script::script_error("unknown python type");
}

PyObject * from_json(const json & json_data)
{
  if (json_data.is_null()) {
    Py_RETURN_NONE;
  }
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
      PyObject * item = from_json(json_data.at(i));
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
      PyObject * val = from_json(*i);
      PyDict_SetItem(dict, key, val);
      Py_DECREF(key);
      Py_DECREF(val);
    }
    return dict;
  }

  throw python::script::script_error("Failed to convert data instance into python.");
}

} // namespace python
