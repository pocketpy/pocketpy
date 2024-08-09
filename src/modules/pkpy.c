#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

static bool pkpy_next(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int res = py_next(argv);
    if(res == -1) return false;
    if(res) return true;
    py_assign(py_retval(), py_tpobject(tp_StopIteration));
    return true;
}

void pk__add_module_pkpy() {
    py_Ref mod = py_newmodule("pkpy");

    py_bindfunc(mod, "next", pkpy_next);
}