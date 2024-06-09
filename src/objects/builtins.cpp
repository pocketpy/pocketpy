#include "pocketpy/objects/builtins.hpp"

namespace pkpy {
const PyVar PY_OP_CALL(Type(), new PyObject(Type(), true));
const PyVar PY_OP_YIELD(Type(), new PyObject(Type(), true));
}  // namespace pkpy
