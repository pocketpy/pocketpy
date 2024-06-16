#include "pocketpy/objects/builtins.hpp"

namespace pkpy {
PyVar PY_OP_CALL(Type(), new PyObject(Type(), true));
PyVar PY_OP_YIELD(Type(), new PyObject(Type(), true));
}  // namespace pkpy
