#include "pocketpy/interpreter/ceval.hpp"

namespace pkpy {

#define PREDICT_INT_OP(op)                                                                                             \
    if(is_int(_0) && is_int(_1)) {                                                                                     \
        TOP() = VAR(_0.as<i64>() op _1.as<i64>());                                                                     \
        DISPATCH()                                                                                                     \
    }

#define PREDICT_INT_DIV_OP(op)                                                                                         \
    if(is_int(_0) && is_int(_1)) {                                                                                     \
        i64 divisor = _1.as<i64>();                                                                                    \
        if(divisor == 0) ZeroDivisionError();                                                                          \
        TOP() = VAR(_0.as<i64>() op divisor);                                                                          \
        DISPATCH()                                                                                                     \
    }

#define BINARY_F_COMPARE(func, op, rfunc)                                                                              \
    PyVar ret;                                                                                                         \
    const PyTypeInfo* _ti = _tp_info(_0);                                                                              \
    if(_ti->m##func) {                                                                                                 \
        ret = _ti->m##func(this, _0, _1);                                                                              \
    } else {                                                                                                           \
        PyVar self;                                                                                                    \
        PyVar _2 = get_unbound_method(_0, func, &self, false);                                                         \
        if(_2 != nullptr)                                                                                              \
            ret = call_method(self, _2, _1);                                                                           \
        else                                                                                                           \
            ret = NotImplemented;                                                                                      \
    }                                                                                                                  \
    if(is_not_implemented(ret)) {                                                                                      \
        PyVar self;                                                                                                    \
        PyVar _2 = get_unbound_method(_1, rfunc, &self, false);                                                        \
        if(_2 != nullptr)                                                                                              \
            ret = call_method(self, _2, _0);                                                                           \
        else                                                                                                           \
            BinaryOptError(op, _0, _1);                                                                                \
        if(is_not_implemented(ret)) BinaryOptError(op, _0, _1);                                                        \
    }

void VM::__op_unpack_sequence(uint16_t arg) {
    PyVar _0 = POPX();
    if(is_type(_0, VM::tp_tuple)) {
        // fast path for tuple
        Tuple& tuple = PK_OBJ_GET(Tuple, _0);
        if(tuple.size() == arg) {
            for(PyVar obj: tuple)
                PUSH(obj);
        } else {
            ValueError(_S("expected ", (int)arg, " values to unpack, got ", (int)tuple.size()));
        }
    } else {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        _0 = py_iter(_0);
        const PyTypeInfo* ti = _tp_info(_0);
        for(int i = 0; i < arg; i++) {
            PyVar _1 = _py_next(ti, _0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        if(_py_next(ti, _0) != StopIteration) ValueError("too many values to unpack");
    }
}

bool VM::py_lt(PyVar _0, PyVar _1) {
    BINARY_F_COMPARE(__lt__, "<", __gt__);
    return ret == True;
}

bool VM::py_le(PyVar _0, PyVar _1) {
    BINARY_F_COMPARE(__le__, "<=", __ge__);
    return ret == True;
}

bool VM::py_gt(PyVar _0, PyVar _1) {
    BINARY_F_COMPARE(__gt__, ">", __lt__);
    return ret == True;
}

bool VM::py_ge(PyVar _0, PyVar _1) {
    BINARY_F_COMPARE(__ge__, ">=", __le__);
    return ret == True;
}

#undef BINARY_F_COMPARE

#if PK_ENABLE_PROFILER
#define CEVAL_STEP_CALLBACK()                                                                                          \
    if(_ceval_on_step) _ceval_on_step(this, frame, byte);                                                              \
    if(_profiler) _profiler->_step(callstack.size(), frame);                                                           \
    if(!_next_breakpoint.empty()) { _next_breakpoint._step(this); }
#else
#define CEVAL_STEP_CALLBACK()                                                                                          \
    if(_ceval_on_step && _ceval_on_step) {                                                                             \
        if(_ceval_on_step)                                                                                             \
            if(_ceval_on_step) _ceval_on_step(this, frame, byte);                                                      \
    }
#endif

#define DISPATCH()                                                                                                     \
    {                                                                                                                  \
        frame->_ip++;                                                                                                  \
        goto __NEXT_STEP;                                                                                              \
    }
#define DISPATCH_JUMP(__offset)                                                                                        \
    {                                                                                                                  \
        frame->_ip += __offset;                                                                                        \
        goto __NEXT_STEP;                                                                                              \
    }
#define DISPATCH_JUMP_ABSOLUTE(__target)                                                                               \
    {                                                                                                                  \
        frame->_ip = &frame->co->codes[__target];                                                                      \
        goto __NEXT_STEP;                                                                                              \
    }

PyVar VM::__run_top_frame() {
    Frame* frame = &callstack.top();
    const Frame* base_frame = frame;
    InternalException __internal_exception;

    while(true) {
        try {
            /**********************************************************************/
            {
            __NEXT_FRAME:
                Bytecode byte;

                if(__internal_exception.type == InternalExceptionType::Null) {
                    // None
                    frame->_ip++;
                } else if(__internal_exception.type == InternalExceptionType::Handled) {
                    // HandledException + continue
                    frame->_ip = &frame->co->codes[__internal_exception.arg];
                    __internal_exception = {};
                } else {
                    // UnhandledException + continue (need_raise = true)
                    // ToBeRaisedException + continue (need_raise = true)
                    __internal_exception = {};
                    __raise_exc();  // no return
                }

            __NEXT_STEP:
                byte = *frame->_ip;
                CEVAL_STEP_CALLBACK()

#if PK_DEBUG_CEVAL_STEP
                __log_s_data();
#endif
                switch((Opcode)byte.op) {
                    case OP_NO_OP: DISPATCH()
                    /*****************************************/
                    case OP_POP_TOP: POP(); DISPATCH()
                    case OP_DUP_TOP: PUSH(TOP()); DISPATCH()
                    case OP_DUP_TOP_TWO:
                        // [a, b]
                        PUSH(SECOND());  // [a, b, a]
                        PUSH(SECOND());  // [a, b, a, b]
                        DISPATCH()
                    case OP_ROT_TWO: std::swap(TOP(), SECOND()); DISPATCH()
                    case OP_ROT_THREE: {
                        // [a, b, c] -> [c, a, b]
                        PyVar _0 = TOP();
                        TOP() = SECOND();
                        SECOND() = THIRD();
                        THIRD() = _0;
                    }
                        DISPATCH()
                    case OP_PRINT_EXPR:
                        if(TOP() != None) stdout_write(py_repr(TOP()) + "\n");
                        POP();
                        DISPATCH()
                    /*****************************************/
                    case OP_LOAD_CONST: PUSH(frame->co->consts[byte.arg]); DISPATCH()
                    case OP_LOAD_NONE: PUSH(None); DISPATCH()
                    case OP_LOAD_TRUE: PUSH(True); DISPATCH()
                    case OP_LOAD_FALSE: PUSH(False); DISPATCH()
                    /*****************************************/
                    case OP_LOAD_SMALL_INT: s_data.emplace(tp_int, (i64)(int16_t)byte.arg); DISPATCH()
                    /*****************************************/
                    case OP_LOAD_ELLIPSIS: PUSH(Ellipsis); DISPATCH()
                    case OP_LOAD_FUNCTION: {
                        const FuncDecl_& decl = frame->co->func_decls[byte.arg];
                        PyVar obj;
                        if(decl->nested) {
                            NameDict_ captured = frame->_locals.to_namedict();
                            obj = VAR(Function(decl, frame->_module, nullptr, captured));
                            captured->set(decl->code->name, obj);
                        } else {
                            obj = VAR(Function(decl, frame->_module, nullptr, nullptr));
                        }
                        PUSH(obj);
                    }
                        DISPATCH()
                    case OP_LOAD_NULL: PUSH(PY_NULL); DISPATCH()
                    /*****************************************/
                    case OP_LOAD_FAST: {
                        PyVar _0 = frame->_locals[byte.arg];
                        if(_0 == PY_NULL) vm->UnboundLocalError(frame->co->varnames[byte.arg]);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_LOAD_NAME: {
                        StrName _name(byte.arg);
                        PyVar* slot = frame->_locals.try_get_name(_name);
                        if(slot != nullptr) {
                            if(*slot == PY_NULL) vm->UnboundLocalError(_name);
                            PUSH(*slot);
                            DISPATCH()
                        }
                        PyVar* _0 = frame->f_closure_try_get(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        _0 = frame->f_globals().try_get_2_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        _0 = vm->builtins->attr().try_get_2_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        vm->NameError(_name);
                    }
                        DISPATCH()
                    case OP_LOAD_NONLOCAL: {
                        StrName _name(byte.arg);
                        PyVar* _0 = frame->f_closure_try_get(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        _0 = frame->f_globals().try_get_2_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        _0 = vm->builtins->attr().try_get_2_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(*_0);
                            DISPATCH()
                        }
                        vm->NameError(_name);
                    }
                        DISPATCH()
                    case OP_LOAD_GLOBAL: {
                        StrName _name(byte.arg);
                        PyVar _0 = frame->f_globals().try_get_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(_0);
                            DISPATCH()
                        }
                        _0 = vm->builtins->attr().try_get_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(_0);
                            DISPATCH()
                        }
                        vm->NameError(_name);
                    }
                        DISPATCH()
                    case OP_LOAD_ATTR: {
                        TOP() = getattr(TOP(), StrName(byte.arg));
                    }
                        DISPATCH()
                    case OP_LOAD_CLASS_GLOBAL: {
                        assert(__curr_class != nullptr);
                        StrName _name(byte.arg);
                        PyVar _0 = getattr(__curr_class, _name, false);
                        if(_0 != nullptr) {
                            PUSH(_0);
                            DISPATCH()
                        }
                        // load global if attribute not found
                        _0 = frame->f_globals().try_get_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(_0);
                            DISPATCH()
                        }
                        _0 = vm->builtins->attr().try_get_likely_found(_name);
                        if(_0 != nullptr) {
                            PUSH(_0);
                            DISPATCH()
                        }
                        vm->NameError(_name);
                    }
                        DISPATCH()
                    case OP_LOAD_METHOD: {
                        PyVar _0;
                        TOP() = get_unbound_method(TOP(), StrName(byte.arg), &_0, true, true);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_LOAD_SUBSCR: {
                        PyVar _1 = POPX();  // b
                        PyVar _0 = TOP();   // a
                        auto _ti = _tp_info(_0);
                        if(_ti->m__getitem__) {
                            TOP() = _ti->m__getitem__(this, _0, _1);
                        } else {
                            TOP() = call_method(_0, __getitem__, _1);
                        }
                    }
                        DISPATCH()
                    case OP_LOAD_SUBSCR_FAST: {
                        PyVar _1 = frame->_locals[byte.arg];
                        if(_1 == PY_NULL) vm->UnboundLocalError(frame->co->varnames[byte.arg]);
                        PyVar _0 = TOP();  // a
                        auto _ti = _tp_info(_0);
                        if(_ti->m__getitem__) {
                            TOP() = _ti->m__getitem__(this, _0, _1);
                        } else {
                            TOP() = call_method(_0, __getitem__, _1);
                        }
                    }
                        DISPATCH()
                    case OP_LOAD_SUBSCR_SMALL_INT: {
                        PyVar _1 = VAR((int16_t)byte.arg);
                        PyVar _0 = TOP();  // a
                        auto _ti = _tp_info(_0);
                        if(_ti->m__getitem__) {
                            TOP() = _ti->m__getitem__(this, _0, _1);
                        } else {
                            TOP() = call_method(_0, __getitem__, _1);
                        }
                    }
                        DISPATCH()
                    case OP_STORE_FAST: frame->_locals[byte.arg] = POPX(); DISPATCH()
                    case OP_STORE_NAME: {
                        StrName _name(byte.arg);
                        PyVar _0 = POPX();
                        if(frame->_callable != nullptr) {
                            PyVar* slot = frame->_locals.try_get_name(_name);
                            if(slot != nullptr) {
                                *slot = _0;  // store in locals if possible
                            } else {
                                Function& func = frame->_callable->as<Function>();
                                if(func.decl == __dynamic_func_decl) {
                                    assert(func._closure != nullptr);
                                    func._closure->set(_name, _0);
                                } else {
                                    vm->NameError(_name);
                                }
                            }
                        } else {
                            frame->f_globals().set(_name, _0);
                        }
                    }
                        DISPATCH()
                    case OP_STORE_GLOBAL: frame->f_globals().set(StrName(byte.arg), POPX()); DISPATCH()
                    case OP_STORE_ATTR: {
                        PyVar _0 = TOP();     // a
                        PyVar _1 = SECOND();  // val
                        setattr(_0, StrName(byte.arg), _1);
                        STACK_SHRINK(2);
                    }
                        DISPATCH()
                    case OP_STORE_SUBSCR: {
                        PyVar _2 = POPX();  // b
                        PyVar _1 = POPX();  // a
                        PyVar _0 = POPX();  // val
                        auto _ti = _tp_info(_1);
                        if(_ti->m__setitem__) {
                            _ti->m__setitem__(this, _1, _2, _0);
                        } else {
                            call_method(_1, __setitem__, _2, _0);
                        }
                    }
                        DISPATCH()
                    case OP_STORE_SUBSCR_FAST: {
                        PyVar _2 = frame->_locals[byte.arg];  // b
                        if(_2 == PY_NULL) vm->UnboundLocalError(frame->co->varnames[byte.arg]);
                        PyVar _1 = POPX();  // a
                        PyVar _0 = POPX();  // val
                        auto _ti = _tp_info(_1);
                        if(_ti->m__setitem__) {
                            _ti->m__setitem__(this, _1, _2, _0);
                        } else {
                            call_method(_1, __setitem__, _2, _0);
                        }
                    }
                        DISPATCH()
                    case OP_DELETE_FAST: {
                        PyVar _0 = frame->_locals[byte.arg];
                        if(_0 == PY_NULL) vm->UnboundLocalError(frame->co->varnames[byte.arg]);
                        frame->_locals[byte.arg].set_null();
                    }
                        DISPATCH()
                    case OP_DELETE_NAME: {
                        StrName _name(byte.arg);
                        if(frame->_callable != nullptr) {
                            PyVar* slot = frame->_locals.try_get_name(_name);
                            if(slot != nullptr) {
                                slot->set_null();
                            } else {
                                Function& func = frame->_callable->as<Function>();
                                if(func.decl == __dynamic_func_decl) {
                                    assert(func._closure != nullptr);
                                    bool ok = func._closure->del(_name);
                                    if(!ok) vm->NameError(_name);
                                } else {
                                    vm->NameError(_name);
                                }
                            }
                        } else {
                            if(!frame->f_globals().del(_name)) vm->NameError(_name);
                        }
                    }
                        DISPATCH()
                    case OP_DELETE_GLOBAL: {
                        StrName _name(byte.arg);
                        if(!frame->f_globals().del(_name)) vm->NameError(_name);
                    }
                        DISPATCH()
                    case OP_DELETE_ATTR: {
                        PyVar _0 = POPX();
                        delattr(_0, StrName(byte.arg));
                    }
                        DISPATCH()
                    case OP_DELETE_SUBSCR: {
                        PyVar _1 = POPX();
                        PyVar _0 = POPX();
                        auto _ti = _tp_info(_0);
                        if(_ti->m__delitem__) {
                            _ti->m__delitem__(this, _0, _1);
                        } else {
                            call_method(_0, __delitem__, _1);
                        }
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_BUILD_LONG: {
                        PyVar _0 = builtins->attr().try_get_likely_found(pk_id_long);
                        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
                        TOP() = call(_0, TOP());
                    }
                        DISPATCH()
                    case OP_BUILD_IMAG: {
                        PyVar _0 = builtins->attr().try_get_likely_found(pk_id_complex);
                        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
                        TOP() = call(_0, VAR(0), TOP());
                    }
                        DISPATCH()
                    case OP_BUILD_BYTES: {
                        const Str& s = CAST(Str&, TOP());
                        unsigned char* p = (unsigned char*)std::malloc(s.size);
                        std::memcpy(p, s.c_str(), s.size);
                        TOP() = VAR(Bytes(p, s.size));
                    }
                        DISPATCH()
                    case OP_BUILD_TUPLE: {
                        PyVar _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
                        STACK_SHRINK(byte.arg);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_LIST: {
                        PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
                        STACK_SHRINK(byte.arg);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_DICT: {
                        if(byte.arg == 0) {
                            PUSH(VAR(Dict()));
                            DISPATCH()
                        }
                        PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
                        _0 = call(_t(tp_dict), _0);
                        STACK_SHRINK(byte.arg);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_SET: {
                        PyVar _0 = VAR(STACK_VIEW(byte.arg).to_list());
                        _0 = call(builtins->attr(pk_id_set), _0);
                        STACK_SHRINK(byte.arg);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_SLICE: {
                        PyVar _2 = POPX();  // step
                        PyVar _1 = POPX();  // stop
                        PyVar _0 = POPX();  // start
                        PUSH(VAR(Slice(_0, _1, _2)));
                    }
                        DISPATCH()
                    case OP_BUILD_STRING: {
                        SStream ss;
                        ArgsView view = STACK_VIEW(byte.arg);
                        for(PyVar obj: view)
                            ss << py_str(obj);
                        STACK_SHRINK(byte.arg);
                        PUSH(VAR(ss.str()));
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_BUILD_TUPLE_UNPACK: {
                        List list;
                        __unpack_as_list(STACK_VIEW(byte.arg), list);
                        STACK_SHRINK(byte.arg);
                        PyVar _0 = VAR(list.to_tuple());
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_LIST_UNPACK: {
                        List list;
                        __unpack_as_list(STACK_VIEW(byte.arg), list);
                        STACK_SHRINK(byte.arg);
                        PyVar _0 = VAR(std::move(list));
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_DICT_UNPACK: {
                        Dict dict;
                        __unpack_as_dict(STACK_VIEW(byte.arg), dict);
                        STACK_SHRINK(byte.arg);
                        PyVar _0 = VAR(std::move(dict));
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_BUILD_SET_UNPACK: {
                        List list;
                        __unpack_as_list(STACK_VIEW(byte.arg), list);
                        STACK_SHRINK(byte.arg);
                        PyVar _0 = VAR(std::move(list));
                        _0 = call(builtins->attr(pk_id_set), _0);
                        PUSH(_0);
                    }
                        DISPATCH()
                        /*****************************************/
#define BINARY_OP_SPECIAL(func)                                                                                        \
    _ti = _tp_info(_0);                                                                                                \
    if(_ti->m##func) {                                                                                                 \
        TOP() = _ti->m##func(this, _0, _1);                                                                            \
    } else {                                                                                                           \
        PyVar self;                                                                                                    \
        PyVar _2 = get_unbound_method(_0, func, &self, false);                                                         \
        if(_2 != nullptr)                                                                                              \
            TOP() = call_method(self, _2, _1);                                                                         \
        else                                                                                                           \
            TOP() = NotImplemented;                                                                                    \
    }

#define BINARY_OP_RSPECIAL(op, func)                                                                                   \
    if(is_not_implemented(TOP())) {                                                                                    \
        PyVar self;                                                                                                    \
        PyVar _2 = get_unbound_method(_1, func, &self, false);                                                         \
        if(_2 != nullptr)                                                                                              \
            TOP() = call_method(self, _2, _0);                                                                         \
        else                                                                                                           \
            BinaryOptError(op, _0, _1);                                                                                \
        if(is_not_implemented(TOP())) BinaryOptError(op, _0, _1);                                                      \
    }

                    case OP_BINARY_TRUEDIV: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__truediv__);
                        if(is_not_implemented(TOP())) BinaryOptError("/", _0, _1);
                    }
                        DISPATCH()
                    case OP_BINARY_POW: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__pow__);
                        if(is_not_implemented(TOP())) BinaryOptError("**", _0, _1);
                    }
                        DISPATCH()
                    case OP_BINARY_ADD: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(+)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__add__);
                        BINARY_OP_RSPECIAL("+", __radd__);
                    }
                        DISPATCH()
                    case OP_BINARY_SUB: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(-)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__sub__);
                        BINARY_OP_RSPECIAL("-", __rsub__);
                    }
                        DISPATCH()
                    case OP_BINARY_MUL: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(*)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__mul__);
                        BINARY_OP_RSPECIAL("*", __rmul__);
                    }
                        DISPATCH()
                    case OP_BINARY_FLOORDIV: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_DIV_OP(/)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__floordiv__);
                        if(is_not_implemented(TOP())) BinaryOptError("//", _0, _1);
                    }
                        DISPATCH()
                    case OP_BINARY_MOD: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_DIV_OP(%)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__mod__);
                        if(is_not_implemented(TOP())) BinaryOptError("%", _0, _1);
                    }
                        DISPATCH()
                    case OP_COMPARE_LT: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(<)
                        TOP() = VAR(py_lt(_0, _1));
                    }
                        DISPATCH()
                    case OP_COMPARE_LE: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(<=)
                        TOP() = VAR(py_le(_0, _1));
                    }
                        DISPATCH()
                    case OP_COMPARE_EQ: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        TOP() = VAR(py_eq(_0, _1));
                    }
                        DISPATCH()
                    case OP_COMPARE_NE: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        TOP() = VAR(py_ne(_0, _1));
                    }
                        DISPATCH()
                    case OP_COMPARE_GT: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(>)
                        TOP() = VAR(py_gt(_0, _1));
                    }
                        DISPATCH()
                    case OP_COMPARE_GE: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(>=)
                        TOP() = VAR(py_ge(_0, _1));
                    }
                        DISPATCH()
                    case OP_BITWISE_LSHIFT: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(<<)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__lshift__);
                        if(is_not_implemented(TOP())) BinaryOptError("<<", _0, _1);
                    }
                        DISPATCH()
                    case OP_BITWISE_RSHIFT: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(>>)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__rshift__);
                        if(is_not_implemented(TOP())) BinaryOptError(">>", _0, _1);
                    }
                        DISPATCH()
                    case OP_BITWISE_AND: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(&)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__and__);
                        if(is_not_implemented(TOP())) BinaryOptError("&", _0, _1);
                    }
                        DISPATCH()
                    case OP_BITWISE_OR: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(|)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__or__);
                        if(is_not_implemented(TOP())) BinaryOptError("|", _0, _1);
                    }
                        DISPATCH()
                    case OP_BITWISE_XOR: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        PREDICT_INT_OP(^)
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__xor__);
                        if(is_not_implemented(TOP())) BinaryOptError("^", _0, _1);
                    }
                        DISPATCH()
                    case OP_BINARY_MATMUL: {
                        PyVar _1 = POPX();
                        PyVar _0 = TOP();
                        const PyTypeInfo* _ti;
                        BINARY_OP_SPECIAL(__matmul__);
                        if(is_not_implemented(TOP())) BinaryOptError("@", _0, _1);
                    }
                        DISPATCH()

#undef BINARY_OP_SPECIAL
#undef BINARY_OP_RSPECIAL
#undef PREDICT_INT_OP

                    case OP_IS_OP: {
                        PyVar _1 = POPX();  // rhs
                        PyVar _0 = TOP();   // lhs
                        TOP() = _0 == _1 ? True : False;
                    }
                        DISPATCH()
                    case OP_IS_NOT_OP: {
                        PyVar _1 = POPX();  // rhs
                        PyVar _0 = TOP();   // lhs
                        TOP() = _0 != _1 ? True : False;
                    }
                        DISPATCH()
                    case OP_CONTAINS_OP: {
                        // a in b -> b __contains__ a
                        auto _ti = _tp_info(TOP());
                        PyVar _0;
                        if(_ti->m__contains__) {
                            _0 = _ti->m__contains__(this, TOP(), SECOND());
                        } else {
                            _0 = call_method(TOP(), __contains__, SECOND());
                        }
                        POP();
                        TOP() = VAR(static_cast<bool>((int)CAST(bool, _0) ^ byte.arg));
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_JUMP_FORWARD: DISPATCH_JUMP((int16_t)byte.arg)
                    case OP_POP_JUMP_IF_FALSE:
                        if(!py_bool(POPX())) DISPATCH_JUMP((int16_t)byte.arg)
                        DISPATCH()
                    case OP_POP_JUMP_IF_TRUE:
                        if(py_bool(POPX())) DISPATCH_JUMP((int16_t)byte.arg)
                        DISPATCH()
                    case OP_JUMP_IF_TRUE_OR_POP:
                        if(py_bool(TOP())) {
                            DISPATCH_JUMP((int16_t)byte.arg)
                        } else {
                            POP();
                            DISPATCH()
                        }
                    case OP_JUMP_IF_FALSE_OR_POP:
                        if(!py_bool(TOP())) {
                            DISPATCH_JUMP((int16_t)byte.arg)
                        } else {
                            POP();
                            DISPATCH()
                        }
                    case OP_SHORTCUT_IF_FALSE_OR_POP:
                        if(!py_bool(TOP())) {  // [b, False]
                            STACK_SHRINK(2);   // []
                            PUSH(vm->False);   // [False]
                            DISPATCH_JUMP((int16_t)byte.arg)
                        } else {
                            POP();  // [b]
                            DISPATCH()
                        }
                    case OP_LOOP_CONTINUE:
                        // just an alias of OP_JUMP_FORWARD
                        DISPATCH_JUMP((int16_t)byte.arg)
                    case OP_LOOP_BREAK: {
                        frame->prepare_jump_break(&s_data, frame->ip() + byte.arg);
                        DISPATCH_JUMP((int16_t)byte.arg)
                    }
                    case OP_JUMP_ABSOLUTE_TOP: DISPATCH_JUMP_ABSOLUTE(_CAST(int, POPX()))
                    case OP_GOTO: {
                        StrName _name(byte.arg);
                        int target = frame->co->labels.get(_name, -1);
                        if(target < 0) RuntimeError(_S("label ", _name.escape(), " not found"));
                        frame->prepare_jump_break(&s_data, target);
                        DISPATCH_JUMP_ABSOLUTE(target)
                    }
                    /*****************************************/
                    case OP_FSTRING_EVAL: {
                        PyVar _0 = frame->co->consts[byte.arg];
                        std::string_view string = CAST(Str&, _0).sv();
                        auto it = __cached_codes.try_get(string);
                        CodeObject_ code;
                        if(it == nullptr) {
                            code = vm->compile(string, "<eval>", EVAL_MODE, true);
                            __cached_codes.insert(string, code);
                        } else {
                            code = *it;
                        }
                        _0 = vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_REPR: TOP() = VAR(py_repr(TOP())); DISPATCH()
                    case OP_CALL: {
                        if(heap._should_auto_collect()) heap._auto_collect();
                        PyVar _0 = vectorcall(byte.arg & 0xFF,         // ARGC
                                              (byte.arg >> 8) & 0xFF,  // KWARGC
                                              true);
                        if(_0 == PY_OP_CALL) {
                            frame = &callstack.top();
                            goto __NEXT_FRAME;
                        }
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_CALL_TP: {
                        if(heap._should_auto_collect()) heap._auto_collect();
                        PyVar _0;
                        PyVar _1;
                        PyVar _2;
                        // [callable, <self>, args: tuple, kwargs: dict | NULL]
                        if(byte.arg) {
                            _2 = POPX();
                            _1 = POPX();
                            for(PyVar obj: _CAST(Tuple&, _1))
                                PUSH(obj);
                            _CAST(Dict&, _2).apply([this](PyVar k, PyVar v) {
                                PUSH(VAR(StrName(CAST(Str&, k)).index));
                                PUSH(v);
                            });
                            _0 = vectorcall(_CAST(Tuple&, _1).size(),  // ARGC
                                            _CAST(Dict&, _2).size(),   // KWARGC
                                            true);
                        } else {
                            // no **kwargs
                            _1 = POPX();
                            for(PyVar obj: _CAST(Tuple&, _1))
                                PUSH(obj);
                            _0 = vectorcall(_CAST(Tuple&, _1).size(),  // ARGC
                                            0,                         // KWARGC
                                            true);
                        }
                        if(_0 == PY_OP_CALL) {
                            frame = &callstack.top();
                            goto __NEXT_FRAME;
                        }
                        PUSH(_0);
                    }
                        DISPATCH()
                    case OP_RETURN_VALUE: {
                        PyVar _0 = byte.arg == BC_NOARG ? POPX() : None;
                        __pop_frame();
                        if(frame == base_frame) {  // [ frameBase<- ]
                            return _0;
                        } else {
                            frame = &callstack.top();
                            PUSH(_0);
                            goto __NEXT_FRAME;
                        }
                    }
                        DISPATCH()
                    case OP_YIELD_VALUE: return PY_OP_YIELD;
                    /*****************************************/
                    case OP_LIST_APPEND: {
                        PyVar _0 = POPX();
                        PK_OBJ_GET(List, SECOND()).push_back(_0);
                    }
                        DISPATCH()
                    case OP_DICT_ADD: {
                        PyVar _0 = POPX();
                        const Tuple& t = PK_OBJ_GET(Tuple, _0);
                        PK_OBJ_GET(Dict, SECOND()).set(this, t[0], t[1]);
                    }
                        DISPATCH()
                    case OP_SET_ADD: {
                        PyVar _0 = POPX();
                        call_method(SECOND(), pk_id_add, _0);
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_UNARY_NEGATIVE: TOP() = py_negate(TOP()); DISPATCH()
                    case OP_UNARY_NOT: TOP() = VAR(!py_bool(TOP())); DISPATCH()  
                    case OP_UNARY_STAR: TOP() = VAR(StarWrapper(byte.arg, TOP())); DISPATCH()
                    case OP_UNARY_INVERT: {
                        PyVar _0;
                        auto _ti = _tp_info(TOP());
                        if(_ti->m__invert__)
                            _0 = _ti->m__invert__(this, TOP());
                        else
                            _0 = call_method(TOP(), __invert__);
                        TOP() = _0;
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_GET_ITER: TOP() = py_iter(TOP()); DISPATCH()
                    case OP_GET_ITER_NEW: TOP() = py_iter(TOP()); DISPATCH()
                    case OP_FOR_ITER: {
                        PyVar _0 = py_next(TOP());
                        if(_0 == StopIteration) {
                            int target = frame->prepare_loop_break(&s_data);
                            DISPATCH_JUMP_ABSOLUTE(target)
                        } else {
                            PUSH(_0);
                            DISPATCH()
                        }
                    }
                    case OP_FOR_ITER_STORE_FAST: {
                        PyVar _0 = py_next(TOP());
                        if(_0 == StopIteration) {
                            int target = frame->prepare_loop_break(&s_data);
                            DISPATCH_JUMP_ABSOLUTE(target)
                        } else {
                            frame->_locals[byte.arg] = _0;
                            DISPATCH()
                        }
                    }
                    case OP_FOR_ITER_STORE_GLOBAL: {
                        PyVar _0 = py_next(TOP());
                        if(_0 == StopIteration) {
                            int target = frame->prepare_loop_break(&s_data);
                            DISPATCH_JUMP_ABSOLUTE(target)
                        } else {
                            frame->f_globals().set(StrName(byte.arg), _0);
                            DISPATCH()
                        }
                    }
                    case OP_FOR_ITER_YIELD_VALUE: {
                        PyVar _0 = py_next(TOP());
                        if(_0 == StopIteration) {
                            int target = frame->prepare_loop_break(&s_data);
                            DISPATCH_JUMP_ABSOLUTE(target)
                        } else {
                            PUSH(_0);
                            return PY_OP_YIELD;
                        }
                    }
                    case OP_FOR_ITER_UNPACK: {
                        PyVar _0 = TOP();
                        const PyTypeInfo* _ti = _tp_info(_0);
                        if(_ti->op__next__) {
                            unsigned n = _ti->op__next__(this, _0);
                            if(n == 0) {
                                // StopIteration
                                int target = frame->prepare_loop_break(&s_data);
                                DISPATCH_JUMP_ABSOLUTE(target)
                            } else if(n == 1) {
                                // UNPACK_SEQUENCE
                                __op_unpack_sequence(byte.arg);
                            } else {
                                if(n != byte.arg) {
                                    ValueError(_S("expected ", (int)byte.arg, " values to unpack, got ", (int)n));
                                }
                            }
                        } else {
                            // FOR_ITER
                            _0 = call_method(_0, __next__);
                            if(_0 != StopIteration) {
                                PUSH(_0);
                                // UNPACK_SEQUENCE
                                __op_unpack_sequence(byte.arg);
                            } else {
                                int target = frame->prepare_loop_break(&s_data);
                                DISPATCH_JUMP_ABSOLUTE(target)
                            }
                        }
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_IMPORT_PATH: {
                        PyVar _0 = frame->co->consts[byte.arg];
                        PUSH(py_import(CAST(Str&, _0)));
                    }
                        DISPATCH()
                    case OP_POP_IMPORT_STAR: {
                        PyVar _0 = POPX();  // pop the module
                        PyVar _1 = _0->attr().try_get(__all__);
                        StrName _name;
                        if(_1 != nullptr) {
                            for(PyVar key: CAST(List&, _1)) {
                                _name = StrName::get(CAST(Str&, key).sv());
                                PyVar value = _0->attr().try_get_likely_found(_name);
                                if(value == nullptr) {
                                    ImportError(_S("cannot import name ", _name.escape()));
                                } else {
                                    frame->f_globals().set(_name, value);
                                }
                            }
                        } else {
                            for(auto& [name, value]: _0->attr().items()) {
                                std::string_view s = name.sv();
                                if(s.empty() || s[0] == '_') continue;
                                frame->f_globals().set(name, value);
                            }
                        }
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_UNPACK_SEQUENCE: {
                        __op_unpack_sequence(byte.arg);
                    }
                        DISPATCH()
                    case OP_UNPACK_EX: {
                        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
                        PyVar _0 = py_iter(POPX());
                        const PyTypeInfo* _ti = _tp_info(_0);
                        PyVar _1;
                        for(int i = 0; i < byte.arg; i++) {
                            _1 = _py_next(_ti, _0);
                            if(_1 == StopIteration) ValueError("not enough values to unpack");
                            PUSH(_1);
                        }
                        List extras;
                        while(true) {
                            _1 = _py_next(_ti, _0);
                            if(_1 == StopIteration) break;
                            extras.push_back(_1);
                        }
                        PUSH(VAR(std::move(extras)));
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_BEGIN_CLASS: {
                        StrName _name(byte.arg);
                        PyVar _0 = POPX();  // super
                        if(is_none(_0)) _0 = _t(tp_object);
                        check_type(_0, tp_type);
                        __curr_class = new_type_object(frame->_module, _name, PK_OBJ_GET(Type, _0), true);
                    }
                        DISPATCH()
                    case OP_END_CLASS: {
                        assert(__curr_class != nullptr);
                        StrName _name(byte.arg);
                        frame->_module->attr().set(_name, __curr_class);
                        // call on_end_subclass
                        PyTypeInfo* ti = &_all_types[__curr_class->as<Type>()];
                        if(ti->base != tp_object) {
                            PyTypeInfo* base_ti = &_all_types[ti->base];
                            if(base_ti->on_end_subclass) base_ti->on_end_subclass(this, ti);
                        }
                        __curr_class = nullptr;
                    }
                        DISPATCH()
                    case OP_STORE_CLASS_ATTR: {
                        assert(__curr_class != nullptr);
                        StrName _name(byte.arg);
                        PyVar _0 = POPX();
                        if(is_type(_0, tp_function)) { PK_OBJ_GET(Function, _0)._class = __curr_class; }
                        __curr_class->attr().set(_name, _0);
                    }
                        DISPATCH()
                    case OP_BEGIN_CLASS_DECORATION: {
                        PUSH(__curr_class);
                    }
                        DISPATCH()
                    case OP_END_CLASS_DECORATION: {
                        __curr_class = POPX().get();
                    }
                        DISPATCH()
                    case OP_ADD_CLASS_ANNOTATION: {
                        assert(__curr_class != nullptr);
                        StrName _name(byte.arg);
                        Type type = __curr_class->as<Type>();
                        _all_types[type].annotated_fields.push_back(_name);
                    }
                        DISPATCH()
                    /*****************************************/
                    case OP_WITH_ENTER: PUSH(call_method(TOP(), __enter__)); DISPATCH()
                    case OP_WITH_EXIT:
                        call_method(TOP(), __exit__);
                        POP();
                        DISPATCH()
                    /*****************************************/
                    case OP_TRY_ENTER: {
                        frame->set_unwind_target(s_data._sp);
                        DISPATCH()
                    }
                    case OP_EXCEPTION_MATCH: {
                        PyVar assumed_type = POPX();
                        check_type(assumed_type, tp_type);
                        PyVar e_obj = TOP();
                        bool ok = isinstance(e_obj, PK_OBJ_GET(Type, assumed_type));
                        PUSH(VAR(ok));
                    }
                        DISPATCH()
                    case OP_RAISE: {
                        if(is_type(TOP(), tp_type)) { TOP() = call(TOP()); }
                        if(!isinstance(TOP(), tp_exception)) { TypeError("exceptions must derive from Exception"); }
                        _error(POPX());
                    }
                        DISPATCH()
                    case OP_RAISE_ASSERT:
                        if(byte.arg) {
                            Str msg = py_str(TOP());
                            POP();
                            AssertionError(msg);
                        } else {
                            AssertionError();
                        }
                        DISPATCH()
                    case OP_RE_RAISE: __raise_exc(true); DISPATCH()
                    case OP_POP_EXCEPTION: __last_exception = POPX().get(); DISPATCH()
                    /*****************************************/
                    case OP_FORMAT_STRING: {
                        PyVar _0 = POPX();
                        const Str& spec = CAST(Str&, frame->co->consts[byte.arg]);
                        PUSH(__format_object(_0, spec));
                    }
                        DISPATCH()
                    /*****************************************/
                    default: PK_UNREACHABLE()
                }
            }
            /**********************************************************************/
            PK_UNREACHABLE()
        } catch(InternalException internal) {
            __internal_exception = internal;
            if(internal.type == InternalExceptionType::Unhandled) {
                __last_exception = POPX().get();
                Exception& _e = __last_exception->as<Exception>();
                bool is_base_frame_to_be_popped = frame == base_frame;
                __pop_frame();
                if(callstack.empty()) {
                    // propagate to the top level
                    throw TopLevelException(this, &_e);
                }
                frame = &callstack.top();
                PUSH(__last_exception);
                if(is_base_frame_to_be_popped) { throw InternalException(InternalExceptionType::ToBeRaised); }
            }
        }
    }
}

#undef TOP
#undef SECOND
#undef THIRD
#undef STACK_SHRINK
#undef PUSH
#undef POP
#undef POPX
#undef STACK_VIEW

#undef DISPATCH
#undef DISPATCH_JUMP
#undef CEVAL_STEP_CALLBACK

}  // namespace pkpy
