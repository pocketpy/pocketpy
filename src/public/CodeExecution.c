#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"
#include <assert.h>
#include <ctype.h>

py_Type pk_code__register() {
    py_Type type = pk_newtype("code", tp_object, NULL, (py_Dtor)CodeObject__dtor, false, true);
    return type;
}

static bool _py_compile(CodeObject* out,
                 const char* source,
                 const char* filename,
                 enum py_CompileMode mode,
                 bool is_dynamic) {
    VM* vm = pk_current_vm;
    SourceData_ src = SourceData__rcnew(source, filename, mode, is_dynamic);
    Error* err = pk_compile(src, out);
    if(err) {
        py_exception(tp_SyntaxError, err->msg);
        py_BaseException__stpush(NULL, &vm->unhandled_exc, err->src, err->lineno, NULL);
        PK_DECREF(src);

        PK_DECREF(err->src);
        PK_FREE(err);
        return false;
    }
    PK_DECREF(src);
    return true;
}

bool pk_exec(CodeObject* co, py_Ref module) {
    VM* vm = pk_current_vm;
    if(!module) module = vm->main;
    assert(module->type == tp_module);

    py_StackRef sp = vm->stack.sp;
    py_Frame* frame = Frame__new(co, sp, module, module, py_NIL(), true);
    VM__push_frame(vm, frame);
    FrameResult res = VM__run_top_frame(vm);
    if(res == RES_ERROR) return false;
    assert(res == RES_RETURN);
    return true;
}

bool pk_execdyn(CodeObject* co, py_Ref module, py_Ref globals, py_Ref locals) {
    VM* vm = pk_current_vm;
    if(!module) module = vm->main;
    assert(module->type == tp_module);

    py_StackRef sp = vm->stack.sp;
    assert(globals != NULL && locals != NULL);

    // check globals
    if(globals->type == tp_namedict) {
        globals = py_getslot(globals, 0);
        assert(globals->type == tp_module);
    } else {
        if(!py_istype(globals, tp_dict)) { return TypeError("globals must be a dict object"); }
    }
    // check locals
    switch(locals->type) {
        case tp_locals: break;
        case tp_dict: break;
        case tp_nil: break;
        default: return TypeError("locals must be a dict object");
    }

    py_Frame* frame = Frame__new(co, sp, module, globals, locals, true);
    VM__push_frame(vm, frame);
    FrameResult res = VM__run_top_frame(vm);
    if(res == RES_ERROR) return false;
    assert(res == RES_RETURN);
    return true;
}

static bool
    pk_smartexec(const char* source, py_Ref module, enum py_CompileMode mode, va_list args) {
    if(module == NULL) module = pk_current_vm->main;
    pk_mappingproxy__namedict(py_pushtmp(), module);  // globals
    py_newdict(py_pushtmp());                         // locals
    bool ok = py_compile(source, "<string>", mode, true);
    if(!ok) return false;
    py_push(py_retval());
    // [globals, locals, code]
    CodeObject* co = py_touserdata(py_peek(-1));
    py_StackRef locals = py_peek(-2);
    int max_index = -1;
    c11__foreach(Bytecode, &co->codes, bc) {
        if(bc->op == OP_LOAD_NAME) {
            c11_sv name = py_name2sv(c11__getitem(py_Name, &co->names, bc->arg));
            if(name.data[0] != '_') continue;
            int index;
            if(name.size == 1) {
                index = 0;
            } else if(name.size == 2 && isdigit(name.data[1])) {
                index = name.data[1] - '0';
            } else {
                continue;
            }
            max_index = c11__max(max_index, index);
        }
    }

    if(max_index == -1) return ValueError("no placeholder found in the source");

    for(int i = 0; i <= max_index; i++) {
        py_Ref val = va_arg(args, py_Ref);
        char buf[3];
        buf[0] = '_';
        buf[1] = '0' + i;
        buf[2] = '\0';
        py_dict_setitem_by_str(locals, buf, val);
        if(i == 0) {
            // _ => _0
            py_dict_setitem_by_str(locals, "_", val);
        }
    }
    ok = pk_execdyn(co, module, py_peek(-3), locals);
    if(!ok) return false;
    py_shrink(3);
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

bool py_smartexec(const char* source, py_Ref module, ...) {
    va_list args;
    va_start(args, module);
    bool ok = pk_smartexec(source, module, EXEC_MODE, args);
    va_end(args);
    return ok;
}

bool py_smarteval(const char* source, py_Ref module, ...) {
    va_list args;
    va_start(args, module);
    bool ok = pk_smartexec(source, module, EVAL_MODE, args);
    va_end(args);
    return ok;
}
