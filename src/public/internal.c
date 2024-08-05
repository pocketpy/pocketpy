#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"
#include <stdint.h>

VM* pk_current_vm;

py_GlobalRef py_True;
py_GlobalRef py_False;
py_GlobalRef py_None;
py_GlobalRef py_NIL;

static VM pk_default_vm;
static VM* pk_all_vm[16];

void py_initialize() {
    MemoryPools__initialize();
    py_Name__initialize();

    pk_current_vm = pk_all_vm[0] = &pk_default_vm;

    // initialize some convenient references
    static py_TValue _True, _False, _None, _NIL;
    py_newbool(&_True, true);
    py_newbool(&_False, false);
    py_newnone(&_None);
    py_newnil(&_NIL);
    py_True = &_True;
    py_False = &_False;
    py_None = &_None;
    py_NIL = &_NIL;
    VM__ctor(&pk_default_vm);
}

void py_finalize() {
    for(int i = 1; i < 16; i++) {
        VM* vm = pk_all_vm[i];
        if(vm) {
            VM__dtor(vm);
            free(vm);
        }
    }
    VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    py_Name__finalize();
    MemoryPools__finalize();
}

void py_switchvm(int index) {
    if(index < 0 || index >= 16) c11__abort("invalid vm index");
    if(!pk_all_vm[index]) {
        pk_all_vm[index] = malloc(sizeof(VM));
        VM__ctor(pk_all_vm[index]);
    }
    pk_current_vm = pk_all_vm[index];
}

int py_currentvm() {
    for(int i = 0; i < 16; i++) {
        if(pk_all_vm[i] == pk_current_vm) return i;
    }
    return -1;
}

const char* pk_opname(Opcode op) {
    const static char* OP_NAMES[] = {
#define OPCODE(name) #name,
#include "pocketpy/xmacros/opcodes.h"
#undef OPCODE
    };
    return OP_NAMES[op];
}

bool py_exec(const char* source, const char* filename, enum py_CompileMode mode, py_Ref module) {
    VM* vm = pk_current_vm;
    CodeObject co;
    SourceData_ src = SourceData__rcnew(source, filename, mode, false);
    Error* err = pk_compile(src, &co);
    if(err) {
        py_exception(tp_SyntaxError, err->msg);
        py_BaseException__stpush(&vm->curr_exception, src, err->lineno, NULL);

        PK_DECREF(src);
        free(err);
        return false;
    }

    if(!module) module = &vm->main;

    Frame* frame = Frame__new(&co, module, NULL, vm->stack.sp, vm->stack.sp);
    VM__push_frame(vm, frame);
    FrameResult res = VM__run_top_frame(vm);
    CodeObject__dtor(&co);
    PK_DECREF(src);
    if(res == RES_ERROR) return false;
    if(res == RES_RETURN) return true;
    c11__unreachedable();
}

bool py_call(py_Ref f, int argc, py_Ref argv) {
    if(f->type == tp_nativefunc) {
        return f->_cfunc(argc, argv);
    } else {
        py_push(f);
        py_pushnil();
        for(int i = 0; i < argc; i++)
            py_push(py_offset(argv, i));
        return py_vectorcall(argc, 0);
    }
}

bool py_vectorcall(uint16_t argc, uint16_t kwargc) {
    VM* vm = pk_current_vm;
    return VM__vectorcall(vm, argc, kwargc, false) != RES_ERROR;
}

py_Ref py_retval() { return &pk_current_vm->last_retval; }

bool py_pushmethod(py_Name name) {
    bool ok = pk_loadmethod(py_peek(-1), name);
    if(ok) pk_current_vm->stack.sp++;
    return ok;
}

bool pk_loadmethod(py_StackRef self, py_Name name) {
    // NOTE: `out` and `out_self` may overlap with `self`
    py_Type type;
    // handle super() proxy
    if(py_istype(self, tp_super)) {
        type = *(py_Type*)py_touserdata(self);
        *self = *py_getslot(self, 0);
    } else {
        type = self->type;
    }

    py_Ref cls_var = py_tpfindname(type, name);
    if(cls_var != NULL) {
        switch(cls_var->type) {
            case tp_function:
            case tp_nativefunc: {
                py_TValue self_bak = *self;
                // `out` may overlap with `self`. If we assign `out`, `self` may be corrupted.
                self[0] = *cls_var;
                self[1] = self_bak;
                break;
            }
            case tp_staticmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = *py_NIL;
                break;
            case tp_classmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = c11__getitem(py_TypeInfo, &pk_current_vm->types, type).self;
                break;
            default: c11__unreachedable();
        }
        return true;
    }
    return false;
}

py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    assert(py_ismagicname(name));
    py_TypeInfo* types = (py_TypeInfo*)pk_current_vm->types.data;
    do {
        py_Ref f = &types[t].magic[name];
        if(!py_isnil(f)) return f;
        t = types[t].base;
    } while(t);
    return NULL;
}

py_Ref py_tpfindname(py_Type t, py_Name name) {
    py_TypeInfo* types = (py_TypeInfo*)pk_current_vm->types.data;
    do {
        py_Ref res = py_getdict(&types[t].self, name);
        if(res) return res;
        t = types[t].base;
    } while(t);
    return NULL;
}

py_Ref py_tpmagic(py_Type type, py_Name name) {
    assert(py_ismagicname(name));
    VM* vm = pk_current_vm;
    return &c11__at(py_TypeInfo, &vm->types, type)->magic[name];
}

py_Ref py_tpobject(py_Type type) {
    assert(type);
    VM* vm = pk_current_vm;
    return &c11__at(py_TypeInfo, &vm->types, type)->self;
}

const char* py_tpname(py_Type type) {
    if(!type) return "nil";
    VM* vm = pk_current_vm;
    py_Name name = c11__at(py_TypeInfo, &vm->types, type)->name;
    return py_name2str(name);
}

bool py_tpcall(py_Type type, int argc, py_Ref argv) {
    return py_call(py_tpobject(type), argc, argv);
}

bool pk_callmagic(py_Name name, int argc, py_Ref argv) {
    assert(argc >= 1);
    assert(py_ismagicname(name));
    py_Ref tmp = py_tpfindmagic(argv->type, name);
    if(!tmp) return AttributeError(argv, name);
    return py_call(tmp, argc, argv);
}

bool StopIteration() {
    VM* vm = pk_current_vm;
    assert(!vm->is_stopiteration);  // flag is already set
    vm->is_stopiteration = true;
    return false;
}