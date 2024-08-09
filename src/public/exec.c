#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

typedef struct {
    const char* source;
    const char* filename;
    int mode;
    int is_dynamic;
} py_ExecKey;

static int py_ExecKey__cmp(const py_ExecKey* a, const py_ExecKey* b) {
    return memcmp(a, b, sizeof(py_ExecKey));
}

static void py_ExecKey__ctor(py_ExecKey* key, const char* source, const char* filename,
                             enum py_CompileMode mode, bool is_dynamic) {
    key->source = source;
    key->filename = filename;
    key->mode = mode;
    key->is_dynamic = is_dynamic;
}

static bool _py_exec(const char* source,
                     const char* filename,
                     enum py_CompileMode mode,
                     py_Ref module,
                     bool is_dynamic) {
    VM* vm = pk_current_vm;
    // py_ExecKey cache_key;
    // py_ExecKey__ctor(&cache_key, source, filename, mode, is_dynamic);
    CodeObject co;
    SourceData_ src = SourceData__rcnew(source, filename, mode, is_dynamic);
    Error* err = pk_compile(src, &co);
    if(err) {
        py_exception(tp_SyntaxError, err->msg);
        py_BaseException__stpush(&vm->curr_exception, src, err->lineno, NULL);

        PK_DECREF(src);
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