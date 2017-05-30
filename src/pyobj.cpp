#include "pyscript.h"


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
#pragma clang diagnostic ignored "-Wc99-extensions"
#endif

/**
 * @brief pypebble_module_init
 * Initialize python exported objects
 * Must be called before Py_Initialize()
 */
PyMODINIT_FUNC    pygm_module_init(void);
static PyObject		      *pygm_log(PyObject *, PyObject *);
/*
static void             *python_malloc(void *, size_t);
static void             *python_calloc(void *, size_t, size_t);
static void             *python_realloc(void *, void *, size_t);
static void              python_free(void *, void *);

static PyMemAllocatorEx allocator = {
  .ctx = NULL,
  .malloc = python_malloc,
  .calloc = python_calloc,
  .realloc = python_realloc,
  .free = python_free
};*/


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
  point p;
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
  .tp_doc = "struct pypoint",
  .tp_getset = pypoint_getset,
  .tp_methods = pypoint_methods,
  .tp_basicsize = sizeof(struct pypoint),
  .tp_dealloc = (destructor)pypoint_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};

static
void
pypoint_dealloc(struct pypoint *p)
{
  PyObject_Del((PyObject*)p);
}

static
PyObject*
pypoint_getx(struct pypoint *pnt, void *)
{
  return PyLong_FromLong(pnt->p.x);
}

static
PyObject*
pypoint_gety(struct pypoint *pnt, void *)
{
  return PyLong_FromLong(pnt->p.y);
}


/**
 * @brief The pysprite struct
 */

struct pysprite {
  PyObject_HEAD
  sprite s;
};

static PyObject *pysprite_getcliprect(struct pysprite *, PyObject *);
static PyObject *pysprite_render(struct pysprite *, PyObject *);

static PyMethodDef pysprite_methods[] = {
  METHOD("get_cliprect",   pysprite_getcliprect,   METH_NOARGS),
  METHOD("render",         pysprite_render,        METH_VARARGS),
  METHOD(NULL, NULL, -1),
};

static PyObject   *pysprite_getidx(struct pysprite *, void *);
static PyObject   *pysprite_getw(struct pysprite *, void *);
static PyObject   *pysprite_geth(struct pysprite *, void *);

static PyGetSetDef pysprite_getset[] = {
  GETTER("w", pysprite_getw),
  GETTER("h", pysprite_geth),
  GETTER("idx", pysprite_getidx),
  GETTER(NULL, NULL),
};

static void        pysprite_dealloc(struct pysprite *);

static PyTypeObject pysprite_type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "gm.sprite",
  .tp_doc = "struct pysprite",
  .tp_getset = pysprite_getset,
  .tp_methods = pysprite_methods,
  .tp_basicsize = sizeof(struct pysprite),
  .tp_dealloc = (destructor)pysprite_dealloc,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
};

static void
pysprite_dealloc(struct pysprite *p)
{
  PyObject_Del((PyObject*)p);
}

static PyObject*
pysprite_getcliprect(struct pysprite *self, PyObject *)
{
  return python::pyrect_alloc(self->s.get_clip_rect());
}

static PyObject*
pysprite_render(struct pysprite *self, PyObject *args)
{
  PyObject       *p   = NULL;
  struct pypoint *pnt = NULL;

  if (!PyArg_ParseTuple(args, "O", &p)) {
    PyErr_SetString(PyExc_TypeError, "function needs gm.point argument");
    return NULL;
  }

  if(Py_TYPE(p) != &pypoint_type) {
    PyErr_SetString(PyExc_TypeError, "argument is not gm.point type");
    return NULL;
  }
  pnt = (struct pypoint*)p;

  self->s.render(GM_GetRenderer(), pnt->p);
  Py_RETURN_NONE;
}

static PyObject*
pysprite_getidx(struct pysprite *self, void *)
{
  return PyLong_FromLong(self->s.idx);
}

static PyObject*
pysprite_getw(struct pysprite *self, void *)
{
  return PyLong_FromLong(self->s.w);
}

static PyObject* pysprite_geth(struct pysprite *self, void *)
{
  return PyLong_FromLong(self->s.h);
}

/**** Log Utility ****/

static
PyObject*
pygm_log(PyObject *self, PyObject *args)
{
  const char	*msg;

  if (!PyArg_ParseTuple(args, "s", &msg)) {
    PyErr_SetString(PyExc_TypeError, "function expects string argument");
    return NULL;
  }

  SDL_Log("python: %s", msg);
  Py_RETURN_NONE;
}

/***** Initialization *****/

PyMODINIT_FUNC
pygm_module_init(void)
{
  PyObject	*pygm;

  if ((pygm = PyModule_Create(&pygm_module)) == NULL)
    throw python::script::script_error("pygm_module_init: failed to setup 'pygm' module");

  python::register_type("pypoint", pygm, &pypoint_type);
  python::register_type("pysprite", pygm, &pysprite_type);

  return pygm;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace python {

/* Exported Types */

PyObject* pypoint_alloc(const point &p)
{
  struct pypoint *pnt;
  pnt = (struct pypoint*)PyObject_New(struct pypoint, &pypoint_type);
  if (pnt == NULL)
    return NULL;

  pnt->p = p;
  return (PyObject *)pnt;
}

PyObject* pyrect_alloc(const rect & r)
{
  Py_RETURN_NONE;
}

PyObject* pycolor_alloc(const color & c)
{
  Py_RETURN_NONE;
}

PyObject* pysprite_alloc(const sprite &s)
{
  struct pysprite *spr;
  spr = (struct pysprite*)PyObject_New(struct pysprite, &pysprite_type);
  if (spr == NULL)
    return NULL;

  spr->s = s;
  return (PyObject *)spr;
}

PyObject* pytexture_alloc(texture *tx)
{
  Py_RETURN_NONE;
}

/***** Initialization *****/

void setup()
{
  Py_NoSiteFlag = 1;
  const config & cfg = config::current();
  path python_path(cfg.get_data()["python_path"].get<std::string>());
  wchar_t * wpython_path = Py_DecodeLocale(python_path.string().c_str(), NULL);
  Py_SetPath(wpython_path);
  SDL_Log("python::setup - %s", python_path.string().c_str());
}

void initialize()
{
  if (PyImport_AppendInittab("gm", &pygm_module_init) == -1) {
    throw script::script_error("PyImport_AppendInittab: failed to add 'gm' module");
  }

  Py_Initialize();
  SDL_Log("python::initialize - %s", Py_GetVersion());
}

void shutdown()
{
  Py_Finalize();
}

} // namespace python
