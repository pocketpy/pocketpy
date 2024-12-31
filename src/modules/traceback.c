#include "pocketpy/pocketpy.h"
#include <stdlib.h>

static bool traceback_format_exc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    char* s = py_formatexc();
    if(!s) {
        py_newnone(py_retval());
    } else {
        py_newstr(py_retval(), s);
        PK_FREE(s);
    }
    return true;
}

static bool traceback_print_exc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_printexc();
    py_newnone(py_retval());
    return true;
}

void pk__add_module_traceback() {
    py_Ref mod = py_newmodule("traceback");

    py_bindfunc(mod, "format_exc", traceback_format_exc);
    py_bindfunc(mod, "print_exc", traceback_print_exc);
}