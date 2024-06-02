#include "pocketpy/objects/object.hpp"

namespace pkpy{
    PyVar::PyVar(PyObject* p): PyVar(p->type, p) {}
}   // namespace pkpy