#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/name.h"
#include "pocketpy/interpreter/vm.h"

_Thread_local VM* pk_current_vm;

static bool pk_initialized;
static bool pk_finalized;

static VM pk_default_vm;
static VM* pk_all_vm[16];
static py_TValue _True, _False, _None, _NIL;

void py_initialize() {
    c11__rtassert(!pk_finalized);

    if(pk_initialized) {
        // c11__abort("py_initialize() can only be called once!");
        return;
    }

    pk_names_initialize();

    // check endianness
    int x = 1;
    bool is_little_endian = *(char*)&x == 1;
    if(!is_little_endian) c11__abort("is_little_endian != true");

    static_assert(sizeof(py_TValue) == 24, "sizeof(py_TValue) != 24");
    static_assert(offsetof(py_TValue, extra) == 4, "offsetof(py_TValue, extra) != 4");

    pk_current_vm = pk_all_vm[0] = &pk_default_vm;

    // initialize some convenient references
    py_newbool(&_True, true);
    py_newbool(&_False, false);
    py_newnone(&_None);
    py_newnil(&_NIL);
    VM__ctor(&pk_default_vm);

    pk_initialized = true;
}

void* py_malloc(size_t size) { return PK_MALLOC(size); }

void* py_realloc(void* ptr, size_t size) { return PK_REALLOC(ptr, size); }

void py_free(void* ptr) { PK_FREE(ptr); }

py_GlobalRef py_True() { return &_True; }

py_GlobalRef py_False() { return &_False; }

py_GlobalRef py_None() { return &_None; }

py_GlobalRef py_NIL() { return &_NIL; }

void py_finalize() {
    if(pk_finalized) c11__abort("py_finalize() can only be called once!");
    pk_finalized = true;

    for(int i = 1; i < 16; i++) {
        VM* vm = pk_all_vm[i];
        if(vm) {
            // temp fix https://github.com/pocketpy/pocketpy/issues/315
            // TODO: refactor VM__ctor and VM__dtor
            pk_current_vm = vm;
            VM__dtor(vm);
            PK_FREE(vm);
        }
    }
    pk_current_vm = &pk_default_vm;
    VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;

    pk_names_finalize();
}

void py_switchvm(int index) {
    if(index < 0 || index >= 16) c11__abort("invalid vm index");
    if(!pk_all_vm[index]) {
        pk_current_vm = pk_all_vm[index] = PK_MALLOC(sizeof(VM));
        memset(pk_current_vm, 0, sizeof(VM));
        VM__ctor(pk_all_vm[index]);
    } else {
        pk_current_vm = pk_all_vm[index];
    }
}

void py_resetvm() {
    VM* vm = pk_current_vm;
    VM__dtor(vm);
    memset(vm, 0, sizeof(VM));
    VM__ctor(vm);
}

void py_resetallvm() {
    for(int i = 0; i < 16; i++) {
        py_switchvm(i);
        py_resetvm();
    }
    py_switchvm(0);
}

int py_currentvm() {
    for(int i = 0; i < 16; i++) {
        if(pk_all_vm[i] == pk_current_vm) return i;
    }
    return -1;
}

void* py_getvmctx() { return pk_current_vm->ctx; }

void py_setvmctx(void* ctx) { pk_current_vm->ctx = ctx; }

void py_sys_setargv(int argc, char** argv) {
    py_GlobalRef sys = py_getmodule("sys");
    py_Ref argv_list = py_getdict(sys, py_name("argv"));
    py_list_clear(argv_list);
    for(int i = 0; i < argc; i++) {
        py_newstr(py_list_emplace(argv_list), argv[i]);
    }
}

py_Callbacks* py_callbacks() { return &pk_current_vm->callbacks; }

const char* pk_opname(Opcode op) {
    const static char* OP_NAMES[] = {
#define OPCODE(name) #name,
#include "pocketpy/xmacros/opcodes.h"
#undef OPCODE
    };
    return OP_NAMES[op];
}

bool py_call(py_Ref f, int argc, py_Ref argv) {
    if(f->type == tp_nativefunc) {
        return py_callcfunc(f->_cfunc, argc, argv);
    } else {
        py_push(f);
        py_pushnil();
        for(int i = 0; i < argc; i++)
            py_push(py_offset(argv, i));
        bool ok = py_vectorcall(argc, 0);
        return ok;
    }
}

#ifndef NDEBUG
bool py_callcfunc(py_CFunction f, int argc, py_Ref argv) {
    if(py_checkexc()) {
        const char* name = py_tpname(pk_current_vm->unhandled_exc.type);
        c11__abort("unhandled exception `%s` was set!", name);
    }
    py_StackRef p0 = py_peek(0);
    // NOTE: sometimes users are using `py_retval()` to pass `argv`
    // It will be reset to `nil` and cause an exception
    py_newnil(py_retval());
    bool ok = f(argc, argv);
    if(!ok) {
        if(!py_checkexc()) { c11__abort("py_CFunction returns `false` but no exception is set!"); }
        return false;
    }
    if(py_peek(0) != p0) {
        c11__abort("py_CFunction corrupts the stack! Did you forget to call `py_pop()`?");
    }
    if(py_isnil(py_retval())) {
        c11__abort(
            "py_CFunction returns nothing! Did you forget to call `py_newnone(py_retval())`?");
    }
    if(py_checkexc()) {
        const char* name = py_tpname(pk_current_vm->unhandled_exc.type);
        c11__abort("py_CFunction returns `true`, but `%s` was set!", name);
    }
    return true;
}
#endif

bool py_vectorcall(uint16_t argc, uint16_t kwargc) {
    return VM__vectorcall(pk_current_vm, argc, kwargc, false) != RES_ERROR;
}

PK_INLINE py_Ref py_retval() { return &pk_current_vm->last_retval; }

bool py_pushmethod(py_Name name) {
    bool ok = pk_loadmethod(py_peek(-1), name);
    if(ok) pk_current_vm->stack.sp++;
    return ok;
}

bool pk_loadmethod(py_StackRef self, py_Name name) {
    // NOTE: `out` and `out_self` may overlap with `self`
    py_Type type;

    if(name == __new__) {
        // __new__ acts like a @staticmethod
        if(self->type == tp_type) {
            // T.__new__(...)
            type = py_totype(self);
        } else if(self->type == tp_super) {
            // super(T, obj).__new__(...)
            type = *(py_Type*)py_touserdata(self);
        } else {
            // invalid usage of `__new__`
            return false;
        }
        py_Ref cls_var = py_tpfindmagic(type, name);
        if(cls_var) {
            self[0] = *cls_var;
            self[1] = *py_NIL();
            return true;
        }
        return false;
    }

    py_TValue self_bak;  // to avoid overlapping
    // handle super() proxy
    if(py_istype(self, tp_super)) {
        type = *(py_Type*)py_touserdata(self);
        // BUG: here we modify `self` which refers to the stack directly
        // If `pk_loadmethod` fails, `self` will be corrupted
        self_bak = *py_getslot(self, 0);
    } else {
        type = self->type;
        self_bak = *self;
    }

    py_TypeInfo* ti = pk_typeinfo(type);

    if(ti->getunboundmethod) {
        bool ok = ti->getunboundmethod(self, name);
        if(ok) {
            assert(py_retval()->type == tp_nativefunc || py_retval()->type == tp_function);
            self[0] = *py_retval();
            self[1] = self_bak;
            return true;
        } else {
            return false;
        }
    }

    py_Ref cls_var = pk_tpfindname(ti, name);
    if(cls_var != NULL) {
        switch(cls_var->type) {
            case tp_function:
            case tp_nativefunc: {
                self[0] = *cls_var;
                self[1] = self_bak;
                break;
            }
            case tp_staticmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = *py_NIL();
                break;
            case tp_classmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = ti->self;
                break;
            default: c11__unreachable();
        }
        return true;
    }
    return false;
}

bool py_tpcall(py_Type type, int argc, py_Ref argv) {
    return py_call(py_tpobject(type), argc, argv);
}

bool pk_callmagic(py_Name name, int argc, py_Ref argv) {
    assert(argc >= 1);
    // assert(py_ismagicname(name));
    py_Ref tmp = py_tpfindmagic(argv->type, name);
    if(!tmp) return AttributeError(argv, name);
    return py_call(tmp, argc, argv);
}

bool StopIteration() {
    bool ok = py_tpcall(tp_StopIteration, 0, NULL);
    if(!ok) return false;
    return py_raise(py_retval());
}
