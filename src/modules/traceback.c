#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/interpreter/vm.h"

static bool traceback_format_exc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    VM* vm = pk_current_vm;
    if(vm->top_frame) {
        FrameExcInfo* info = Frame__top_exc_info(vm->top_frame);
        if(info && !py_isnil(&info->exc)) {
            char* res = formatexc_internal(&info->exc);
            py_newstr(py_retval(), res);
            PK_FREE(res);
            return true;
        }
    }
    py_newnone(py_retval());
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