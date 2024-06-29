#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

int UnboundLocalError(py_Name name) { return -1; }

int NameError(py_Name name) { return -1; }

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
#define TOP() self->stack.sp[-1]
#define SECOND() self->stack.sp[-2]
#define THIRD() self->stack.sp[-3]
#define STACK_SHRINK(n) (self->stack.sp -= n)
#define PUSH(v) (*self->stack.sp++ = v)
#define POP() (--self->stack.sp)
#define POPX() (*--self->stack.sp)
#define SP() (self->stack.sp)

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
                py_TValue tmp = TOP();
                TOP() = SECOND();
                SECOND() = tmp;
                DISPATCH();
            }
            case OP_ROT_THREE: {
                // [a, b, c] -> [c, a, b]
                py_TValue _0 = TOP();
                TOP() = SECOND();
                SECOND() = THIRD();
                THIRD() = _0;
                DISPATCH();
            }
            case OP_PRINT_EXPR:
                if(TOP().type != tp_none_type) {
                    int err = py_repr(&TOP());
                    if(err) goto __ERROR;
                    self->_stdout("%s\n", py_tostr(&TOP()));
                    POP();
                }
                POP();
                DISPATCH();
            /*****************************************/
            case OP_LOAD_CONST:
                PUSH(c11__getitem(py_TValue, &frame->co->consts, byte.arg));
                DISPATCH();
            case OP_LOAD_NONE: PUSH(self->None); DISPATCH();
            case OP_LOAD_TRUE: PUSH(self->True); DISPATCH();
            case OP_LOAD_FALSE: PUSH(self->False); DISPATCH();
            /*****************************************/
            case OP_LOAD_SMALL_INT: py_newint(SP()++, (int64_t)(int16_t)byte.arg); DISPATCH();
            /*****************************************/
            case OP_LOAD_ELLIPSIS: PUSH(self->Ellipsis); DISPATCH();
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
                PUSH(frame->locals[byte.arg]);
                if(py_isnull(&TOP())) {
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
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = Frame__f_closure_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = Frame__f_globals_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_NONLOCAL: {
                py_Name name = byte.arg;
                py_Ref tmp = Frame__f_closure_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = Frame__f_globals_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_GLOBAL: {
                py_Name name = byte.arg;
                py_Ref tmp = Frame__f_globals_try_get(frame, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                tmp = py_getdict(&self->builtins, name);
                if(tmp != NULL) {
                    PUSH(*tmp);
                    DISPATCH();
                }
                NameError(name);
                goto __ERROR;
            }
            case OP_LOAD_ATTR: {
                int err = py_getattr(&TOP(), byte.arg, &TOP());
                if(err) goto __ERROR;
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
