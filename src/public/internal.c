#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/vm.h"

VM* pk_current_vm;

static VM pk_default_vm;
static VM* pk_all_vm[16];
static py_TValue _True, _False, _None, _NIL;

void py_initialize() {
    if(pk_current_vm) {
        // c11__abort("py_initialize() can only be called once!");
        return;
    }

    // check endianness
    int x = 1;
    bool is_little_endian = *(char*)&x == 1;
    if(!is_little_endian) c11__abort("is_little_endian != true");

    // check py_TValue; 16 bytes to make py_arg() macro work
    static_assert(sizeof(py_CFunction) <= 8, "sizeof(py_CFunction) > 8");
    static_assert(sizeof(py_TValue) == 16, "sizeof(py_TValue) != 16");
    static_assert(offsetof(py_TValue, extra) == 4, "offsetof(py_TValue, extra) != 4");

    py_Name__initialize();

    pk_current_vm = pk_all_vm[0] = &pk_default_vm;

    // initialize some convenient references
    py_newbool(&_True, true);
    py_newbool(&_False, false);
    py_newnone(&_None);
    py_newnil(&_NIL);
    VM__ctor(&pk_default_vm);
}

py_GlobalRef py_True() { return &_True; }

py_GlobalRef py_False() { return &_False; }

py_GlobalRef py_None() { return &_None; }

py_GlobalRef py_NIL() { return &_NIL; }

void py_finalize() {
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
    py_Name__finalize();
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

int py_currentvm() {
    for(int i = 0; i < 16; i++) {
        if(pk_all_vm[i] == pk_current_vm) return i;
    }
    return -1;
}

void* py_getvmctx() { return pk_current_vm->ctx; }

void py_setvmctx(void* ctx) { pk_current_vm->ctx = ctx; }

void py_interrupt() { pk_current_vm->is_signal_interrupted = true; }

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
        py_StackRef p0 = py_peek(0);
        py_push(f);
        py_pushnil();
        for(int i = 0; i < argc; i++)
            py_push(py_offset(argv, i));
        bool ok = py_vectorcall(argc, 0);
        pk_current_vm->stack.sp = p0;
        return ok;
    }
}

#ifndef NDEBUG
bool py_callcfunc(py_CFunction f, int argc, py_Ref argv) {
    py_StackRef p0 = py_peek(0);
    py_newnil(py_retval());
    bool ok = f(argc, argv);
    if(!ok) {
        if(!py_checkexc(true)) {
            c11__abort("py_CFunction returns `false` but no exception is set!");
        }
        return false;
    }
    if(py_peek(0) != p0) {
        c11__abort("py_CFunction corrupts the stack! Did you forget to call `py_pop()`?");
    }
    if(py_isnil(py_retval())) {
        c11__abort(
            "py_CFunction returns nothing! Did you forget to call `py_newnone(py_retval())`?");
    }
    if(py_checkexc(true)) {
        const char* name = py_tpname(pk_current_vm->curr_exception.type);
        c11__abort("py_CFunction returns `true`, but `%s` was set!", name);
    }
    return true;
}
#endif

bool py_vectorcall(uint16_t argc, uint16_t kwargc) {
    return VM__vectorcall(pk_current_vm, argc, kwargc, false) != RES_ERROR;
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

    if(name == __new__) {
        // __new__ acts like a @staticmethod
        if(py_istype(self, tp_type)) {
            // T.__new__(...)
            type = py_totype(self);
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
                self[1] = *py_NIL();
                break;
            case tp_classmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = pk__type_info(type)->self;
                break;
            default: c11__unreachable();
        }
        return true;
    }
    return false;
}

py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    assert(py_ismagicname(name));
    py_TypeInfo* ti = pk__type_info(t);
    do {
        py_Ref f = TypeList__magic_readonly_nullable(ti, name);
        if(f != NULL) return f;
        ti = ti->base_ti;
    } while(ti);
    return NULL;
}

py_Ref py_tpfindname(py_Type t, py_Name name) {
    py_TypeInfo* ti = pk__type_info(t);
    do {
        py_Ref res = py_getdict(&ti->self, name);
        if(res) return res;
        ti = ti->base_ti;
    } while(ti);
    return NULL;
}

py_Ref py_tpgetmagic(py_Type type, py_Name name) {
    assert(py_ismagicname(name));
    py_TypeInfo* ti = pk__type_info(type);
    return TypeList__magic(ti, name);
}

py_Ref py_tpobject(py_Type type) {
    assert(type);
    return &pk__type_info(type)->self;
}

const char* py_tpname(py_Type type) {
    if(!type) return "nil";
    py_Name name = pk__type_info(type)->name;
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
    bool ok = py_tpcall(tp_StopIteration, 0, NULL);
    if(!ok) return false;
    return py_raise(py_retval());
}
