#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

pk_VM* pk_current_vm;
static pk_VM pk_default_vm;

void py_initialize(){
    Pools_initialize();
    pk_StrName__initialize();
    pk_current_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

void py_finalize(){
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    pk_StrName__finalize();
    Pools_finalize();
}

int py_exec(const char* source){
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
