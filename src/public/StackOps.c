#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

PK_INLINE py_Ref py_peek(int i) {
    assert(i <= 0);
    return pk_current_vm->stack.sp + i;
}

PK_INLINE void py_push(py_Ref src) {
    VM* vm = pk_current_vm;
    *vm->stack.sp++ = *src;
}

PK_INLINE void py_pushnil() {
    VM* vm = pk_current_vm;
    py_newnil(vm->stack.sp++);
}

PK_INLINE void py_pushnone() {
    VM* vm = pk_current_vm;
    py_newnone(vm->stack.sp++);
}

PK_INLINE void py_pushname(py_Name name) {
    VM* vm = pk_current_vm;
    py_newint(vm->stack.sp++, (uintptr_t)name);
}

PK_INLINE void py_pop() {
    VM* vm = pk_current_vm;
    vm->stack.sp--;
}

PK_INLINE void py_shrink(int n) {
    VM* vm = pk_current_vm;
    vm->stack.sp -= n;
}

PK_INLINE py_Ref py_pushtmp() {
    VM* vm = pk_current_vm;
    return vm->stack.sp++;
}

PK_INLINE bool py_pushmethod(py_Name name) {
    bool ok = pk_loadmethod(py_peek(-1), name);
    if(ok) pk_current_vm->stack.sp++;
    return ok;
}

bool py_pusheval(const char* expr, py_GlobalRef module) {
    bool ok = py_exec(expr, "<string>", EVAL_MODE, module);
    if(!ok) return false;
    py_push(py_retval());
    return true;
}

PK_INLINE bool py_vectorcall(uint16_t argc, uint16_t kwargc) {
    return VM__vectorcall(pk_current_vm, argc, kwargc, false) != RES_ERROR;
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

bool py_tpcall(py_Type type, int argc, py_Ref argv) {
    return py_call(py_tpobject(type), argc, argv);
}

bool py_binaryop(py_Ref lhs, py_Ref rhs, py_Name op, py_Name rop) {
    py_push(lhs);
    py_push(rhs);
    bool ok = pk_stack_binaryop(pk_current_vm, op, rop);
    py_shrink(2);
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
            default: return false;
        }
        return true;
    }
    return false;
}

bool pk_callmagic(py_Name name, int argc, py_Ref argv) {
    assert(argc >= 1);
    // assert(py_ismagicname(name));
    py_Ref tmp = py_tpfindmagic(argv->type, name);
    if(!tmp) return AttributeError(argv, name);
    return py_call(tmp, argc, argv);
}
