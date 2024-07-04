#include "pocketpy/pocketpy.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

#include <stdarg.h>

void py_printexc(){
    pk_VM* vm = pk_current_vm;
    if(vm->has_error){
        assert(vm->last_retval.type == tp_exception);
    }else{
        vm->_stdout("NoneType: None\n");
    }
}


void py_formatexc(char *out){

}

bool py_exception(const char* name, const char* fmt, ...){
    pk_VM* vm = pk_current_vm;
    assert(!vm->has_error);     // an error is already set
    vm->has_error = true;

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);

    c11_string* res = c11_sbuf__submit(&buf);
    // vm->last_retval = py_newexception(name, res->data);
    vm->_stderr("%s: %s\n", name, res->data);
    c11_string__delete(res);
    return false;
}