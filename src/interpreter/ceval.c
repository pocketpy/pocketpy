#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>

static bool stack_unpack_sequence(pk_VM* self, uint16_t arg);

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
        pk_FrameResult res = pk_VM__vectorcall(self, (argc), (kwargc), true);                      \
        switch(res) {                                                                              \
            case RES_RETURN: PUSH(&self->last_retval); break;                                      \
            case RES_CALL: frame = self->top_frame; goto __NEXT_FRAME;                             \
            case RES_ERROR: goto __ERROR;                                                          \
            default: c11__unreachedable();                                                         \
        }                                                                                          \
    } while(0)

pk_FrameResult pk_VM__run_top_frame(pk_VM* self) {
    Frame* frame = self->top_frame;
    const Frame* base_frame = frame;

    while(true) {
        Bytecode byte;
    __NEXT_FRAME:
        // if(__internal_exception.type == InternalExceptionType::Null) {
        //     // None
        //     frame->_ip++;
        // } else if(__internal_exception.type == InternalExceptionType::Handled) {
        //     // HandledException + continue
        //     frame->_ip = c11__at(Bytecode, &frame->co->codes, __internal_exception.arg);
        //     __internal_exception = {};
        // } else {
        //     // UnhandledException + continue (need_raise = true)
        //     // ToBeRaisedException + continue (need_raise = true)
        //     __internal_exception = {};
        //     __raise_exc();  // no return
        // }

        frame->ip++;

    __NEXT_STEP:
        byte = *frame->ip;

#if 1
        c11_sbuf buf;
        c11_sbuf__ctor(&buf);
        for(py_Ref p = self->stack.begin; p != SP(); p++) {
            switch(p->type) {
                case 0: c11_sbuf__write_cstr(&buf, "nil"); break;
                case tp_int: c11_sbuf__write_i64(&buf, p->_i64); break;
                case tp_float: c11_sbuf__write_f64(&buf, p->_f64, -1); break;
                case tp_bool: c11_sbuf__write_cstr(&buf, p->_bool ? "True" : "False"); break;
                case tp_none_type: c11_sbuf__write_cstr(&buf, "None"); break;
                case tp_list: {
                    pk_sprintf(&buf, "list(%d)", py_list__len(p));
                    break;
                }
                case tp_tuple: {
                    pk_sprintf(&buf, "tuple(%d)", py_tuple__len(p));
                    break;
                }
                case tp_function: {
                    Function* ud = py_touserdata(p);
                    c11_sbuf__write_cstr(&buf, ud->decl->code.name->data);
                    c11_sbuf__write_cstr(&buf, "()");
                    break;
                }
                case tp_type: {
                    pk_sprintf(&buf, "<class '%t'>", py_totype(p));
                    break;
                }
                case tp_str: {
                    int size;
                    const char* data = py_tostrn(p, &size);
                    pk_sprintf(&buf, "%q", (c11_sv){data, size});
                    break;
                }
                default: {
                    pk_sprintf(&buf, "(%t)", p->type);
                    break;
                }
            }
            if(p != TOP()) c11_sbuf__write_cstr(&buf, ", ");
        }
        c11_string* stack_str = c11_sbuf__submit(&buf);
        printf("L%-3d: %-25s %-6d [%s]\n",
               Frame__lineno(frame),
               pk_opname(byte.op),
               byte.arg,
               stack_str->data);
        c11_string__delete(stack_str);
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
                if(TOP()->type != tp_none_type) {
                    bool ok = py_repr(TOP());
                    if(!ok) goto __ERROR;
                    self->_stdout("%s\n", py_tostr(&self->last_retval));
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
                Function__ctor(ud, decl, frame->module);
                if(decl->nested) {
                    ud->closure = FastLocals__to_namedict(frame->locals, frame->locals_co);
                    py_Name name = py_namev(c11_string__sv(decl->code.name));
                    // capture itself to allow recursion
                    pk_NameDict__set(ud->closure, name, *SP());
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
                tmp = Frame__f_globals_try_get(frame, name);
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
                tmp = Frame__f_globals_try_get(frame, name);
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
                py_Ref tmp = Frame__f_globals_try_get(frame, name);
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
                if(!py_getattr(TOP(), byte.arg, TOP())) {
                    AttributeError(TOP(), byte.arg);
                    goto __ERROR;
                }
                DISPATCH();
            }
            case OP_LOAD_CLASS_GLOBAL: {
                assert(self->__curr_class.type);
                py_Name name = byte.arg;
                if(py_getattr(&self->__curr_class, name, SP())) {
                    SP()++;
                    DISPATCH();
                }
                // load global if attribute not found
                py_Ref tmp = Frame__f_globals_try_get(frame, name);
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
                // [self]
                bool ok = py_getunboundmethod(TOP(), byte.arg, TOP(), SP());
                if(ok) {
                    // [unbound, self]
                    SP()++;
                } else {
                    // fallback to getattr
                    int res = py_getattr(TOP(), byte.arg, TOP());
                    if(res != 1) {
                        if(res == 0) { AttributeError(TOP(), byte.arg); }
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
                        py_TValue* next_sp = TOP();
                        bool ok = magic->_cfunc(2, SECOND());
                        if(!ok) goto __ERROR;
                        SP() = next_sp;
                        *TOP() = self->last_retval;
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
            case OP_STORE_FAST: frame->locals[byte.arg] = POPX(); DISPATCH();
            case OP_STORE_NAME: {
                py_Name _name = byte.arg;
                py_TValue _0 = POPX();
                if(frame->function) {
                    py_Ref slot = Frame__f_locals_try_get(frame, _name);
                    if(slot != NULL) {
                        *slot = _0;  // store in locals if possible
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
                    pk_NameDict__set(Frame__f_globals(frame), _name, _0);
                }
                DISPATCH();
            }
            case OP_STORE_GLOBAL:
                pk_NameDict__set(Frame__f_globals(frame), byte.arg, POPX());
                DISPATCH();

            case OP_STORE_ATTR: {
                int err = py_setattr(TOP(), byte.arg, SECOND());
                if(err) goto __ERROR;
                STACK_SHRINK(2);
                DISPATCH();
            }
            case OP_STORE_SUBSCR: {
                // [val, a, b] -> a[b] = val
                PUSH(THIRD());  // [val, a, b, val]
                py_Ref magic = py_tpfindmagic(SECOND()->type, __setitem__);
                if(magic) {
                    if(magic->type == tp_nativefunc) {
                        py_TValue* next_sp = THIRD();
                        bool ok = magic->_cfunc(3, THIRD());
                        if(!ok) goto __ERROR;
                        SP() = next_sp;
                        *TOP() = self->last_retval;
                    } else {
                        *FOURTH() = *magic;  // [__selitem__, a, b, val]
                        vectorcall_opcall(2, 0);
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
                    // if(!frame->f_globals().del(_name)) vm->NameError(_name);
                    bool ok = pk_NameDict__del(Frame__f_globals(frame), name);
                    if(!ok) {
                        NameError(name);
                        goto __ERROR;
                    }
                }
                DISPATCH();
            }
            case OP_DELETE_GLOBAL: {
                py_Name name = byte.arg;
                bool ok = pk_NameDict__del(Frame__f_globals(frame), name);
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
                        py_TValue* next_sp = SECOND();
                        bool ok = magic->_cfunc(2, SECOND());
                        if(!ok) goto __ERROR;
                        SP() = next_sp;
                    } else {
                        INSERT_THIRD();     // [?, a, b]
                        *THIRD() = *magic;  // [__delitem__, a, b]
                        vectorcall_opcall(1, 0);
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
                *TOP() = *f;              // [complex]
                py_newnil(SP()++);        // [complex, NULL]
                py_newint(SP()++, 0);     // [complex, NULL, 0]
                *SP()++ = tmp;            // [complex, NULL, 0, x]
                vectorcall_opcall(2, 0);  // [complex(x)]
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
                    py_tuple__setitem(&tmp, i, begin + i);
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
                    py_list__setitem(&tmp, i, begin + i);
                }
                SP() = begin;
                PUSH(&tmp);
                DISPATCH();
            }
            case OP_BUILD_DICT: {
                py_TValue* begin = SP() - byte.arg;
                py_Ref tmp = py_pushtmp();
                py_newdict(tmp);
                for(int i = 0; i < byte.arg; i += 2) {
                    if(!py_setitem(tmp, begin + i, begin + i + 1)) goto __ERROR;
                }
                SP() = begin;
                PUSH(tmp);
                DISPATCH();
            }
            case OP_BUILD_SET: {
                py_TValue* begin = SP() - byte.arg;
                py_Ref tmp = py_pushtmp();
                py_newset(tmp);
                py_Name id_add = py_name("add");
                for(int i = 0; i < byte.arg; i++) {
                    if(!py_callmethod(tmp, id_add, 1, begin + i)) goto __ERROR;
                }
                SP() = begin;
                PUSH(tmp);
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
                    int size;
                    const char* data = py_tostrn(&self->last_retval, &size);
                    c11_sbuf__write_cstrn(&ss, data, size);
                }
                SP() = begin;
                c11_string* res = c11_sbuf__submit(&ss);
                py_newstrn(SP()++, res->data, res->size);
                c11_string__delete(res);
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
                        py_TValue* next_sp = TOP();
                        bool ok = magic->_cfunc(2, SECOND());
                        if(!ok) goto __ERROR;
                        SP() = next_sp;
                        *TOP() = self->last_retval;
                    } else {
                        INSERT_THIRD();     // [?, b, a]
                        *THIRD() = *magic;  // [__contains__, a, b]
                        vectorcall_opcall(1, 0);
                    }
                    bool res = py_tobool(TOP());
                    if(byte.arg) py_newbool(TOP(), !res);
                    DISPATCH();
                }
                // TODO: fallback to __iter__?
                TypeError("argument of type '%t' is not iterable", SECOND()->type);
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
                // case OP_GOTO: {
                //     py_Name _name(byte.arg);
                //     int target = c11_smallmap_n2i__get(&frame->co->labels, byte.arg, -1);
                //     if(target < 0) RuntimeError(_S("label ", _name.escape(), " not found"));
                //     frame->prepare_jump_break(&s_data, target);
                //     DISPATCH_JUMP_ABSOLUTE(target)
                // }
                /*****************************************/
            case OP_FSTRING_EVAL: {
                assert(false);
            }
            case OP_REPR: {
                assert(false);
            }
            case OP_CALL: {
                pk_ManagedHeap__collect_if_needed(&self->heap);
                vectorcall_opcall(byte.arg & 0xFF, byte.arg >> 8);
                DISPATCH();
            }
            case OP_CALL_VARGS: {
                assert(false);
            }
            case OP_RETURN_VALUE: {
                if(byte.arg == BC_NOARG) {
                    self->last_retval = POPX();
                } else {
                    py_newnone(&self->last_retval);
                }
                pk_VM__pop_frame(self);
                if(frame == base_frame) {  // [ frameBase<- ]
                    return RES_RETURN;
                } else {
                    frame = self->top_frame;
                    PUSH(&self->last_retval);
                    goto __NEXT_FRAME;
                }
                DISPATCH();
            }

                /////////
            case OP_UNARY_NEGATIVE: {
                if(!py_callmagic(__neg__, 1, TOP())) goto __ERROR;
                *TOP() = self->last_retval;
                DISPATCH();
            }
            case OP_UNARY_NOT: {
                int res = py_bool(TOP());
                if(res < 0) goto __ERROR;
                py_newbool(TOP(), !res);
                DISPATCH();
            }
            // case OP_UNARY_STAR: TOP() = VAR(StarWrapper(byte.arg, TOP())); DISPATCH();
            case OP_UNARY_INVERT: {
                if(!py_callmagic(__invert__, 1, TOP())) goto __ERROR;
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
                    py_list__setitem(TOP(), i, p + byte.arg + i);
                }
                DISPATCH();
            }

            ///////////
            case OP_RAISE_ASSERT: {
                if(byte.arg) {
                    if(!py_str(TOP())) goto __ERROR;
                    POP();
                    py_exception("AssertionError", "%s", py_tostr(py_retval()));
                } else {
                    py_exception("AssertionError", "");
                }
                goto __ERROR;
            }
            default: c11__unreachedable();
        }

        assert(false);  // should never reach here

    __ERROR:
        // 1. Exception can be handled inside the current frame
        // 2. Exception need to be propagated to the upper frame
        printf("error.op: %s, line: %d\n", pk_opname(byte.op), Frame__lineno(frame));
        return RES_ERROR;
    }

    return RES_RETURN;
}

bool pk_stack_binaryop(pk_VM* self, py_Name op, py_Name rop) {
    // [a, b]
    py_Ref magic = py_tpfindmagic(SECOND()->type, op);
    if(magic) {
        bool ok = py_call(magic, 2, SECOND());
        if(!ok) return false;
        if(self->last_retval.type != tp_not_implemented_type) return true;
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
            if(self->last_retval.type != tp_not_implemented_type) return true;
        }
    }
    // eq/ne op never fails due to object.__eq__
    return py_exception("TypeError", "unsupported operand type(s) for '%n'", op);
}

bool py_binaryop(const py_Ref lhs, const py_Ref rhs, py_Name op, py_Name rop) {
    pk_VM* self = pk_current_vm;
    PUSH(lhs);
    PUSH(rhs);
    bool ok = pk_stack_binaryop(self, op, rop);
    STACK_SHRINK(2);
    return ok;
}

static bool stack_unpack_sequence(pk_VM* self, uint16_t arg) {
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
