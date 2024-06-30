#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

int UnboundLocalError(py_Name name) { return -1; }

int NameError(py_Name name) { return -1; }

#define AttributeError(obj, name)

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
#define PUSH(v) (*self->stack.sp++ = *v)
#define POP() (--self->stack.sp)
#define POPX() (*--self->stack.sp)
#define SP() (self->stack.sp)

#define vectorcall_opcall(n)                                                                       \
    do {                                                                                           \
        pk_FrameResult res = pk_vectorcall(n, 0, true);                                            \
        switch(res) {                                                                              \
            case RES_RETURN: PUSH(&self->last_retval); break;                                      \
            case RES_CALL:                                                                         \
                frame = self->top_frame;                                                           \
                PUSH(&self->last_retval);                                                          \
                goto __NEXT_FRAME;                                                                 \
            case RES_ERROR: goto __ERROR;                                                          \
            default: PK_UNREACHABLE();                                                             \
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
                    py_TValue tmp;
                    if(py_repr(TOP(), &tmp)) self->_stdout("%s\n", py_tostr(&tmp));
                }
                POP();
                DISPATCH();
            /*****************************************/
            case OP_LOAD_CONST: PUSH(c11__at(py_TValue, &frame->co->consts, byte.arg)); DISPATCH();
            case OP_LOAD_NONE: PUSH(&self->None); DISPATCH();
            case OP_LOAD_TRUE: PUSH(&self->True); DISPATCH();
            case OP_LOAD_FALSE: PUSH(&self->False); DISPATCH();
            /*****************************************/
            case OP_LOAD_SMALL_INT: py_newint(SP()++, (int64_t)(int16_t)byte.arg); DISPATCH();
            /*****************************************/
            case OP_LOAD_ELLIPSIS: PUSH(&self->Ellipsis); DISPATCH();
            case OP_LOAD_FUNCTION: {
                // FuncDecl_ decl = c11__getitem(FuncDecl_, &frame->co->func_decls, byte.arg);
                // py_TValue obj;
                // if(decl->nested) {
                //     NameDict* captured = frame->_locals.to_namedict();
                //     obj =
                //         new_object<Function>(tp_function, decl, frame->_module, nullptr,
                //         captured);
                //     uint16_t name = pk_StrName__map2(py_Str__sv(&decl->code->name));
                //     captured->set(name, obj);
                // } else {
                //     obj = new_object<Function>(tp_function, decl, frame->_module, nullptr,
                //     nullptr);
                // }
                // PUSH(obj);DISPATCH();
            }
            case OP_LOAD_NULL:
                py_newnull(SP()++);
                DISPATCH();
                /*****************************************/
            case OP_LOAD_FAST: {
                PUSH(&frame->locals[byte.arg]);
                if(py_isnull(TOP())) {
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
                    if(py_isnull(tmp)) {
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
                // `py_getunboundmethod` never fails on `fallback=true`
                py_getunboundmethod(TOP(), byte.arg, true, TOP(), SP());
                SP()++;
                DISPATCH();
            }
            case OP_LOAD_SUBSCR: {
                // [a, b] -> a[b]
                pk_TypeInfo* ti = pk_tpinfo(SECOND());
                if(ti->m__getitem__) {
                    if(!ti->m__getitem__(2, SECOND(), SECOND())) goto __ERROR;
                } else {
                    if(!py_callmethod(SECOND(), __getitem__, TOP())) goto __ERROR;
                    // // [a, b] -> [?, a, b]
                    // PUSH(TOP());           // [a, b, b]
                    // *SECOND() = *THIRD();  // [a, a, b]
                    // bool ok = py_getunboundmethod(SECOND(), __getitem__, false, THIRD(),
                    // SECOND()); if(!ok) {
                    //     // __getitem__ not found
                    //     goto __ERROR;
                    // }
                    // py_vectorcall(2, 0, );
                }
                DISPATCH();
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
                pk_TypeInfo* ti = pk_tpinfo(SECOND());
                PUSH(THIRD());  // [val, a, b, val]
                if(ti->m__setitem__) {
                    if(!ti->m__setitem__(3, THIRD(), FOURTH())) goto __ERROR;
                    STACK_SHRINK(3);  // [retval]
                } else {
                    bool ok = py_getunboundmethod(THIRD(), __setitem__, false, FOURTH(), THIRD());
                    if(!ok) goto __ERROR;
                    // [__setitem__, self, b, val]
                    vectorcall_opcall(3);
                    POP();  // discard retval
                }
                DISPATCH();
            }
            case OP_DELETE_FAST: {
                py_Ref tmp = &frame->locals[byte.arg];
                if(py_isnull(tmp)) {
                    UnboundLocalError(c11__getitem(uint16_t, &frame->co->varnames, byte.arg));
                    goto __ERROR;
                }
                py_newnull(tmp);
                DISPATCH();
            }
            case OP_DELETE_NAME: {
                StrName name = byte.arg;
                if(frame->function) {
                    py_TValue* slot = Frame__f_locals_try_get(frame, name);
                    if(slot) {
                        py_newnull(slot);
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
                StrName name = byte.arg;
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
                pk_TypeInfo* ti = pk_tpinfo(SECOND());
                if(ti->m__delitem__) {
                    if(!ti->m__delitem__(2, SECOND(), SECOND())) goto __ERROR;
                    POP();
                } else {
                    PUSH(TOP());           // [a, b, b]
                    *SECOND() = *THIRD();  // [a, a, b]
                    bool ok = py_getunboundmethod(SECOND(), __delitem__, false, THIRD(), SECOND());
                    // [__delitem__, self, b]
                    if(!ok) goto __ERROR;
                    vectorcall_opcall(2);
                    POP();  // discard retval
                }
                DISPATCH();
            }

                /*****************************************/

            case OP_BUILD_LONG: {
                // [x]
                py_Ref f = py_getdict(&self->builtins, pk_id_long);
                assert(f != NULL);
                if(!py_call(f, TOP())) goto __ERROR;
                *TOP() = self->last_retval;
                DISPATCH();
            }

            case OP_BUILD_IMAG: {
                py_Ref _0 = py_getdict(&self->builtins, pk_id_complex);
                assert(_0 != NULL);
                py_TValue zero;
                py_newint(&zero, 0);
                if(!py_call(_0, &zero, TOP())) goto __ERROR;
                *TOP() = self->last_retval;
                DISPATCH();
            }
            case OP_BUILD_BYTES: {
                py_Str* s = py_touserdata(TOP());
                unsigned char* p = (unsigned char*)malloc(s->size);
                memcpy(p, py_Str__data(s), s->size);
                py_newbytes(TOP(), p, s->size);
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
            // case OP_BUILD_LIST: {
            //     PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
            //     STACK_SHRINK(byte.arg);
            //     PUSH(_0);
            //     DISPATCH();
            // }
            // case OP_BUILD_DICT: {
            //     if(byte.arg == 0) {
            //         PUSH(VAR(Dict()));
            //         DISPATCH()
            //     }
            //     PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
            //     _0 = call(_t(tp_dict), _0);
            //     STACK_SHRINK(byte.arg);
            //     PUSH(_0);
            //     DISPATCH();
            // }
            // case OP_BUILD_SET: {
            //     PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
            //     _0 = call(builtins->attr()[pk_id_set], _0);
            //     STACK_SHRINK(byte.arg);
            //     PUSH(_0);
            //     DISPATCH();
            // }
            // case OP_BUILD_SLICE: {
            //     PyVar _2 = POPX();  // step
            //     PyVar _1 = POPX();  // stop
            //     PyVar _0 = POPX();  // start
            //     PUSH(VAR(Slice(_0, _1, _2)));
            //     DISPATCH();
            // }
            // case OP_BUILD_STRING: {
            //     SStream ss;
            //     ArgsView view = STACK_VIEW(byte.arg);
            //     for(PyVar obj: view)
            //         ss << py_str(obj);
            //     STACK_SHRINK(byte.arg);
            //     PUSH(VAR(ss.str()));
            //     DISPATCH();
            // }
            /**************************** */
            case OP_RETURN_VALUE: {
                self->last_retval = byte.arg == BC_NOARG ? POPX() : self->None;
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
            default: PK_UNREACHABLE();
        }

        assert(false);  // should never reach here

    __ERROR:
        // 1. Exception can be handled inside the current frame
        // 2. Exception need to be propagated to the upper frame
        assert(false);
        return RES_ERROR;
    }

    return RES_RETURN;
}
