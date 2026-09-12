#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/interpreter/vm.h"

static char* traceback_formatexc() {
    // A nested try body or a helper call must not hide the active handler.
    for(py_Frame* frame = pk_current_vm->top_frame; frame; frame = frame->f_back) {
        for(int i = frame->exc_stack.length - 1; i >= 0; i--) {
            FrameExcInfo* info = c11__at(FrameExcInfo, &frame->exc_stack, i);
            if(!py_isnil(&info->exc)) return formatexc_internal(&info->exc);
        }
    }
    return NULL;
}

static bool traceback_format_exc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    char* res = traceback_formatexc();
    if(res) {
        py_newstr(py_retval(), res);
        PK_FREE(res);
    } else {
        py_newnone(py_retval());
    }
    return true;
}

static bool traceback_print_exc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    char* res = traceback_formatexc();
    if(res) {
        pk_current_vm->callbacks.print(res);
        pk_current_vm->callbacks.print("\n");
        PK_FREE(res);
    }
    py_newnone(py_retval());
    return true;
}

void pk__add_module_traceback() {
    py_Ref mod = py_newmodule("traceback");

    py_bindfunc(mod, "format_exc", traceback_format_exc);
    py_bindfunc(mod, "print_exc", traceback_print_exc);
}
