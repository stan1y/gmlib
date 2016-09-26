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

  SDL_Log("python::initialize - done %s", Py_GetVersion());
}

void shutdown()
{
  Py_Finalize();
}

};