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
        if(out) *out = *--vm->stack.sp;
        return 0;
    }
    PK_UNREACHABLE();
}