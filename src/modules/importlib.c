#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

static bool importlib_reload(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_module);
    return py_importlib_reload(argv);
}

void pk__add_module_importlib() {
    py_Ref mod = py_newmodule("importlib");

    py_bindfunc(mod, "reload", importlib_reload);
}
