#include "pocketpy/common/config.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/error.h"
#include <stdbool.h>

static bool stack_unpack_sequence(VM* self, uint16_t arg);
static bool format_object(py_Ref obj, c11_sv spec);

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
        frame->ip = c11__at(Bytecode, &frame->co->codes, __target);                                \
        goto __NEXT_STEP;                                                                          \
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

#define vectorcall_opcall(argc, kwargc)                                                            \
    do {                                                                                           \
        FrameResult res = VM__vectorcall(self, (argc), (kwargc), true);                            \
        switch(res) {                                                                              \
            case RES_RETURN: PUSH(&self->last_retval); break;                                      \
            case RES_CALL: frame = self->top_frame; goto __NEXT_FRAME;                             \
            case RES_ERROR: goto __ERROR;                                                          \
            default: c11__unreachedable();                                                         \
        }                                                                                          \
    } while(0)

static bool unpack_dict_to_buffer(py_Ref key, py_Ref val, void* ctx) {
    py_TValue** p = ctx;
    if(py_isstr(key)) {
        py_Name name = py_namev(py_tosv(key));
        py_newint(*p, name);
        py_assign(*p + 1, val);
        (*p) += 2;
        return true;
    }
    return TypeError("keywords must be strings, not '%t'", key->type);
}

FrameResult VM__run_top_frame(VM* self) {
    Frame* frame = self->top_frame;
    const Frame* base_frame = frame;

    while(true) {
        Bytecode byte;
    __NEXT_FRAME:
        frame->ip++;

    __NEXT_STEP:
        byte = *frame->ip;

        pk_print_stack(self, frame, byte);

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
                    self->print(py_tostr(&self->last_retval));
                    self->print("\n");
                }
                POP();
                DISPATCH();
            /*****************************************/
            case OP_LOAD_CONST: PUSH(c11__at(py_TValue, &frame->co->consts, byte.arg)); DISPATCH();
            case OP_LOAD_NONE: py_newnone(SP()++); DISPATCH();
            case OP_LOAD_TRUE: py_newbool(SP()++, true); DISPATCH();
            case OP_LOAD_FALSE: py_newbool(SP()++, false); DISPATCH();
            /*****************************************/
            case OP_LOAD_SMALL_INT: py_newint(SP()++, (int16_t)byte.arg); DISPATCH();
            /*****************************************/
            case OP_LOAD_ELLIPSIS: py_newellipsis(SP()++); DISPATCH();
            case OP_LOAD_FUNCTION: {
                FuncDecl_ decl = c11__getitem(FuncDecl_, &frame->co->func_decls, byte.arg);
                Function* ud = py_newobject(SP(), tp_function, 0, sizeof(Function));
                Function__ctor(ud, decl, &frame->module);
                if(decl->nested) {
                    ud->closure = FastLocals__to_namedict(frame->locals, frame->co);
                    py_Name name = py_name(decl->code.name->data);
                    // capture itself to allow recursion
                    NameDict__set(ud->closure, name, *SP());
                }
                SP()++;
                DISPATCH();
            }
            case OP_LOAD_NULL:
                py_newnil(SP()++);
                DISPATCH();
                /*****************************************/
            case OP_LOAD_FAST: {
                PUSH(&frame->locals[byte.arg]);
                if(py_isnil(TOP())) {
                    py_Name name = c11__getitem(uint16_t, &frame->co->varnames, byte.arg);
                    UnboundLocalError(name);
                    goto __ERROR;
                }
                DISPATCH();
            }
            case OP_LOAD_NAME: {
                py_Name name = byte.arg;
                py_Ref tmp = Frame__f_locals_try_get(frame, name);
                if(tmp != NULL) {
                    if(py_isnil(tmp)) {
                        UnboundLocalError(name);
                        goto __ERROR;
                    }
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = Frame__f_closure_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&frame->module, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_NONLOCAL: {
                py_Name name = byte.arg;
                py_Ref tmp = Frame__f_closure_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&frame->module, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_GLOBAL: {
                py_Name name = byte.arg;
                py_Ref tmp = py_getdict(&frame->module, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_ATTR: {
                if(py_getattr(TOP(), byte.arg)) {
                    py_assign(TOP(), py_retval());
                } else {
                    goto __ERROR;
                }
                DISPATCH();
            }
            case OP_LOAD_CLASS_GLOBAL: {
                py_Name name = byte.arg;
                py_Ref tmp = py_getdict(self->__curr_class, name);
                if(tmp) {
                    PUSH(tmp);
                    DISPATCH();
                }
                // load global if attribute not found
                tmp = py_getdict(&frame->module, name);
                if(tmp) {
                    PUSH(tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp) {
                    PUSH(tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_METHOD: {
                // [self] -> [unbound, self]
                bool ok = py_pushmethod(byte.arg);
                if(!ok) {
                    // fallback to getattr
                    if(py_getattr(TOP(), byte.arg)) {
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
                        if(!magic->_cfunc(2, SECOND())) goto __ERROR;
                        POP();
                    } else {
                        INSERT_THIRD();     // [?, a, b]
                        *THIRD() = *magic;  // [__getitem__, a, b]
                        if(!py_vectorcall(1, 0)) goto __ERROR;
                    }
                    *TOP() = self->last_retval;
                    DISPATCH();
                }
                TypeError("'%t' object is not subscriptable", SECOND()->type);
                goto __ERROR;
            }
            case OP_STORE_FAST: frame->locals[byte.arg] = POPX(); DISPATCH();
            case OP_STORE_NAME: {
                py_Name name = byte.arg;
                if(frame->function) {
                    py_Ref slot = Frame__f_locals_try_get(frame, name);
                    if(slot != NULL) {
                        *slot = *TOP();  // store in locals if possible
                    } else {
                        // Function& func = frame->_callable->as<Function>();
                        // if(func.decl == __dynamic_func_decl) {
                        //     assert(func._closure != nullptr);
                        //     func._closure->set(_name, _0);
                        // } else {
                        //     NameError(_name);
                        //     goto __ERROR;
                        // }
                    }
                } else {
                    py_setdict(&frame->module, name, TOP());
                }
                POP();
                DISPATCH();
            }
            case OP_STORE_GLOBAL: {
                py_setdict(&frame->module, byte.arg, TOP());
                POP();
                DISPATCH();
            }
            case OP_STORE_ATTR: {
                // [val, a] -> a.b = val
                if(!py_setattr(TOP(), byte.arg, SECOND())) goto __ERROR;
                STACK_SHRINK(2);
                DISPATCH();
            }
            case OP_STORE_SUBSCR: {
                // [val, a, b] -> a[b] = val
                py_Ref magic = py_tpfindmagic(SECOND()->type, __setitem__);
                if(magic) {
                    PUSH(THIRD());  // [val, a, b, val]
                    if(magic->type == tp_nativefunc) {
                        if(!magic->_cfunc(3, THIRD())) goto __ERROR;
                        STACK_SHRINK(4);
                    } else {
                        *FOURTH() = *magic;  // [__selitem__, a, b, val]
                        if(!py_vectorcall(2, 0)) goto __ERROR;
                        POP();  // discard retval
                    }
                    DISPATCH();
                }
                TypeError("'%t' object does not support item assignment", SECOND()->type);
                goto __ERROR;
            }
            case OP_DELETE_FAST: {
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
                py_Name name = byte.arg;
                if(frame->function) {
                    py_TValue* slot = Frame__f_locals_try_get(frame, name);
                    if(slot) {
                        py_newnil(slot);
                    } else {
                        // Function& func = frame->_callable->as<Function>();
                        // if(func.decl == __dynamic_func_decl) {
                        //     assert(func._closure != nullptr);
                        //     bool ok = func._closure->del(_name);
                        //     if(!ok) vm->NameError(_name);
                        // } else {
                        //     vm->NameError(_name);
                        // }
                    }
                } else {
                    bool ok = py_deldict(&frame->module, name);
                    if(!ok) {
                        NameError(name);
                        goto __ERROR;
                    }
                }
                DISPATCH();
            }
            case OP_DELETE_GLOBAL: {
                py_Name name = byte.arg;
                bool ok = py_deldict(&frame->module, name);
                if(!ok) {
                    NameError(name);
                    goto __ERROR;
                }
                DISPATCH();
            }

            case OP_DELETE_ATTR: {
                if(!py_delattr(TOP(), byte.arg)) goto __ERROR;
                DISPATCH();
            }

            case OP_DELETE_SUBSCR: {
                // [a, b] -> del a[b]
                py_Ref magic = py_tpfindmagic(SECOND()->type, __delitem__);
                if(magic) {
                    if(magic->type == tp_nativefunc) {
                        if(!magic->_cfunc(2, SECOND())) goto __ERROR;
                        STACK_SHRINK(2);
                    } else {
                        INSERT_THIRD();     // [?, a, b]
                        *THIRD() = *magic;  // [__delitem__, a, b]
                        if(!py_vectorcall(1, 0)) goto __ERROR;
                        POP();  // discard retval
                    }
                    DISPATCH();
                }
                TypeError("'%t' object does not support item deletion", SECOND()->type);
                goto __ERROR;
            }
                /*****************************************/

            case OP_BUILD_LONG: {
                // [x]
                py_Ref f = py_getdict(&self->builtins, py_name("long"));
                assert(f != NULL);
                if(!py_call(f, 1, TOP())) goto __ERROR;
                *TOP() = self->last_retval;
                DISPATCH();
            }

            case OP_BUILD_IMAG: {
                // [x]
                py_Ref f = py_getdict(&self->builtins, py_name("complex"));
                assert(f != NULL);
                py_TValue tmp = *TOP();
                *TOP() = *f;           // [complex]
                py_newnil(SP()++);     // [complex, NULL]
                py_newint(SP()++, 0);  // [complex, NULL, 0]
                *SP()++ = tmp;         // [complex, NULL, 0, x]
                if(!py_vectorcall(2, 0)) goto __ERROR;
                PUSH(py_retval());
                DISPATCH();
            }
            case OP_BUILD_BYTES: {
                int size;
                const char* data = py_tostrn(TOP(), &size);
                unsigned char* p = py_newbytes(TOP(), size);
                memcpy(p, data, size);
                DISPATCH();
            }
            case OP_BUILD_TUPLE: {
                py_TValue tmp;
                py_newtuple(&tmp, byte.arg);
                py_TValue* begin = SP() - byte.arg;
                for(int i = 0; i < byte.arg; i++) {
                    py_tuple_setitem(&tmp, i, begin + i);
                }
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
                    py_dict_setitem(tmp, begin + i, begin + i + 1);
                    if(py_checkexc()) goto __ERROR;
                }
                SP() = begin;
                PUSH(tmp);
                DISPATCH();
            }
            case OP_BUILD_SET: {
                py_TValue* begin = SP() - byte.arg;
                py_Ref typeobject_set = py_getdict(&self->builtins, py_name("set"));
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
                py_newslice(&tmp);
                py_setslot(&tmp, 0, THIRD());
                py_setslot(&tmp, 1, SECOND());
                py_setslot(&tmp, 2, TOP());
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
            case OP_BINARY_OP: {
                py_Name op = byte.arg & 0xFF;
                py_Name rop = byte.arg >> 8;
                if(!pk_stack_binaryop(self, op, rop)) goto __ERROR;
                POP();
                *TOP() = self->last_retval;
                DISPATCH();
            }
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
                        if(!magic->_cfunc(2, SECOND())) goto __ERROR;
                        POP();
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
            case OP_LOOP_CONTINUE:
                // just an alias of OP_JUMP_FORWARD
                DISPATCH_JUMP((int16_t)byte.arg);
            case OP_LOOP_BREAK: {
                int target = Frame__ip(frame) + byte.arg;
                Frame__prepare_jump_break(frame, &self->stack, target);
                DISPATCH_JUMP((int16_t)byte.arg);
            }
            case OP_JUMP_ABSOLUTE_TOP: {
                int target = py_toint(TOP());
                POP();
                DISPATCH_JUMP_ABSOLUTE(target);
            }
            case OP_GOTO: {
                int target = c11_smallmap_n2i__get(&frame->co->labels, byte.arg, -1);
                if(target < 0) {
                    RuntimeError("label '%n' not found", byte.arg);
                    goto __ERROR;
                }
                Frame__prepare_jump_break(frame, &self->stack, target);
                DISPATCH_JUMP_ABSOLUTE(target);
            }
                /*****************************************/
            case OP_REPR: {
                if(!py_repr(TOP())) goto __ERROR;
                py_assign(TOP(), py_retval());
                DISPATCH();
            }
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
                py_TValue* buf = self->__vectorcall_buffer;

                for(py_TValue* curr = base; curr != p1; curr++) {
                    if(curr->type != tp_star_wrapper) {
                        buf[n++] = *curr;
                    } else {
                        py_TValue* args = py_getslot(curr, 0);
                        int length;
                        py_TValue* p = pk_arrayview(args, &length);
                        if(p) {
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
                assert(false);
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
                py_dict_setitem(FOURTH(), SECOND(), TOP());
                if(py_checkexc()) goto __ERROR;
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
                    int target = Frame__prepare_loop_break(frame, &self->stack);
                    DISPATCH_JUMP_ABSOLUTE(target);
                }
            }
            ////////
            case OP_IMPORT_PATH: {
                py_Ref path_object = c11__at(py_TValue, &frame->co->consts, byte.arg);
                const char* path = py_tostr(path_object);
                int res = py_import(path);
                if(res == -1) goto __ERROR;
                if(res == 0) {
                    ImportError("module '%s' not found", path);
                    goto __ERROR;
                }
                PUSH(py_retval());
                DISPATCH();
            }
            case OP_POP_IMPORT_STAR: {
                // [module]
                NameDict* dict = PyObject__dict(TOP()->_obj);
                py_Ref all = NameDict__try_get(dict, __all__);
                if(all) {
                    int length;
                    py_TValue* p = pk_arrayview(all, &length);
                    if(!p) {
                        TypeError("'__all__' must be a list or tuple, got '%t'", all->type);
                        goto __ERROR;
                    }
                    for(int i = 0; i < length; i++) {
                        py_Name name = py_namev(py_tosv(p + i));
                        py_Ref value = NameDict__try_get(dict, name);
                        if(value == NULL) {
                            ImportError("cannot import name '%n'", name);
                            goto __ERROR;
                        } else {
                            py_setdict(&frame->module, name, value);
                        }
                    }
                } else {
                    for(int i = 0; i < dict->count; i++) {
                        NameDict_KV* kv = c11__at(NameDict_KV, dict, i);
                        if(!kv->key) continue;
                        c11_sv name = py_name2sv(kv->key);
                        if(name.size == 0 || name.data[0] == '_') continue;
                        py_setdict(&frame->module, kv->key, &kv->value);
                    }
                }
                POP();
                DISPATCH();
            }
            ////////
            case OP_UNPACK_SEQUENCE: {
                if(!stack_unpack_sequence(self, byte.arg)) goto __ERROR;
                DISPATCH();
            }
            case OP_UNPACK_EX: {
                int length;
                py_TValue* p = pk_arrayview(TOP(), &length);
                if(!p) {
                    TypeError("expected list or tuple to unpack, got '%t'", TOP()->type);
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
                py_Name name = byte.arg;
                py_Type base;
                if(py_isnone(TOP())) {
                    base = tp_object;
                } else {
                    if(!py_checktype(TOP(), tp_type)) goto __ERROR;
                    base = py_totype(TOP());
                }
                POP();
                py_Type type =
                    pk_newtype(py_name2str(name), base, &frame->module, NULL, true, false);
                PUSH(py_tpobject(type));
                self->__curr_class = TOP();
                DISPATCH();
            }
            case OP_END_CLASS: {
                // [cls or decorated]
                py_Name name = byte.arg;
                // set into f_globals
                py_setdict(&frame->module, name, TOP());

                if(py_istype(TOP(), tp_type)) {
                    // call on_end_subclass
                    py_TypeInfo* ti = c11__at(py_TypeInfo, &self->types, py_totype(TOP()));
                    if(ti->base != tp_object) {
                        // PyTypeInfo* base_ti = &_all_types[ti->base];
                        py_TypeInfo* base_ti = c11__at(py_TypeInfo, &self->types, ti->base);
                        if(base_ti->on_end_subclass) base_ti->on_end_subclass(ti);
                    }
                }
                POP();
                self->__curr_class = NULL;
                DISPATCH();
            }
            case OP_STORE_CLASS_ATTR: {
                py_Name name = byte.arg;
                if(py_istype(TOP(), tp_function)) {
                    Function* ud = py_touserdata(TOP());
                    ud->clazz = self->__curr_class->_obj;
                }
                py_setdict(self->__curr_class, name, TOP());
                POP();
                DISPATCH();
            }
            case OP_ADD_CLASS_ANNOTATION: {
                py_Type type = py_totype(self->__curr_class);
                py_TypeInfo* ti = c11__at(py_TypeInfo, &self->types, type);
                c11_vector__push(py_Name, &ti->annotated_fields, byte.arg);
                DISPATCH();
            }
            ///////////
            case OP_TRY_ENTER: {
                Frame__set_unwind_target(frame, SP());
                DISPATCH();
            }
            case OP_EXCEPTION_MATCH: {
                if(!py_checktype(TOP(), tp_type)) goto __ERROR;
                bool ok = py_isinstance(&self->curr_exception, py_totype(TOP()));
                py_newbool(TOP(), ok);
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
                assert(self->curr_exception.type);
                goto __ERROR_RE_RAISE;
            }
            case OP_PUSH_EXCEPTION: {
                assert(self->curr_exception.type);
                PUSH(&self->curr_exception);
                DISPATCH();
            }
            case OP_POP_EXCEPTION: {
                assert(self->curr_exception.type);
                py_clearexc(NULL);
                DISPATCH();
            }
            //////////////////
            case OP_FSTRING_EVAL: {
                py_TValue* tmp = c11__at(py_TValue, &frame->co->consts, byte.arg);
                const char* string = py_tostr(tmp);
                // TODO: optimize this
                if(!py_exec(string, "<eval>", EVAL_MODE, &frame->module)) goto __ERROR;
                PUSH(py_retval());
                DISPATCH();
            }
            case OP_FORMAT_STRING: {
                py_Ref spec = c11__at(py_TValue, &frame->co->consts, byte.arg);
                bool ok = format_object(TOP(), py_tosv(spec));
                if(!ok) goto __ERROR;
                py_assign(TOP(), py_retval());
                DISPATCH();
            }
            default: c11__unreachedable();
        }

        c11__unreachedable();

    __ERROR:
        pk_print_stack(self, frame, (Bytecode){0});
        py_BaseException__set_lineno(&self->curr_exception, Frame__lineno(frame), frame->co);
    __ERROR_RE_RAISE:
        do {
        } while(0);
        // printf("error.op: %s, line: %d\n", pk_opname(byte.op), Frame__lineno(frame));
        int lineno = py_BaseException__get_lineno(&self->curr_exception, frame->co);
        py_BaseException__stpush(&self->curr_exception,
                                 frame->co->src,
                                 lineno < 0 ? Frame__lineno(frame) : lineno,
                                 frame->function ? frame->co->name->data : NULL);

        int target = Frame__prepare_jump_exception_handler(frame, &self->stack);
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
            goto __ERROR;
        }
    }

    return RES_RETURN;
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
    if(op == __eq__ || op == __ne__) {
        bool res = py_isidentical(SECOND(), TOP());
        py_newbool(py_retval(), res);
        return true;
    }
    return TypeError("unsupported operand type(s) for '%n'", op);
}

bool py_binaryop(py_Ref lhs, py_Ref rhs, py_Name op, py_Name rop) {
    VM* self = pk_current_vm;
    PUSH(lhs);
    PUSH(rhs);
    bool ok = pk_stack_binaryop(self, op, rop);
    STACK_SHRINK(2);
    return ok;
}

static bool stack_unpack_sequence(VM* self, uint16_t arg) {
    int length;
    py_TValue* p = pk_arrayview(TOP(), &length);
    if(!p) return TypeError("expected list or tuple to unpack, got '%t'", TOP()->type);
    if(length != arg) return ValueError("expected %d values to unpack, got %d", arg, length);
    POP();
    for(int i = 0; i < length; i++) {
        PUSH(p + i);
    }
    return true;
}

static bool format_object(py_Ref val, c11_sv spec) {
    if(spec.size == 0) return py_str(val);

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
            if(res != IntParsing_SUCCESS) return ValueError("invalid format specifer");
        }
        IntParsingResult res = c11__parse_uint(c11_sv__slice(spec, dot + 1), &precision, 10);
        if(res != IntParsing_SUCCESS) return ValueError("invalid format specifer");
    } else {
        // {10s}
        IntParsingResult res = c11__parse_uint(spec, &width, 10);
        if(res != IntParsing_SUCCESS) return ValueError("invalid format specifer");
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
            default: c11__unreachedable();
        }
    } else {
        c11_sbuf__write_sv(&buf, c11_string__sv(body));
    }

    c11_string__delete(body);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}