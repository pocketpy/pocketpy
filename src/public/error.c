#include "pocketpy/pocketpy.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

#include <stdarg.h>

void py_printexc() {
    pk_VM* vm = pk_current_vm;
    if(py_isnil(&vm->last_exception)) {
        vm->_stdout("NoneType: None\n");
    } else {
        const char* name = py_tpname(vm->last_exception.type);
        bool ok = py_str(&vm->last_exception);
        if(!ok) abort();
        const char* message = py_tostr(py_retval());
        vm->_stdout("%s: %s\n", name, message);
    }
}

void py_formatexc(char* out) {}

bool py_exception(const char* name, const char* fmt, ...) {
    pk_VM* vm = pk_current_vm;
    // an error is already set
    assert(py_isnil(&vm->last_exception));

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);

    c11_string* res = c11_sbuf__submit(&buf);
    py_Ref message = py_pushtmp();
    py_newstrn(message, res->data, res->size);
    c11_string__delete(res);
    bool ok = py_tpcall(tp_exception, 1, message);
    py_pop();

    if(!ok) abort();
    vm->last_exception = *py_retval();

    return false;
}