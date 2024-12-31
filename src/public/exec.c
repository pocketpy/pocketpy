#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"

static void code__gc_mark(void* ud) { CodeObject__gc_mark(ud); }

py_Type pk_code__register() {
    py_Type type = pk_newtype("code", tp_object, NULL, (py_Dtor)CodeObject__dtor, false, true);
    pk__tp_set_marker(type, code__gc_mark);
    return type;
}

bool _py_compile(CodeObject* out,
                 const char* source,
                 const char* filename,
                 enum py_CompileMode mode,
                 bool is_dynamic) {
    VM* vm = pk_current_vm;
    SourceData_ src = SourceData__rcnew(source, filename, mode, is_dynamic);
    Error* err = pk_compile(src, out);
    if(err) {
        py_exception(tp_SyntaxError, err->msg);
        py_BaseException__stpush(&vm->curr_exception, err->src, err->lineno, NULL);
        PK_DECREF(src);

        PK_DECREF(err->src);
        PK_FREE(err);
        return false;
    }
    PK_DECREF(src);
    return true;
}

bool py_compile(const char* source,
                const char* filename,
                enum py_CompileMode mode,
                bool is_dynamic) {
    CodeObject co;
    bool ok = _py_compile(&co, source, filename, mode, is_dynamic);
    if(ok) {
        // compile success
        CodeObject* ud = py_newobject(py_retval(), tp_code, 0, sizeof(CodeObject));
        *ud = co;
    }
    return ok;
}

bool pk_exec(CodeObject* co, py_Ref module) {
    VM* vm = pk_current_vm;
    if(!module) module = &vm->main;
    assert(module->type == tp_module);

    py_StackRef sp = vm->stack.sp;
    if(co->src->is_dynamic) sp -= 3;  // [globals, locals, code]

    Frame* frame = Frame__new(co, module, sp, sp, false);
    VM__push_frame(vm, frame);
    FrameResult res = VM__run_top_frame(vm);
    if(res == RES_ERROR) return false;
    if(res == RES_RETURN) return true;
    c11__unreachable();
}

bool py_exec(const char* source, const char* filename, enum py_CompileMode mode, py_Ref module) {
    CodeObject co;
    if(!_py_compile(&co, source, filename, mode, false)) return false;
    bool ok = pk_exec(&co, module);
    CodeObject__dtor(&co);
    return ok;
}

bool py_eval(const char* source, py_Ref module) {
    return py_exec(source, "<string>", EVAL_MODE, module);
}