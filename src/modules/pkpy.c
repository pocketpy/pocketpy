#include "pocketpy/pocketpy.h"

void pk__add_module_pkpy() { py_newmodule("pkpy"); }

#ifdef _WIN32

#include <conio.h>

static bool conio__kbhit(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _kbhit();
    py_newint(py_retval(), ret);
    return true;
}

static bool conio__getch(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int ret = _getch();
    py_newint(py_retval(), ret);
    return true;
}

#endif

void pk__add_module_conio() {
    py_Ref mod = py_newmodule("conio");

#ifdef _WIN32
    py_bindfunc(mod, "_kbhit", conio__kbhit);
    py_bindfunc(mod, "_getch", conio__getch);
#endif
}