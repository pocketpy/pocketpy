#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

static bool _py_exec(const char* source,
                     const char* filename,
                     enum py_CompileMode mode,
                     py_Ref module,
                     bool is_dynamic) {
    VM* vm = pk_current_vm;
    CodeObject co;
    SourceData_ src = SourceData__rcnew(source, filename, mode, is_dynamic);
    Error* err = pk_compile(src, &co);
    if(err) {
        py_exception(tp_SyntaxError, err->msg);
        py_BaseException__stpush(&vm->curr_exception, err->src, err->lineno, NULL);
        PK_DECREF(src);
        
        PK_DECREF(err->src);
        free(err);
        return false;
    }

    if(!module) module = &vm->main;

    py_StackRef sp = vm->stack.sp;
    if(is_dynamic) {
        // [globals, locals]
        sp -= 2;
    }

    Frame* frame = Frame__new(&co, module, sp, sp, false, is_dynamic);
    VM__push_frame(vm, frame);
    FrameResult res = VM__run_top_frame(vm);
    CodeObject__dtor(&co);
    PK_DECREF(src);
    if(res == RES_ERROR) return false;
    if(res == RES_RETURN) return true;
    c11__unreachedable();
}

bool py_exec(const char* source, const char* filename, enum py_CompileMode mode, py_Ref module) {
    return _py_exec(source, filename, mode, module, false);
}

bool py_execdyn(const char* source, const char* filename, enum py_CompileMode mode, py_Ref module) {
    return _py_exec(source, filename, mode, module, true);
}