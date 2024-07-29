#include "pocketpy/common/utils.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

#include <stdarg.h>

bool py_checkexc() {
    pk_VM* vm = pk_current_vm;
    return !py_isnil(&vm->curr_exception);
}

void py_printexc() {
    pk_VM* vm = pk_current_vm;
    if(py_isnil(&vm->curr_exception)) {
        vm->_stdout("NoneType: None\n");
    } else {
        const char* name = py_tpname(vm->curr_exception.type);
        bool ok = py_str(&vm->curr_exception);
        if(!ok) c11__abort("py_printexc(): failed to convert exception to string");
        const char* message = py_tostr(py_retval());
        vm->_stdout("%s: %s\n", name, message);
    }
}

char* py_formatexc() {
    pk_VM* vm = pk_current_vm;
    if(py_isnil(&vm->curr_exception)) {
        return NULL;
    }
    assert(false);
}

bool py_exception(const char* name, const char* fmt, ...) {
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
    
    py_Ref exc_type = py_getdict(&pk_current_vm->builtins, py_name(name));
    if(exc_type == NULL) c11__abort("py_exception(): '%s' not found", name);
    bool ok = py_call(exc_type, 1, message);
    if(!ok) c11__abort("py_exception(): failed to create exception object");
    py_pop();

    return py_raise(py_retval());
}

bool py_raise(py_Ref exc) {
    assert(py_isinstance(exc, tp_BaseException));
    pk_VM* vm = pk_current_vm;
    vm->curr_exception = *exc;
    return false;
}

bool KeyError(py_Ref key){
    py_Ref cls = py_getdict(&pk_current_vm->builtins, py_name("KeyError"));
    bool ok = py_call(cls, 1, key);
    if(!ok) return false;
    return py_raise(py_retval());
}