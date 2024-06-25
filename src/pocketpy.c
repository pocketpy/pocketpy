#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include <assert.h>
#include <stdlib.h>

pk_VM* pk_current_vm;
static pk_VM pk_default_vm;

void py_initialize(){
    Pools_initialize();
    pk_StrName__initialize();
    pk_current_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

int py_exec_simple(const char* source){
    CodeObject* co = NULL;
    pk_VM* vm = pk_current_vm;
    Frame* frame = Frame__new(
        co,
        &vm->main,
        NULL,
        vm->stack.sp,
        vm->stack.sp,
        co
    );
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    if(res == RES_ERROR) return vm->last_error->type;
    if(res == RES_RETURN) return 0; // vm->last_retval;
    assert(0);  // unreachable
}

py_Error* py_getlasterror(){
    return pk_current_vm->last_error;
}

void py_Error__print(py_Error* self){
    abort();
}

void py_finalize(){
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    pk_StrName__finalize();
    Pools_finalize();
}

void py_setdict(py_Ref self, py_Name name, const py_Ref val){
    pk_NameDict__set(
        PyObject__dict(self->_obj),
        name,
        *val
    );
}

void py_newint(py_Ref self, int64_t val){
    self->type = tp_int;
    self->is_ptr = false;
    self->_i64 = val;
}

