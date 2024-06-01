#include "pocketpy/obj.h"

namespace pkpy{
    PyVar::PyVar(PyObject* p): PyVar(p->type, p) {}
}   // namespace pkpy