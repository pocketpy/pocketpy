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
    pk_Compiler__initialize();
    pk_current_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

void py_finalize() {
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    pk_Compiler__finalize();
    pk_StrName__finalize();
    pk_MemoryPools__finalize();
}

int py_exec(const char* source) {
    pk_SourceData_ src = pk_SourceData__rcnew(source, "main.py", EXEC_MODE, false);
    CodeObject co;
    Error* err = pk_compile(src, &co);
    PK_DECREF(src);
    if(err) abort();

    pk_VM* vm = pk_current_vm;
    Frame* frame = Frame__new(&co, &vm->main, NULL, vm->stack.sp, vm->stack.sp, &co);
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    if(res == RES_ERROR) return vm->last_error->type;
    if(res == RES_RETURN) return 0;
    PK_UNREACHABLE();
}

int py_eval(const char* source) {
    CodeObject* co = NULL;
    pk_VM* vm = pk_current_vm;
    Frame* frame = Frame__new(co, &vm->main, NULL, vm->stack.sp, vm->stack.sp, co);
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    if(res == RES_ERROR) return vm->last_error->type;
    if(res == RES_RETURN) return 0;
    PK_UNREACHABLE();
}