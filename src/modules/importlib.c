#include "pocketpy/pocketpy.h"

static bool importlib_reload(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_module);
    bool ok = py_importlib_reload(argv);
    py_newnone(py_retval());
    return ok;
}

void pk__add_module_importlib() {
    py_Ref mod = py_newmodule("importlib");

    py_bindfunc(mod, "reload", importlib_reload);
}