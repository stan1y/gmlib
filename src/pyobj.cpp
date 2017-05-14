#include "pyscript.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
#pragma clang diagnostic ignored "-Wc99-extensions"
#endif

/**
 * @brief pypebble_init
 * Initialize python exported objects
 * Must be called before Py_Initialize()
 */
PyMODINIT_FUNC          pygm_init(void);
static PyObject		     *pygm_log(PyObject *, PyObject *);

static struct PyMethodDef pygm_methods[] = {
  METHOD("log", pygm_log, METH_VARARGS),
  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef pygm_module = {
  PyModuleDef_HEAD_INIT, "gm", NULL, -1, pygm_methods
};

/**
 * @brief The pypoint struct
 */

struct pypoint {
  PyObject_HEAD
  point * p;
};

static PyMethodDef pypoint_methods[] = {
  METHOD(NULL, NULL, -1),
};

static PyObject   *pypoint_getx(struct pypoint *, void *);
static PyObject   *pypoint_gety(struct pypoint *, void *);

static PyGetSetDef pypoint_getset[] = {
  GETTER("x", pypoint_getx),
  GETTER("y", pypoint_gety),
  GETTER(NULL, NULL),
};

static void        pypoint_dealloc(struct pypoint *);

static PyTypeObject pypoint_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "gm.point",
  .tp_doc = "struct point",
  .tp_getset = pypoint_getset,
  .tp_methods = pypoint_methods,
  .tp_basicsize = sizeof(struct pypoint),
  .tp_dealloc = (destructor)pypoint_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};

static void        pypoint_dealloc(struct pypoint *)
{

}

static
PyObject*
pypoint_getx(struct pypoint *pnt, void *)
{
  return PyLong_FromLong(pnt->p->x);
}

static
PyObject*
pypoint_gety(struct pypoint *pnt, void *)
{
  return PyLong_FromLong(pnt->p->y);
}

/**** Log Utility ****/

static
PyObject*
pygm_log(PyObject *self, PyObject *args)
{
  const char	*msg;

  if (!PyArg_ParseTuple(args, "s", &msg)) {
    PyErr_SetString(PyExc_TypeError, "function expects string argument");
    return (NULL);
  }

  SDL_Log("python: %s", msg);
  Py_RETURN_TRUE;
}

/***** Initialization *****/

PyMODINIT_FUNC
pygm_init(void)
{
  PyObject	*pygm;

  if ((pygm = PyModule_Create(&pygm_module)) == NULL)
    throw python::script::script_error("pygm_init: failed to setup 'pygm' module");

  python::register_type("pypoint", pygm, &pypoint_type);

  return pygm;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace python {

PyObject*
pypoint_alloc(point *p)
{
  struct pypoint *pnt;
  pnt = (struct pypoint*)PyObject_New(struct point, &pypoint_type);
  if (pnt == NULL)
    return NULL;

  pnt->p = p;
  return (PyObject *)pnt;
}

void initialize()
{
  Py_NoSiteFlag = 1;
  const config & cfg = config::current();
  path python_path(cfg.get_data()["python_path"].get<std::string>());
  wchar_t * wpython_path = Py_DecodeLocale(python_path.string().c_str(), NULL);
  Py_SetPath(wpython_path);

  if (PyImport_AppendInittab("gm", &pygm_init) == -1) {
    throw script::script_error("PyImport_AppendInittab: failed to add 'gm' module");
  }

  Py_Initialize();
  SDL_Log("python::initialize - %s, %s", Py_GetVersion(), python_path.string().c_str());
}

void shutdown()
{
  Py_Finalize();
}

} // namespace python
