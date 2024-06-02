#include "pocketpy/objects/builtins.hpp"

namespace pkpy{
    PyVar const PY_OP_CALL(Type(), new PyObject(Type()));
    PyVar const PY_OP_YIELD(Type(), new PyObject(Type()));
}   // namespace pkpy