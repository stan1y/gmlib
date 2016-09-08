#ifndef _GM_PYTHON_H_
#define _GM_PYTHON_H_

#include "GMLib.h"

namespace python {

/* initialize embedded python environment */
void initialize(const config * cfg);
void shutdown();

};

#endif //_GM_PYTHON_H_