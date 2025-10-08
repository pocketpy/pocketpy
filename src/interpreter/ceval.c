#include "pocketpy/common/str.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/error.h"
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#define DISPATCH()                                                                                 \
    do {                                                                                           \
        frame->ip++;                                                                               \
        goto __NEXT_STEP;                                                                          \
    } while(0)
#define DISPATCH_JUMP(__offset)                                                                    \
    do {                                                                                           \
        frame->ip += __offset;                                                                     \
        goto __NEXT_STEP;                                                                          \
    } while(0)
#define DISPATCH_JUMP_ABSOLUTE(__target)                                                           \
    do {                                                                                           \
        frame->ip = __target;                                                                      \
        goto __NEXT_STEP;                                                                          \
    } while(0)

#define RESET_CO_CACHE()                                                                           \
    do {                                                                                           \
        co_codes = frame->co->codes.data;                                                          \
        co_names = frame->co->names.data;                                                          \
    } while(0)

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP() (self->stack.sp - 1)
#define SECOND() (self->stack.sp - 2)
#define THIRD() (self->stack.sp - 3)
#define FOURTH() (self->stack.sp - 4)
#define STACK_SHRINK(n) (self->stack.sp -= n)
#define STACK_GROW(n) (self->stack.sp += n)
#define PUSH(v)                                                                                    \
    do {                                                                                           \
        *self->stack.sp = *(v);                                                                    \
        self->stack.sp++;                                                                          \
    } while(0)
#define POP() (--self->stack.sp)
#define POPX() (*--self->stack.sp)
#define SP() (self->stack.sp)

// [a, b] -> [?, a, b]
#define INSERT_THIRD()                                                                             \
    do {                                                                                           \
        PUSH(TOP());                                                                               \
        *SECOND() = *THIRD();                                                                      \
    } while(0)

// Must use a DISPATCH() after vectorcall_opcall() immediately!
#define vectorcall_opcall(argc, kwargc)                                                            \
    do {                                                                                           \
        FrameResult res = VM__vectorcall(self, (argc), (kwargc), true);                            \
        switch(res) {                                                                              \
            case RES_RETURN: PUSH(&self->last_retval); break;                                      \
            case RES_CALL: frame = self->top_frame; goto __NEXT_FRAME;                             \
            case RES_ERROR: goto __ERROR;                                                          \
            default: c11__unreachable();                                                           \
        }                                                                                          \
    } while(0)

static bool unpack_dict_to_buffer(py_Ref key, py_Ref val, void* ctx) {
    py_TValue** p = ctx;
    if(py_isstr(key)) {
        py_Name name = py_namev(py_tosv(key));
        py_newint(*p, (uintptr_t)name);
        py_assign(*p + 1, val);
        (*p) += 2;
        return true;
    }
    return TypeError("keywords must be strings, not '%t'", key->type);
}

FrameResult VM__run_top_frame(VM* self) {
    py_Frame* frame = self->top_frame;
    Bytecode* co_codes;
    py_Name* co_names;
    Bytecode byte;

    const py_Frame* base_frame = frame;

__NEXT_FRAME:
    if(self->recursion_depth >= self->max_recursion_depth) {
        py_exception(tp_RecursionError, "maximum recursion depth exceeded");
        goto __ERROR;
    }
    RESET_CO_CACHE();
    frame->ip++;

__NEXT_STEP:
    byte = co_codes[frame->ip];

    if(self->trace_info.func) {
        bool is_virtual = byte.op == OP_RETURN_VALUE && byte.arg == BC_RETURN_VIRTUAL;
        if(!is_virtual) {
            SourceLocation loc = Frame__source_location(frame);
            SourceLocation prev_loc = self->trace_info.prev_loc;
            if(loc.lineno != prev_loc.lineno || loc.src != prev_loc.src) {
                if(prev_loc.src) PK_DECREF(prev_loc.src);
                PK_INCREF(loc.src);
                self->trace_info.prev_loc = loc;
                self->trace_info.func(frame, TRACE_EVENT_LINE);
            }
        }
    }

#if PK_ENABLE_WATCHDOG
    if(self->watchdog_info.max_reset_time > 0) {
        if(py_debugger_status() == 0 && clock() > self->watchdog_info.max_reset_time) {
            self->watchdog_info.max_reset_time = 0;
            TimeoutError("watchdog timeout");
            goto __ERROR;
        }
    }
#endif

#ifndef NDEBUG
    pk_print_stack(self, frame, byte);
#endif

    switch((Opcode)byte.op) {
        case OP_NO_OP: DISPATCH();
        /*****************************************/
        case OP_POP_TOP: POP(); DISPATCH();
        case OP_DUP_TOP: PUSH(TOP()); DISPATCH();
        case OP_DUP_TOP_TWO:
            // [a, b]
            PUSH(SECOND());  // [a, b, a]
            PUSH(SECOND());  // [a, b, a, b]
            DISPATCH();
        case OP_ROT_TWO: {
            py_TValue tmp = *TOP();
            *TOP() = *SECOND();
            *SECOND() = tmp;
            DISPATCH();
        }
        case OP_ROT_THREE: {
            // [a, b, c] -> [c, a, b]
            py_TValue tmp = *TOP();
            *TOP() = *SECOND();
            *SECOND() = *THIRD();
            *THIRD() = tmp;
            DISPATCH();
        }
        case OP_PRINT_EXPR:
            if(TOP()->type != tp_NoneType) {
                bool ok = py_repr(TOP());
                if(!ok) goto __ERROR;
                self->callbacks.print(py_tostr(&self->last_retval));
                self->callbacks.print("\n");
            }
            POP();
            DISPATCH();
        /*****************************************/
        case OP_LOAD_CONST: {
            PUSH(c11__at(py_TValue, &frame->co->consts, byte.arg));
            DISPATCH();
        }
        case OP_LOAD_NONE: {
            py_newnone(SP()++);
            DISPATCH();
        }
        case OP_LOAD_TRUE: {
            py_newbool(SP()++, true);
            DISPATCH();
        }
        case OP_LOAD_FALSE: {
            py_newbool(SP()++, false);
            DISPATCH();
        }
        /*****************************************/
        case OP_LOAD_SMALL_INT: {
            py_newint(SP()++, (int16_t)byte.arg);
            DISPATCH();
        }
        /*****************************************/
        case OP_LOAD_ELLIPSIS: {
            py_newellipsis(SP()++);
            DISPATCH();
        }
        case OP_LOAD_FUNCTION: {
            FuncDecl_ decl = c11__getitem(FuncDecl_, &frame->co->func_decls, byte.arg);
            Function* ud = py_newobject(SP(), tp_function, 0, sizeof(Function));
            Function__ctor(ud, decl, frame->module, frame->globals);
            if(decl->nested) {
                if(frame->is_locals_special) {
                    RuntimeError("cannot create closure from special locals");
                    goto __ERROR;
                }
                ud->closure = FastLocals__to_namedict(frame->locals, frame->co);
                py_Name name = py_name(decl->code.name->data);
                // capture itself to allow recursion
                NameDict__set(ud->closure, name, SP());
            } else {
                if(self->curr_class) ud->clazz = self->curr_class->_obj;
            }
            SP()++;
            DISPATCH();
        }
        case OP_LOAD_NULL:
            py_newnil(SP()++);
            DISPATCH();
            /*****************************************/
        case OP_LOAD_FAST: {
            assert(!frame->is_locals_special);
            py_Ref val = &frame->locals[byte.arg];
            if(!py_isnil(val)) {
                PUSH(val);
                DISPATCH();
            }
            py_Name name = c11__getitem(py_Name, &frame->co->varnames, byte.arg);
            UnboundLocalError(name);
            goto __ERROR;
        }
        case OP_LOAD_NAME: {
            assert(frame->is_locals_special);
            py_Name name = co_names[byte.arg];
            // locals
            switch(frame->locals->type) {
                case tp_locals: {
                    py_Frame* noproxy = frame->locals->_ptr;
                    py_Ref slot = Frame__getlocal_noproxy(noproxy, name);
                    if(slot == NULL) break;
                    if(py_isnil(slot)) {
                        UnboundLocalError(name);
                        goto __ERROR;
                    }
                    PUSH(slot);
                    DISPATCH();
                }
                case tp_dict: {
                    int res = py_dict_getitem(frame->locals, py_name2ref(name));
                    if(res == 1) {
                        PUSH(&self->last_retval);
                        DISPATCH();
                    }
                    if(res == 0) break;
                    assert(res == -1);
                    goto __ERROR;
                }
                case tp_nil: break;
                default: c11__unreachable();
            }
            // globals
            int res = Frame__getglobal(frame, name);
            if(res == 1) {
                PUSH(&self->last_retval);
                DISPATCH();
            }
            if(res == -1) goto __ERROR;
            // builtins
            py_Ref tmp = py_getdict(self->builtins, name);
            if(tmp != NULL) {
                PUSH(tmp);
                DISPATCH();
            }
            NameError(name);
            goto __ERROR;
        }
        case OP_LOAD_NONLOCAL: {
            py_Name name = co_names[byte.arg];
            py_Ref tmp = Frame__getclosure(frame, name);
            if(tmp != NULL) {
                PUSH(tmp);
                DISPATCH();
            }
            int res = Frame__getglobal(frame, name);
            if(res == 1) {
                PUSH(&self->last_retval);
                DISPATCH();
            }
            if(res == -1) goto __ERROR;

            tmp = py_getdict(self->builtins, name);
            if(tmp != NULL) {
                PUSH(tmp);
                DISPATCH();
            }
            NameError(name);
            goto __ERROR;
        }
        case OP_LOAD_GLOBAL: {
            py_Name name = co_names[byte.arg];
            int res = Frame__getglobal(frame, name);
            if(res == 1) {
                PUSH(&self->last_retval);
                DISPATCH();
            }
            if(res == -1) goto __ERROR;
            py_Ref tmp = py_getdict(self->builtins, name);
            if(tmp != NULL) {
                PUSH(tmp);
                DISPATCH();
            }
            NameError(name);
            goto __ERROR;
        }
        case OP_LOAD_ATTR: {
            py_Name name = co_names[byte.arg];
            if(py_getattr(TOP(), name)) {
                py_assign(TOP(), py_retval());
            } else {
                goto __ERROR;
            }
            DISPATCH();
        }
        case OP_LOAD_CLASS_GLOBAL: {
            assert(self->curr_class);
            py_Name name = co_names[byte.arg];
            py_Ref tmp = py_getdict(self->curr_class, name);
            if(tmp) {
                PUSH(tmp);
                DISPATCH();
            }
            // load global if attribute not found
            int res = Frame__getglobal(frame, name);
            if(res == 1) {
                PUSH(&self->last_retval);
                DISPATCH();
            }
            if(res == -1) goto __ERROR;
            tmp = py_getdict(self->builtins, name);
            if(tmp) {
                PUSH(tmp);
                DISPATCH();
            }
            NameError(name);
            goto __ERROR;
        }
        case OP_LOAD_METHOD: {
            // [self] -> [unbound, self]
            py_Name name = co_names[byte.arg];
            bool ok = py_pushmethod(name);
            if(!ok) {
                // fallback to getattr
                if(py_getattr(TOP(), name)) {
                    py_assign(TOP(), py_retval());
                    py_newnil(SP()++);
                } else {
                    goto __ERROR;
                }
            }
            DISPATCH();
        }
        case OP_LOAD_SUBSCR: {
            // [a, b] -> a[b]
            py_Ref magic = py_tpfindmagic(SECOND()->type, __getitem__);
            if(magic) {
                if(magic->type == tp_nativefunc) {
                    if(!py_callcfunc(magic->_cfunc, 2, SECOND())) goto __ERROR;
                    POP();
                    py_assign(TOP(), py_retval());
                } else {
                    INSERT_THIRD();     // [?, a, b]
                    *THIRD() = *magic;  // [__getitem__, a, b]
                    vectorcall_opcall(1, 0);
                }
                DISPATCH();
            }
            TypeError("'%t' object is not subscriptable", SECOND()->type);
            goto __ERROR;
        }
        case OP_STORE_FAST: {
            assert(!frame->is_locals_special);
            frame->locals[byte.arg] = POPX();
            DISPATCH();
        }
        case OP_STORE_NAME: {
            assert(frame->is_locals_special);
            py_Name name = co_names[byte.arg];
            switch(frame->locals->type) {
                case tp_locals: {
                    py_Frame* noproxy = frame->locals->_ptr;
                    py_Ref slot = Frame__getlocal_noproxy(noproxy, name);
                    if(slot == NULL) {
                        UnboundLocalError(name);
                        goto __ERROR;
                    }
                    *slot = POPX();
                    DISPATCH();
                }
                case tp_dict: {
                    if(!py_dict_setitem(frame->locals, py_name2ref(name), TOP())) goto __ERROR;
                    POP();
                    DISPATCH();
                }
                case tp_nil: {
                    // globals
                    if(!Frame__setglobal(frame, name, TOP())) goto __ERROR;
                    POP();
                    DISPATCH();
                }
                default: c11__unreachable();
            }
        }
        case OP_STORE_GLOBAL: {
            py_Name name = co_names[byte.arg];
            if(!Frame__setglobal(frame, name, TOP())) goto __ERROR;
            POP();
            DISPATCH();
        }
        case OP_STORE_ATTR: {
            // [val, a] -> a.b = val
            py_Name name = co_names[byte.arg];
            if(!py_setattr(TOP(), name, SECOND())) goto __ERROR;
            STACK_SHRINK(2);
            DISPATCH();
        }
        case OP_STORE_SUBSCR: {
            // [val, a, b] -> a[b] = val
            py_Ref magic = py_tpfindmagic(SECOND()->type, __setitem__);
            if(magic) {
                PUSH(THIRD());  // [val, a, b, val]
                if(magic->type == tp_nativefunc) {
                    if(!py_callcfunc(magic->_cfunc, 3, THIRD())) goto __ERROR;
                    STACK_SHRINK(4);
                } else {
                    *FOURTH() = *magic;  // [__setitem__, a, b, val]
                    if(!py_vectorcall(2, 0)) goto __ERROR;
                }
                DISPATCH();
            }
            TypeError("'%t' object does not support item assignment", SECOND()->type);
            goto __ERROR;
        }
        case OP_DELETE_FAST: {
            assert(!frame->is_locals_special);
            py_Ref tmp = &frame->locals[byte.arg];
            if(py_isnil(tmp)) {
                py_Name name = c11__getitem(py_Name, &frame->co->varnames, byte.arg);
                UnboundLocalError(name);
                goto __ERROR;
            }
            py_newnil(tmp);
            DISPATCH();
        }
        case OP_DELETE_NAME: {
            assert(frame->is_locals_special);
            py_Name name = co_names[byte.arg];
            switch(frame->locals->type) {
                case tp_locals: {
                    py_Frame* noproxy = frame->locals->_ptr;
                    py_Ref slot = Frame__getlocal_noproxy(noproxy, name);
                    if(slot == NULL || py_isnil(slot)) {
                        UnboundLocalError(name);
                        goto __ERROR;
                    }
                    py_newnil(slot);
                    DISPATCH();
                }
                case tp_dict: {
                    int res = py_dict_delitem(frame->locals, py_name2ref(name));
                    if(res == 1) DISPATCH();
                    if(res == 0) UnboundLocalError(name);
                    goto __ERROR;
                }
                case tp_nil: {
                    // globals
                    int res = Frame__delglobal(frame, name);
                    if(res == 1) DISPATCH();
                    if(res == 0) NameError(name);
                    goto __ERROR;
                }
                default: c11__unreachable();
            }
        }
        case OP_DELETE_GLOBAL: {
            py_Name name = co_names[byte.arg];
            int res = Frame__delglobal(frame, name);
            if(res == 1) DISPATCH();
            if(res == -1) goto __ERROR;
            NameError(name);
            goto __ERROR;
        }

        case OP_DELETE_ATTR: {
            py_Name name = co_names[byte.arg];
            if(!py_delattr(TOP(), name)) goto __ERROR;
            DISPATCH();
        }

        case OP_DELETE_SUBSCR: {
            // [a, b] -> del a[b]
            py_Ref magic = py_tpfindmagic(SECOND()->type, __delitem__);
            if(magic) {
                if(magic->type == tp_nativefunc) {
                    if(!py_callcfunc(magic->_cfunc, 2, SECOND())) goto __ERROR;
                    STACK_SHRINK(2);
                } else {
                    INSERT_THIRD();     // [?, a, b]
                    *THIRD() = *magic;  // [__delitem__, a, b]
                    if(!py_vectorcall(1, 0)) goto __ERROR;
                }
                DISPATCH();
            }
            TypeError("'%t' object does not support item deletion", SECOND()->type);
            goto __ERROR;
        }
        /*****************************************/
        case OP_BUILD_IMAG: {
            // [x]
            py_Ref f = py_getdict(self->builtins, py_name("complex"));
            assert(f != NULL);
            py_TValue tmp = *TOP();
            *TOP() = *f;           // [complex]
            py_newnil(SP()++);     // [complex, NULL]
            py_newint(SP()++, 0);  // [complex, NULL, 0]
            *SP()++ = tmp;         // [complex, NULL, 0, x]
            vectorcall_opcall(2, 0);
            DISPATCH();
        }
        case OP_BUILD_BYTES: {
            int size;
            py_Ref string = c11__at(py_TValue, &frame->co->consts, byte.arg);
            const char* data = py_tostrn(string, &size);
            unsigned char* p = py_newbytes(SP()++, size);
            memcpy(p, data, size);
            DISPATCH();
        }
        case OP_BUILD_TUPLE: {
            py_TValue tmp;
            py_Ref p = py_newtuple(&tmp, byte.arg);
            py_TValue* begin = SP() - byte.arg;
            for(int i = 0; i < byte.arg; i++)
                p[i] = begin[i];
            SP() = begin;
            PUSH(&tmp);
            DISPATCH();
        }
        case OP_BUILD_LIST: {
            py_TValue tmp;
            py_newlistn(&tmp, byte.arg);
            py_TValue* begin = SP() - byte.arg;
            for(int i = 0; i < byte.arg; i++) {
                py_list_setitem(&tmp, i, begin + i);
            }
            SP() = begin;
            PUSH(&tmp);
            DISPATCH();
        }
        case OP_BUILD_DICT: {
            py_TValue* begin = SP() - byte.arg * 2;
            py_Ref tmp = py_pushtmp();
            py_newdict(tmp);
            for(int i = 0; i < byte.arg * 2; i += 2) {
                bool ok = py_dict_setitem(tmp, begin + i, begin + i + 1);
                if(!ok) goto __ERROR;
            }
            SP() = begin;
            PUSH(tmp);
            DISPATCH();
        }
        case OP_BUILD_SET: {
            py_TValue* begin = SP() - byte.arg;
            py_Ref typeobject_set = py_getdict(self->builtins, py_name("set"));
            assert(typeobject_set != NULL);
            py_push(typeobject_set);
            py_pushnil();
            if(!py_vectorcall(0, 0)) goto __ERROR;
            py_push(py_retval());  // empty set
            py_Name id_add = py_name("add");
            for(int i = 0; i < byte.arg; i++) {
                py_push(TOP());
                if(!py_pushmethod(id_add)) {
                    c11__abort("OP_BUILD_SET: failed to load method 'add'");
                }
                py_push(begin + i);
                if(!py_vectorcall(1, 0)) goto __ERROR;
            }
            py_TValue tmp = *TOP();
            SP() = begin;
            PUSH(&tmp);
            DISPATCH();
        }
        case OP_BUILD_SLICE: {
            // [start, stop, step]
            py_TValue tmp;
            py_ObjectRef slots = py_newslice(&tmp);
            slots[0] = *THIRD();
            slots[1] = *SECOND();
            slots[2] = *TOP();
            STACK_SHRINK(3);
            PUSH(&tmp);
            DISPATCH();
        }
        case OP_BUILD_STRING: {
            py_TValue* begin = SP() - byte.arg;
            c11_sbuf ss;
            c11_sbuf__ctor(&ss);
            for(int i = 0; i < byte.arg; i++) {
                if(!py_str(begin + i)) goto __ERROR;
                c11_sbuf__write_sv(&ss, py_tosv(&self->last_retval));
            }
            SP() = begin;
            c11_sbuf__py_submit(&ss, SP()++);
            DISPATCH();
        }
        /*****************************/
#define CASE_BINARY_OP(label, op, rop)                                                             \
    case label: {                                                                                  \
        if(!pk_stack_binaryop(self, op, rop)) goto __ERROR;                                        \
        POP();                                                                                     \
        *TOP() = self->last_retval;                                                                \
        DISPATCH();                                                                                \
    }
            CASE_BINARY_OP(OP_BINARY_ADD, __add__, __radd__)
            CASE_BINARY_OP(OP_BINARY_SUB, __sub__, __rsub__)
            CASE_BINARY_OP(OP_BINARY_MUL, __mul__, __rmul__)
            CASE_BINARY_OP(OP_BINARY_TRUEDIV, __truediv__, __rtruediv__)
            CASE_BINARY_OP(OP_BINARY_FLOORDIV, __floordiv__, __rfloordiv__)
            CASE_BINARY_OP(OP_BINARY_MOD, __mod__, __rmod__)
            CASE_BINARY_OP(OP_BINARY_POW, __pow__, __rpow__)
            CASE_BINARY_OP(OP_BINARY_LSHIFT, __lshift__, 0)
            CASE_BINARY_OP(OP_BINARY_RSHIFT, __rshift__, 0)
            CASE_BINARY_OP(OP_BINARY_AND, __and__, 0)
            CASE_BINARY_OP(OP_BINARY_OR, __or__, 0)
            CASE_BINARY_OP(OP_BINARY_XOR, __xor__, 0)
            CASE_BINARY_OP(OP_BINARY_MATMUL, __matmul__, 0)
            CASE_BINARY_OP(OP_COMPARE_LT, __lt__, __gt__)
            CASE_BINARY_OP(OP_COMPARE_LE, __le__, __ge__)
            CASE_BINARY_OP(OP_COMPARE_EQ, __eq__, __eq__)
            CASE_BINARY_OP(OP_COMPARE_NE, __ne__, __ne__)
            CASE_BINARY_OP(OP_COMPARE_GT, __gt__, __lt__)
            CASE_BINARY_OP(OP_COMPARE_GE, __ge__, __le__)
#undef CASE_BINARY_OP
        case OP_IS_OP: {
            bool res = py_isidentical(SECOND(), TOP());
            POP();
            if(byte.arg) res = !res;
            py_newbool(TOP(), res);
            DISPATCH();
        }
        case OP_CONTAINS_OP: {
            // [b, a] -> b __contains__ a (a in b) -> [retval]
            py_Ref magic = py_tpfindmagic(SECOND()->type, __contains__);
            if(magic) {
                if(magic->type == tp_nativefunc) {
                    if(!py_callcfunc(magic->_cfunc, 2, SECOND())) goto __ERROR;
                    STACK_SHRINK(2);
                } else {
                    INSERT_THIRD();     // [?, b, a]
                    *THIRD() = *magic;  // [__contains__, a, b]
                    if(!py_vectorcall(1, 0)) goto __ERROR;
                }
                bool res = py_tobool(py_retval());
                if(byte.arg) res = !res;
                py_newbool(SP()++, res);
                DISPATCH();
            }
            TypeError("'%t' type does not support '__contains__'", SECOND()->type);
            goto __ERROR;
        }
            /*****************************************/
        case OP_JUMP_FORWARD: DISPATCH_JUMP((int16_t)byte.arg);
        case OP_POP_JUMP_IF_NOT_MATCH: {
            int res = py_equal(SECOND(), TOP());
            if(res < 0) goto __ERROR;
            STACK_SHRINK(2);
            if(!res) DISPATCH_JUMP((int16_t)byte.arg);
            DISPATCH();
        }
        case OP_POP_JUMP_IF_FALSE: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            POP();
            if(!res) DISPATCH_JUMP((int16_t)byte.arg);
            DISPATCH();
        }
        case OP_POP_JUMP_IF_TRUE: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            POP();
            if(res) DISPATCH_JUMP((int16_t)byte.arg);
            DISPATCH();
        }
        case OP_JUMP_IF_TRUE_OR_POP: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            if(res) {
                DISPATCH_JUMP((int16_t)byte.arg);
            } else {
                POP();
                DISPATCH();
            }
        }
        case OP_JUMP_IF_FALSE_OR_POP: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            if(!res) {
                DISPATCH_JUMP((int16_t)byte.arg);
            } else {
                POP();
                DISPATCH();
            }
        }
        case OP_SHORTCUT_IF_FALSE_OR_POP: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            if(!res) {                      // [b, False]
                STACK_SHRINK(2);            // []
                py_newbool(SP()++, false);  // [False]
                DISPATCH_JUMP((int16_t)byte.arg);
            } else {
                POP();  // [b]
                DISPATCH();
            }
        }
        case OP_LOOP_CONTINUE: {
            DISPATCH_JUMP((int16_t)byte.arg);
        }
        case OP_LOOP_BREAK: {
            DISPATCH_JUMP((int16_t)byte.arg);
        }
        /*****************************************/
        case OP_CALL: {
            ManagedHeap__collect_if_needed(&self->heap);
            vectorcall_opcall(byte.arg & 0xFF, byte.arg >> 8);
            DISPATCH();
        }
        case OP_CALL_VARGS: {
            // [_0, _1, _2 | k1, v1, k2, v2]
            uint16_t argc = byte.arg & 0xFF;
            uint16_t kwargc = byte.arg >> 8;

            int n = 0;
            py_TValue* sp = SP();
            py_TValue* p1 = sp - kwargc * 2;
            py_TValue* base = p1 - argc;
            py_TValue* buf = self->vectorcall_buffer;

            for(py_TValue* curr = base; curr != p1; curr++) {
                if(curr->type != tp_star_wrapper) {
                    buf[n++] = *curr;
                } else {
                    py_TValue* args = py_getslot(curr, 0);
                    py_TValue* p;
                    int length = pk_arrayview(args, &p);
                    if(length != -1) {
                        for(int j = 0; j < length; j++) {
                            buf[n++] = p[j];
                        }
                        argc += length - 1;
                    } else {
                        TypeError("*args must be a list or tuple, got '%t'", args->type);
                        goto __ERROR;
                    }
                }
            }

            for(py_TValue* curr = p1; curr != sp; curr += 2) {
                if(curr[1].type != tp_star_wrapper) {
                    buf[n++] = curr[0];
                    buf[n++] = curr[1];
                } else {
                    assert(py_toint(&curr[0]) == 0);
                    py_TValue* kwargs = py_getslot(&curr[1], 0);
                    if(kwargs->type == tp_dict) {
                        py_TValue* p = buf + n;
                        if(!py_dict_apply(kwargs, unpack_dict_to_buffer, &p)) goto __ERROR;
                        n = p - buf;
                        kwargc += py_dict_len(kwargs) - 1;
                    } else {
                        TypeError("**kwargs must be a dict, got '%t'", kwargs->type);
                        goto __ERROR;
                    }
                }
            }

            memcpy(base, buf, n * sizeof(py_TValue));
            SP() = base + n;

            vectorcall_opcall(argc, kwargc);
            DISPATCH();
        }
        case OP_RETURN_VALUE: {
            if(byte.arg == BC_NOARG) {
                self->last_retval = POPX();
            } else {
                py_newnone(&self->last_retval);
            }
            VM__pop_frame(self);
            if(frame == base_frame) {  // [ frameBase<- ]
                return RES_RETURN;
            } else {
                frame = self->top_frame;
                PUSH(&self->last_retval);
                goto __NEXT_FRAME;
            }
            DISPATCH();
        }
        case OP_YIELD_VALUE: {
            if(byte.arg == 1) {
                py_newnone(py_retval());
            } else {
                py_assign(py_retval(), TOP());
                POP();
            }
            return RES_YIELD;
        }
        case OP_FOR_ITER_YIELD_VALUE: {
            int res = py_next(TOP());
            if(res == -1) goto __ERROR;
            if(res) {
                return RES_YIELD;
            } else {
                assert(self->last_retval.type == tp_StopIteration);
                BaseException* ud = py_touserdata(py_retval());
                py_ObjectRef value = &ud->args;
                if(py_isnil(value)) value = py_None();
                *TOP() = *value;  // [iter] -> [retval]
                DISPATCH_JUMP((int16_t)byte.arg);
            }
        }
        /////////
        case OP_LIST_APPEND: {
            // [list, iter, value]
            py_list_append(THIRD(), TOP());
            POP();
            DISPATCH();
        }
        case OP_DICT_ADD: {
            // [dict, iter, key, value]
            bool ok = py_dict_setitem(FOURTH(), SECOND(), TOP());
            if(!ok) goto __ERROR;
            STACK_SHRINK(2);
            DISPATCH();
        }
        case OP_SET_ADD: {
            // [set, iter, value]
            py_push(THIRD());  // [| set]
            if(!py_pushmethod(py_name("add"))) {
                c11__abort("OP_SET_ADD: failed to load method 'add'");
            }  // [|add() set]
            py_push(THIRD());
            if(!py_vectorcall(1, 0)) goto __ERROR;
            POP();
            DISPATCH();
        }
        /////////
        case OP_UNARY_NEGATIVE: {
            if(!pk_callmagic(__neg__, 1, TOP())) goto __ERROR;
            *TOP() = self->last_retval;
            DISPATCH();
        }
        case OP_UNARY_NOT: {
            int res = py_bool(TOP());
            if(res < 0) goto __ERROR;
            py_newbool(TOP(), !res);
            DISPATCH();
        }
        case OP_UNARY_STAR: {
            py_TValue value = POPX();
            int* level = py_newobject(SP()++, tp_star_wrapper, 1, sizeof(int));
            *level = byte.arg;
            py_setslot(TOP(), 0, &value);
            DISPATCH();
        }
        case OP_UNARY_INVERT: {
            if(!pk_callmagic(__invert__, 1, TOP())) goto __ERROR;
            *TOP() = self->last_retval;
            DISPATCH();
        }
        ////////////////
        case OP_GET_ITER: {
            if(!py_iter(TOP())) goto __ERROR;
            *TOP() = *py_retval();
            DISPATCH();
        }
        case OP_FOR_ITER: {
            int res = py_next(TOP());
            if(res == -1) goto __ERROR;
            if(res) {
                PUSH(py_retval());
                DISPATCH();
            } else {
                assert(self->last_retval.type == tp_StopIteration);
                POP();  // [iter] -> []
                DISPATCH_JUMP((int16_t)byte.arg);
            }
        }
        ////////
        case OP_IMPORT_PATH: {
            py_Ref path_object = c11__at(py_TValue, &frame->co->consts, byte.arg);
            const char* path = py_tostr(path_object);
            int res = py_import(path);
            if(res == -1) goto __ERROR;
            if(res == 0) {
                ImportError("No module named '%s'", path);
                goto __ERROR;
            }
            PUSH(py_retval());
            DISPATCH();
        }
        case OP_POP_IMPORT_STAR: {
            // [module]
            NameDict* dict = PyObject__dict(TOP()->_obj);
            py_ItemRef all = NameDict__try_get(dict, __all__);
            if(all) {
                py_TValue* p;
                int length = pk_arrayview(all, &p);
                if(length == -1) {
                    TypeError("'__all__' must be a list or tuple, got '%t'", all->type);
                    goto __ERROR;
                }
                for(int i = 0; i < length; i++) {
                    py_Name name = py_namev(py_tosv(p + i));
                    py_ItemRef value = NameDict__try_get(dict, name);
                    if(value == NULL) {
                        ImportError("cannot import name '%n'", name);
                        goto __ERROR;
                    } else {
                        if(!Frame__setglobal(frame, name, value)) goto __ERROR;
                    }
                }
            } else {
                for(int i = 0; i < dict->capacity; i++) {
                    NameDict_KV* kv = &dict->items[i];
                    if(kv->key == NULL) continue;
                    c11_sv name = py_name2sv(kv->key);
                    if(name.size == 0 || name.data[0] == '_') continue;
                    if(!Frame__setglobal(frame, kv->key, &kv->value)) goto __ERROR;
                }
            }
            POP();
            DISPATCH();
        }
        ////////
        case OP_UNPACK_SEQUENCE: {
            py_TValue* p;
            int length;

            switch(TOP()->type) {
                case tp_tuple: {
                    length = py_tuple_len(TOP());
                    p = py_tuple_data(TOP());
                    break;
                }
                case tp_list: {
                    length = py_list_len(TOP());
                    p = py_list_data(TOP());
                    break;
                }
                case tp_vec2i: {
                    length = 2;
                    if(byte.arg != length) break;
                    c11_vec2i val = py_tovec2i(TOP());
                    POP();
                    py_newint(SP()++, val.x);
                    py_newint(SP()++, val.y);
                    DISPATCH();
                }
                case tp_vec2: {
                    length = 2;
                    if(byte.arg != length) break;
                    c11_vec2 val = py_tovec2(TOP());
                    POP();
                    py_newfloat(SP()++, val.x);
                    py_newfloat(SP()++, val.y);
                    DISPATCH();
                }
                case tp_vec3i: {
                    length = 3;
                    if(byte.arg != length) break;
                    c11_vec3i val = py_tovec3i(TOP());
                    POP();
                    py_newint(SP()++, val.x);
                    py_newint(SP()++, val.y);
                    py_newint(SP()++, val.z);
                    DISPATCH();
                }
                case tp_vec3: {
                    length = 3;
                    if(byte.arg != length) break;
                    c11_vec3 val = py_tovec3(TOP());
                    POP();
                    py_newfloat(SP()++, val.x);
                    py_newfloat(SP()++, val.y);
                    py_newfloat(SP()++, val.z);
                    DISPATCH();
                }
                default: {
                    TypeError("expected list or tuple to unpack, got %t", TOP()->type);
                    goto __ERROR;
                }
            }
            if(length != byte.arg) {
                ValueError("expected %d values to unpack, got %d", byte.arg, length);
                goto __ERROR;
            }
            POP();
            for(int i = 0; i < length; i++) {
                PUSH(p + i);
            }
            DISPATCH();
        }
        case OP_UNPACK_EX: {
            py_TValue* p;
            int length = pk_arrayview(TOP(), &p);
            if(length == -1) {
                TypeError("expected list or tuple to unpack, got %t", TOP()->type);
                goto __ERROR;
            }
            int exceed = length - byte.arg;
            if(exceed < 0) {
                ValueError("not enough values to unpack");
                goto __ERROR;
            }
            POP();
            for(int i = 0; i < byte.arg; i++) {
                PUSH(p + i);
            }
            py_newlistn(SP()++, exceed);
            for(int i = 0; i < exceed; i++) {
                py_list_setitem(TOP(), i, p + byte.arg + i);
            }
            DISPATCH();
        }
        ///////////
        case OP_BEGIN_CLASS: {
            // [base]
            py_Name name = co_names[byte.arg];
            py_Type base;
            if(py_isnone(TOP())) {
                base = tp_object;
            } else {
                if(!py_checktype(TOP(), tp_type)) goto __ERROR;
                base = py_totype(TOP());
            }
            POP();

            py_TypeInfo* base_ti = pk_typeinfo(base);
            if(base_ti->is_final) {
                TypeError("type '%t' is not an acceptable base type", base);
                goto __ERROR;
            }

            py_Type type = pk_newtypewithmode(name,
                                              base,
                                              frame->module,
                                              NULL,
                                              base_ti->is_python,
                                              false,
                                              frame->co->src->mode);
            PUSH(py_tpobject(type));
            self->curr_class = TOP();
            DISPATCH();
        }
        case OP_END_CLASS: {
            // [cls or decorated]
            py_Name name = co_names[byte.arg];
            if(!Frame__setglobal(frame, name, TOP())) goto __ERROR;

            if(py_istype(TOP(), tp_type)) {
                // call on_end_subclass
                py_TypeInfo* ti = py_touserdata(TOP());
                if(ti->base != tp_object) {
                    py_TypeInfo* base_ti = ti->base_ti;
                    if(base_ti->on_end_subclass) base_ti->on_end_subclass(ti);
                }
                py_TValue* slot_eq = py_getdict(&ti->self, __eq__);
                py_TValue* slot_ne = py_getdict(&ti->self, __ne__);
                if(slot_eq && !slot_ne) {
                    TypeError("'%n' implements '__eq__' but not '__ne__'", ti->name);
                    goto __ERROR;
                }
            }
            // class with decorator is unsafe currently
            // it skips the above check
            POP();
            self->curr_class = NULL;
            DISPATCH();
        }
        case OP_STORE_CLASS_ATTR: {
            assert(self->curr_class);
            py_Name name = co_names[byte.arg];
            // TOP() can be a function, classmethod or custom decorator
            py_setdict(self->curr_class, name, TOP());
            POP();
            DISPATCH();
        }
        case OP_ADD_CLASS_ANNOTATION: {
            assert(self->curr_class);
            // [type_hint string]
            py_TypeInfo* ti = py_touserdata(self->curr_class);
            if(py_isnil(&ti->annotations)) py_newdict(&ti->annotations);
            py_Name name = co_names[byte.arg];
            bool ok = py_dict_setitem_by_str(&ti->annotations, py_name2str(name), TOP());
            if(!ok) goto __ERROR;
            POP();
            DISPATCH();
        }
        ///////////
        case OP_WITH_ENTER: {
            // [expr]
            py_push(TOP());
            if(!py_pushmethod(__enter__)) {
                TypeError("'%t' object does not support the context manager protocol", TOP()->type);
                goto __ERROR;
            }
            vectorcall_opcall(0, 0);
            DISPATCH();
        }
        case OP_WITH_EXIT: {
            // [expr]
            py_push(TOP());
            if(!py_pushmethod(__exit__)) {
                TypeError("'%t' object does not support the context manager protocol", TOP()->type);
                goto __ERROR;
            }
            if(!py_vectorcall(0, 0)) goto __ERROR;
            POP();
            DISPATCH();
        }
        ///////////
        case OP_BEGIN_TRY: {
            Frame__begin_try(frame, SP());
            DISPATCH();
        }
        case OP_END_TRY: {
            c11_vector__pop(&frame->exc_stack);
            DISPATCH();
        }
        case OP_EXCEPTION_MATCH: {
            if(!py_checktype(TOP(), tp_type)) goto __ERROR;
            bool ok = py_isinstance(&self->unhandled_exc, py_totype(TOP()));
            py_newbool(TOP(), ok);
            DISPATCH();
        }
        case OP_HANDLE_EXCEPTION: {
            FrameExcInfo* info = Frame__top_exc_info(frame);
            assert(info != NULL && py_isnil(&info->exc));
            info->exc = self->unhandled_exc;
            py_newnil(&self->unhandled_exc);
            DISPATCH();
        }
        case OP_RAISE: {
            // [exception]
            if(py_istype(TOP(), tp_type)) {
                if(!py_tpcall(py_totype(TOP()), 0, NULL)) goto __ERROR;
                py_assign(TOP(), py_retval());
            }
            if(!py_isinstance(TOP(), tp_BaseException)) {
                TypeError("exceptions must derive from BaseException");
                goto __ERROR;
            }
            py_raise(TOP());
            goto __ERROR;
        }
        case OP_RAISE_ASSERT: {
            if(byte.arg) {
                if(!py_str(TOP())) goto __ERROR;
                POP();
                py_exception(tp_AssertionError, "%s", py_tostr(py_retval()));
            } else {
                py_exception(tp_AssertionError, "");
            }
            goto __ERROR;
        }
        case OP_RE_RAISE: {
            if(py_isnil(&self->unhandled_exc)) {
                FrameExcInfo* info = Frame__top_exc_info(frame);
                assert(info != NULL && !py_isnil(&info->exc));
                self->unhandled_exc = info->exc;
            }
            c11_vector__pop(&frame->exc_stack);
            goto __ERROR_RE_RAISE;
        }
        case OP_PUSH_EXCEPTION: {
            FrameExcInfo* info = Frame__top_exc_info(frame);
            assert(info != NULL && !py_isnil(&info->exc));
            PUSH(&info->exc);
            DISPATCH();
        }
        //////////////////
        case OP_FORMAT_STRING: {
            py_Ref spec = c11__at(py_TValue, &frame->co->consts, byte.arg);
            bool ok = pk_format_object(self, TOP(), py_tosv(spec));
            if(!ok) goto __ERROR;
            py_assign(TOP(), py_retval());
            DISPATCH();
        }
        default: c11__unreachable();
    }

    c11__unreachable();

__ERROR:
    assert(!py_isnil(&self->unhandled_exc));
    py_BaseException__stpush(frame,
                             &self->unhandled_exc,
                             frame->co->src,
                             Frame__lineno(frame),
                             !frame->is_locals_special ? frame->co->name->data : NULL);
__ERROR_RE_RAISE:
    do {
        self->curr_class = NULL;
        self->curr_decl_based_function = NULL;
    } while(0);

    int target = Frame__goto_exception_handler(frame, &self->stack, &self->unhandled_exc);
    if(target >= 0) {
        // 1. Exception can be handled inside the current frame
        DISPATCH_JUMP_ABSOLUTE(target);
    } else {
        // 2. Exception need to be propagated to the upper frame
        bool is_base_frame_to_be_popped = frame == base_frame;
        VM__pop_frame(self);
        if(self->top_frame == NULL || is_base_frame_to_be_popped) {
            // propagate to the top level
            return RES_ERROR;
        }
        frame = self->top_frame;
        RESET_CO_CACHE();
        goto __ERROR;
    }

    c11__unreachable();
}

const char* pk_op2str(py_Name op) {
    if(__eq__ == op) return "==";
    if(__ne__ == op) return "!=";
    if(__lt__ == op) return "<";
    if(__le__ == op) return "<=";
    if(__gt__ == op) return ">";
    if(__ge__ == op) return ">=";
    if(__add__ == op) return "+";
    if(__sub__ == op) return "-";
    if(__mul__ == op) return "*";
    if(__truediv__ == op) return "/";
    if(__floordiv__ == op) return "//";
    if(__mod__ == op) return "%";
    if(__pow__ == op) return "**";
    if(__lshift__ == op) return "<<";
    if(__rshift__ == op) return ">>";
    if(__and__ == op) return "&";
    if(__or__ == op) return "|";
    if(__xor__ == op) return "^";
    if(__neg__ == op) return "-";
    if(__invert__ == op) return "~";
    if(__matmul__ == op) return "@";
    return py_name2str(op);
}

bool pk_stack_binaryop(VM* self, py_Name op, py_Name rop) {
    // [a, b]
    py_Ref magic = py_tpfindmagic(SECOND()->type, op);
    if(magic) {
        bool ok = py_call(magic, 2, SECOND());
        if(!ok) return false;
        if(self->last_retval.type != tp_NotImplementedType) return true;
    }
    // try reverse operation
    if(rop) {
        // [a, b] -> [b, a]
        py_TValue tmp = *TOP();
        *TOP() = *SECOND();
        *SECOND() = tmp;
        magic = py_tpfindmagic(SECOND()->type, rop);
        if(magic) {
            bool ok = py_call(magic, 2, SECOND());
            if(!ok) return false;
            if(self->last_retval.type != tp_NotImplementedType) return true;
        }
    }
    // eq/ne op never fails
    bool res = py_isidentical(SECOND(), TOP());
    if(op == __eq__) {
        py_newbool(py_retval(), res);
        return true;
    }
    if(op == __ne__) {
        py_newbool(py_retval(), !res);
        return true;
    }

    py_Type lhs_t = rop ? TOP()->type : SECOND()->type;
    py_Type rhs_t = rop ? SECOND()->type : TOP()->type;
    return TypeError("unsupported operand type(s) for '%s': '%t' and '%t'",
                     pk_op2str(op),
                     lhs_t,
                     rhs_t);
}

bool pk_format_object(VM* self, py_Ref val, c11_sv spec) {
    // format TOS via `spec` inplace
    // spec: '!r:.2f', ':.2f', '.2f'
    if(spec.size == 0) return py_str(val);

    if(spec.data[0] == '!') {
        if(c11_sv__startswith(spec, (c11_sv){"!r", 2})) {
            spec.data += 2;
            spec.size -= 2;
            if(!py_repr(val)) return false;
            py_assign(val, py_retval());
            if(spec.size == 0) return true;
        } else {
            return ValueError("invalid conversion specifier (only !r is supported)");
        }
    }

    assert(spec.size > 0);

    if(spec.data[0] == ':') {
        spec.data++;
        spec.size--;
    }

    char type;
    switch(spec.data[spec.size - 1]) {
        case 'f':
        case 'd':
        case 's':
            type = spec.data[spec.size - 1];
            spec.size--;  // remove last char
            break;
        default: type = ' '; break;
    }

    char pad_c = ' ';
    if(strchr("0-=*#@!~", spec.data[0])) {
        pad_c = spec.data[0];
        spec = c11_sv__slice(spec, 1);
    }

    char align;
    if(spec.data[0] == '^') {
        align = '^';
        spec = c11_sv__slice(spec, 1);
    } else if(spec.data[0] == '>') {
        align = '>';
        spec = c11_sv__slice(spec, 1);
    } else if(spec.data[0] == '<') {
        align = '<';
        spec = c11_sv__slice(spec, 1);
    } else {
        align = (py_isint(val) || py_isfloat(val)) ? '>' : '<';
    }

    int dot = c11_sv__index(spec, '.');
    py_i64 width, precision;

    if(dot >= 0) {
        if(dot == 0) {
            // {.2f}
            width = -1;
        } else {
            // {10.2f}
            IntParsingResult res = c11__parse_uint(c11_sv__slice2(spec, 0, dot), &width, 10);
            if(res != IntParsing_SUCCESS) return ValueError("invalid format specifier");
        }
        IntParsingResult res = c11__parse_uint(c11_sv__slice(spec, dot + 1), &precision, 10);
        if(res != IntParsing_SUCCESS) return ValueError("invalid format specifier");
    } else {
        // {10s}
        IntParsingResult res = c11__parse_uint(spec, &width, 10);
        if(res != IntParsing_SUCCESS) return ValueError("invalid format specifier");
        precision = -1;
    }

    if(type != 'f' && dot >= 0) {
        return ValueError("precision not allowed in the format specifier");
    }

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);

    if(type == 'f') {
        py_f64 x;
        if(!py_castfloat(val, &x)) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        if(precision < 0) precision = 6;
        c11_sbuf__write_f64(&buf, x, precision);
    } else if(type == 'd') {
        if(!py_checkint(val)) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_i64(&buf, py_toint(val));
    } else if(type == 's') {
        if(!py_checkstr(val)) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_sv(&buf, py_tosv(val));
    } else {
        if(!py_str(val)) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
    }

    c11_string* body = c11_sbuf__submit(&buf);
    int length = c11_sv__u8_length(c11_string__sv(body));
    c11_sbuf__ctor(&buf);  // reinit sbuf

    if(width != -1 && width > length) {
        switch(align) {
            case '>': {
                c11_sbuf__write_pad(&buf, width - length, pad_c);
                c11_sbuf__write_sv(&buf, c11_string__sv(body));
                break;
            }
            case '<': {
                c11_sbuf__write_sv(&buf, c11_string__sv(body));
                c11_sbuf__write_pad(&buf, width - length, pad_c);
                break;
            }
            case '^': {
                int pad_left = (width - length) / 2;
                int pad_right = (width - length) - pad_left;
                c11_sbuf__write_pad(&buf, pad_left, pad_c);
                c11_sbuf__write_sv(&buf, c11_string__sv(body));
                c11_sbuf__write_pad(&buf, pad_right, pad_c);
                break;
            }
            default: c11__unreachable();
        }
    } else {
        c11_sbuf__write_sv(&buf, c11_string__sv(body));
    }

    c11_string__delete(body);
    // inplace update
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

#undef CHECK_RETURN_FROM_EXCEPT_OR_FINALLY
#undef DISPATCH
#undef DISPATCH_JUMP
#undef DISPATCH_JUMP_ABSOLUTE
#undef TOP
#undef SECOND
#undef THIRD
#undef FOURTH
#undef STACK_SHRINK
#undef STACK_GROW
#undef PUSH
#undef POP
#undef POPX
#undef SP
#undef INSERT_THIRD
#undef vectorcall_opcall
#undef RESET_CO_CACHE
