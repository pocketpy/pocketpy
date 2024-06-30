#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

pk_VM* pk_current_vm;
static pk_VM pk_default_vm;

void py_initialize() {
    pk_MemoryPools__initialize();
    pk_StrName__initialize();
    pk_current_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

void py_finalize() {
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    pk_StrName__finalize();
    pk_MemoryPools__finalize();
}

int py_exec(const char* source) { PK_UNREACHABLE(); }

int py_eval(const char* source, py_Ref out) {
    CodeObject co;
    pk_SourceData_ src = pk_SourceData__rcnew(source, "main.py", EVAL_MODE, false);
    Error* err = pk_compile(src, &co);
    if(err) {
        PK_DECREF(src);
        return -1;
    }
    pk_VM* vm = pk_current_vm;
    Frame* frame = Frame__new(&co, &vm->main, NULL, vm->stack.sp, vm->stack.sp, &co);
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    CodeObject__dtor(&co);
    PK_DECREF(src);
    if(res == RES_ERROR) return vm->last_error->type;
    if(res == RES_RETURN){
        *out = vm->last_retval;
        return 0;
    }
    PK_UNREACHABLE();
}

bool py_call(py_Ref f, int argc, py_Ref argv){
    return -1;
}

bool py_callmethod(py_Ref self, py_Name name, int argc, py_Ref argv){
    return -1;
}

int pk_vectorcall(int argc, int kwargc, bool op_call){
    return -1;
}

py_Ref py_lastretval(){
    return &pk_current_vm->last_retval;
}

bool py_getunboundmethod(const py_Ref self, py_Name name, bool fallback, py_Ref out, py_Ref out_self){
    return -1;
}

pk_TypeInfo* pk_tpinfo(const py_Ref self){
    pk_VM* vm = pk_current_vm;
    return c11__at(pk_TypeInfo, &vm->types, self->type);
}

