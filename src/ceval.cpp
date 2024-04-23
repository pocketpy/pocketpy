#include "pocketpy/ceval.h"

namespace pkpy{

#define PREDICT_INT_OP(op)  \
    if(is_small_int(_0) && is_small_int(_1)){   \
        TOP() = VAR((PK_BITS(_0)>>2) op (PK_BITS(_1)>>2)); \
        DISPATCH() \
    }

#define PREDICT_INT_DIV_OP(op)  \
    if(is_small_int(_0) && is_small_int(_1)){   \
        if(_1 == PK_SMALL_INT(0)) ZeroDivisionError();   \
        TOP() = VAR((PK_BITS(_0)>>2) op (PK_BITS(_1)>>2)); \
        DISPATCH() \
    }

#define BINARY_F_COMPARE(func, op, rfunc)                           \
        PyObject* ret;                                              \
        const PyTypeInfo* _ti = _inst_type_info(_0);                \
        if(_ti->m##func){                               \
            ret = _ti->m##func(this, _0, _1);           \
        }else{                                          \
            PyObject* self;                                                     \
            PyObject* _2 = get_unbound_method(_0, func, &self, false);          \
            if(_2 != nullptr) ret = call_method(self, _2, _1);                  \
            else ret = NotImplemented;                                          \
        }                                                                       \
        if(ret == NotImplemented){                                              \
            PyObject* self;                                                     \
            PyObject* _2 = get_unbound_method(_1, rfunc, &self, false);         \
            if(_2 != nullptr) ret = call_method(self, _2, _0);                  \
            else BinaryOptError(op, _0, _1);                                    \
            if(ret == NotImplemented) BinaryOptError(op, _0, _1);               \
        }


bool VM::py_lt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__lt__, "<", __gt__);
    return ret == True;
}

bool VM::py_le(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__le__, "<=", __ge__);
    return ret == True;
}

bool VM::py_gt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__gt__, ">", __lt__);
    return ret == True;
}

bool VM::py_ge(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__ge__, ">=", __le__);
    return ret == True;
}

#undef BINARY_F_COMPARE

PyObject* VM::_run_top_frame(){
    Frame* frame = top_frame();
    const Frame* base_frame = frame;
    bool need_raise = false;

    while(true){
        try{
            if(need_raise){ need_raise = false; _raise(); }
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 */
{

#define CEVAL_STEP_CALLBACK() \
    if(_ceval_on_step) _ceval_on_step(this, frame, byte);   \
    if(_profiler) _profiler->_step(callstack.size(), frame);        \
    if(!_next_breakpoint.empty()) { _next_breakpoint._step(this); }

#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    // cache
    const CodeObject* co = frame->co;
    const Bytecode* co_codes = co->codes.data();

    Bytecode byte = co_codes[frame->next_bytecode()];
    CEVAL_STEP_CALLBACK();

#define TARGET(op) case OP_##op:
#define DISPATCH() { byte = co_codes[frame->next_bytecode()]; CEVAL_STEP_CALLBACK(); goto __NEXT_STEP;}

__NEXT_STEP:;
#if PK_DEBUG_CEVAL_STEP
    _log_s_data();
#endif
    switch ((Opcode)byte.op)
    {
    TARGET(NO_OP) DISPATCH();
    /*****************************************/
    TARGET(POP_TOP) POP(); DISPATCH();
    TARGET(DUP_TOP) PUSH(TOP()); DISPATCH();
    TARGET(ROT_TWO) std::swap(TOP(), SECOND()); DISPATCH();
    TARGET(ROT_THREE){
        PyObject* _0 = TOP();
        TOP() = SECOND();
        SECOND() = THIRD();
        THIRD() = _0;
    } DISPATCH();
    TARGET(PRINT_EXPR){
        if(TOP() != None) stdout_write(CAST(Str&, py_repr(TOP())) + "\n");
        POP();
    } DISPATCH();
    /*****************************************/
    TARGET(LOAD_CONST)
        if(heap._should_auto_collect()) heap._auto_collect();
        PUSH(co->consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE)       PUSH(None); DISPATCH();
    TARGET(LOAD_TRUE)       PUSH(True); DISPATCH();
    TARGET(LOAD_FALSE)      PUSH(False); DISPATCH();
    /*****************************************/
    TARGET(LOAD_INT_0)      PUSH(PK_SMALL_INT(0)); DISPATCH();
    TARGET(LOAD_INT_1)      PUSH(PK_SMALL_INT(1)); DISPATCH();
    TARGET(LOAD_INT_2)      PUSH(PK_SMALL_INT(2)); DISPATCH();
    TARGET(LOAD_INT_3)      PUSH(PK_SMALL_INT(3)); DISPATCH();
    TARGET(LOAD_INT_4)      PUSH(PK_SMALL_INT(4)); DISPATCH();
    TARGET(LOAD_INT_5)      PUSH(PK_SMALL_INT(5)); DISPATCH();
    TARGET(LOAD_INT_6)      PUSH(PK_SMALL_INT(6)); DISPATCH();
    TARGET(LOAD_INT_7)      PUSH(PK_SMALL_INT(7)); DISPATCH();
    TARGET(LOAD_INT_8)      PUSH(PK_SMALL_INT(8)); DISPATCH();
    TARGET(LOAD_INT_9)      PUSH(PK_SMALL_INT(9)); DISPATCH();
    TARGET(LOAD_INT_10)     PUSH(PK_SMALL_INT(10)); DISPATCH();
    TARGET(LOAD_INT_11)     PUSH(PK_SMALL_INT(11)); DISPATCH();
    TARGET(LOAD_INT_12)     PUSH(PK_SMALL_INT(12)); DISPATCH();
    TARGET(LOAD_INT_13)     PUSH(PK_SMALL_INT(13)); DISPATCH();
    TARGET(LOAD_INT_14)     PUSH(PK_SMALL_INT(14)); DISPATCH();
    TARGET(LOAD_INT_15)     PUSH(PK_SMALL_INT(15)); DISPATCH();
    TARGET(LOAD_INT_16)     PUSH(PK_SMALL_INT(16)); DISPATCH();
    /*****************************************/
    TARGET(LOAD_ELLIPSIS)   PUSH(Ellipsis); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        const FuncDecl_& decl = co->func_decls[byte.arg];
        PyObject* obj;
        if(decl->nested){
            NameDict_ captured = frame->_locals.to_namedict();
            obj = VAR(Function(decl, frame->_module, nullptr, captured));
            captured->set(decl->code->name, obj);
        }else{
            obj = VAR(Function(decl, frame->_module, nullptr, nullptr));
        }
        PUSH(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) PUSH(PY_NULL); DISPATCH();
    /*****************************************/
    TARGET(LOAD_FAST) {
        if(heap._should_auto_collect()) heap._auto_collect();
        PyObject* _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->UnboundLocalError(co->varnames[byte.arg]);
        PUSH(_0);
    } DISPATCH();
    TARGET(LOAD_NAME) {
        StrName _name(byte.arg);
        PyObject** slot = frame->_locals.try_get_name(_name);
        if(slot != nullptr) {
            if(*slot == PY_NULL) vm->UnboundLocalError(_name);
            PUSH(*slot);
            DISPATCH();
        }
        PyObject* _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_NONLOCAL) {
        if(heap._should_auto_collect()) heap._auto_collect();
        StrName _name(byte.arg);
        PyObject* _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_GLOBAL){
        if(heap._should_auto_collect()) heap._auto_collect();
        StrName _name(byte.arg);
        PyObject* _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_ATTR){
        TOP() = getattr(TOP(), StrName(byte.arg));
    } DISPATCH();
    TARGET(LOAD_CLASS_GLOBAL){
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        PyObject* _0 = getattr(_curr_class, _name, false);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        // load global if attribute not found
        _0 = frame->f_globals().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get_likely_found(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
    } DISPATCH();
    TARGET(LOAD_METHOD){
        PyObject* _0;
        TOP() = get_unbound_method(TOP(), StrName(byte.arg), &_0, true, true);
        PUSH(_0);
    }DISPATCH();
    TARGET(LOAD_SUBSCR){
        PyObject* _1 = POPX();    // b
        PyObject* _0 = TOP();     // a
        auto _ti = _inst_type_info(_0);
        if(_ti->m__getitem__){
            TOP() = _ti->m__getitem__(this, _0, _1);
        }else{
            TOP() = call_method(_0, __getitem__, _1);
        }
    } DISPATCH();
    TARGET(STORE_FAST)
        frame->_locals[byte.arg] = POPX();
        DISPATCH();
    TARGET(STORE_NAME){
        StrName _name(byte.arg);
        PyObject* _0 = POPX();
        if(frame->_callable != nullptr){
            PyObject** slot = frame->_locals.try_get_name(_name);
            if(slot == nullptr) vm->UnboundLocalError(_name);
            *slot = _0;
        }else{
            frame->f_globals().set(_name, _0);
        }
    } DISPATCH();
    TARGET(STORE_GLOBAL)
        frame->f_globals().set(StrName(byte.arg), POPX());
        DISPATCH();
    TARGET(STORE_ATTR) {
        PyObject* _0 = TOP();         // a
        PyObject* _1 = SECOND();      // val
        setattr(_0, StrName(byte.arg), _1);
        STACK_SHRINK(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR){
        PyObject* _2 = POPX();        // b
        PyObject* _1 = POPX();        // a
        PyObject* _0 = POPX();        // val
        auto _ti = _inst_type_info(_1);
        if(_ti->m__setitem__){
            _ti->m__setitem__(this, _1, _2, _0);
        }else{
            call_method(_1, __setitem__, _2, _0);
        }
    }DISPATCH();
    TARGET(DELETE_FAST){
        PyObject* _0 = frame->_locals[byte.arg];
        if(_0 == PY_NULL) vm->UnboundLocalError(co->varnames[byte.arg]);
        frame->_locals[byte.arg] = PY_NULL;
    }DISPATCH();
    TARGET(DELETE_NAME){
        StrName _name(byte.arg);
        if(frame->_callable != nullptr){
            PyObject** slot = frame->_locals.try_get_name(_name);
            if(slot == nullptr) vm->UnboundLocalError(_name);
            *slot = PY_NULL;
        }else{
            if(!frame->f_globals().del(_name)) vm->NameError(_name);
        }
    } DISPATCH();
    TARGET(DELETE_GLOBAL){
        StrName _name(byte.arg);
        if(!frame->f_globals().del(_name)) vm->NameError(_name);
    }DISPATCH();
    TARGET(DELETE_ATTR){
        PyObject* _0 = POPX();
        delattr(_0, StrName(byte.arg));
    } DISPATCH();
    TARGET(DELETE_SUBSCR){
        PyObject* _1 = POPX();
        PyObject* _0 = POPX();
        auto _ti = _inst_type_info(_0);
        if(_ti->m__delitem__){
            _ti->m__delitem__(this, _0, _1);
        }else{
            call_method(_0, __delitem__, _1);
        }
    }DISPATCH();
    /*****************************************/
    TARGET(BUILD_LONG) {
        PyObject* _0 = builtins->attr().try_get_likely_found(pk_id_long);
        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
        TOP() = call(_0, TOP());
    } DISPATCH();
    TARGET(BUILD_IMAG) {
        PyObject* _0 = builtins->attr().try_get_likely_found(pk_id_complex);
        if(_0 == nullptr) AttributeError(builtins, pk_id_long);
        TOP() = call(_0, VAR(0), TOP());
    } DISPATCH();
    TARGET(BUILD_BYTES) {
        const Str& s = CAST(Str&, TOP());
        unsigned char* p = new unsigned char[s.size];
        memcpy(p, s.data, s.size);
        TOP() = VAR(Bytes(p, s.size));
    } DISPATCH();
    TARGET(BUILD_TUPLE){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_LIST){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_DICT){
        if(byte.arg == 0){
            PUSH(VAR(Dict(this)));
            DISPATCH();
        }
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(_t(tp_dict), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SET){
        PyObject* _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(builtins->attr(pk_id_set), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SLICE){
        PyObject* _2 = POPX();    // step
        PyObject* _1 = POPX();    // stop
        PyObject* _0 = POPX();    // start
        PUSH(VAR(Slice(_0, _1, _2)));
    } DISPATCH();
    TARGET(BUILD_STRING) {
        SStream ss;
        ArgsView view = STACK_VIEW(byte.arg);
        for(PyObject* obj : view) ss << CAST(Str&, py_str(obj));
        STACK_SHRINK(byte.arg);
        PUSH(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
    TARGET(BUILD_TUPLE_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(Tuple(std::move(list)));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_LIST_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(list));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_DICT_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        Dict dict(this);
        _unpack_as_dict(STACK_VIEW(byte.arg), dict);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(dict));
        PUSH(_0);
    } DISPATCH();
    TARGET(BUILD_SET_UNPACK) {
        auto _lock = heap.gc_scope_lock();
        List list;
        _unpack_as_list(STACK_VIEW(byte.arg), list);
        STACK_SHRINK(byte.arg);
        PyObject* _0 = VAR(std::move(list));
        _0 = call(builtins->attr(pk_id_set), _0);
        PUSH(_0);
    } DISPATCH();
    /*****************************************/
#define BINARY_OP_SPECIAL(func)                         \
        _ti = _inst_type_info(_0);                      \
        if(_ti->m##func){                               \
            TOP() = _ti->m##func(this, _0, _1);         \
        }else{                                          \
            PyObject* self;                                         \
            PyObject* _2 = get_unbound_method(_0, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _1);    \
            else TOP() = NotImplemented;                            \
        }

#define BINARY_OP_RSPECIAL(op, func)                                \
        if(TOP() == NotImplemented){                                \
            PyObject* self;                                         \
            PyObject* _2 = get_unbound_method(_1, func, &self, false);        \
            if(_2 != nullptr) TOP() = call_method(self, _2, _0);    \
            else BinaryOptError(op, _0, _1);                        \
            if(TOP() == NotImplemented) BinaryOptError(op, _0, _1); \
        }

    TARGET(BINARY_TRUEDIV){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__truediv__);
        if(TOP() == NotImplemented) BinaryOptError("/", _0, _1);
    } DISPATCH();
    TARGET(BINARY_POW){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__pow__);
        if(TOP() == NotImplemented) BinaryOptError("**", _0, _1);
    } DISPATCH();
    TARGET(BINARY_ADD){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(+)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__add__);
        BINARY_OP_RSPECIAL("+", __radd__);
    } DISPATCH()
    TARGET(BINARY_SUB){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(-)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__sub__);
        BINARY_OP_RSPECIAL("-", __rsub__);
    } DISPATCH()
    TARGET(BINARY_MUL){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(*)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__mul__);
        BINARY_OP_RSPECIAL("*", __rmul__);
    } DISPATCH()
    TARGET(BINARY_FLOORDIV){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_DIV_OP(/)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__floordiv__);
        if(TOP() == NotImplemented) BinaryOptError("//", _0, _1);
    } DISPATCH()
    TARGET(BINARY_MOD){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_DIV_OP(%)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__mod__);
        if(TOP() == NotImplemented) BinaryOptError("%", _0, _1);
    } DISPATCH()
    TARGET(COMPARE_LT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(<)
        TOP() = VAR(py_lt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_LE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(<=)
        TOP() = VAR(py_le(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_EQ){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_eq(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_NE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_ne(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(>)
        TOP() = VAR(py_gt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(>=)
        TOP() = VAR(py_ge(_0, _1));
    } DISPATCH()
    TARGET(BITWISE_LSHIFT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(<<)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__lshift__);
        if(TOP() == NotImplemented) BinaryOptError("<<", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_RSHIFT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(>>)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__rshift__);
        if(TOP() == NotImplemented) BinaryOptError(">>", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_AND){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(&)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__and__);
        if(TOP() == NotImplemented) BinaryOptError("&", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_OR){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(|)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__or__);
        if(TOP() == NotImplemented) BinaryOptError("|", _0, _1);
    } DISPATCH()
    TARGET(BITWISE_XOR){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        PREDICT_INT_OP(^)
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__xor__);
        if(TOP() == NotImplemented) BinaryOptError("^", _0, _1);
    } DISPATCH()
    TARGET(BINARY_MATMUL){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__matmul__);
        if(TOP() == NotImplemented) BinaryOptError("@", _0, _1);
    } DISPATCH();

#undef BINARY_OP_SPECIAL
#undef BINARY_OP_RSPECIAL
#undef PREDICT_INT_OP

    TARGET(IS_OP){
        PyObject* _1 = POPX();    // rhs
        PyObject* _0 = TOP();     // lhs
        TOP() = VAR(static_cast<bool>((_0==_1) ^ byte.arg));
    } DISPATCH();
    TARGET(CONTAINS_OP){
        // a in b -> b __contains__ a
        auto _ti = _inst_type_info(TOP());
        PyObject* _0;
        if(_ti->m__contains__){
            _0 = _ti->m__contains__(this, TOP(), SECOND());
        }else{
            _0 = call_method(TOP(), __contains__, SECOND());
        }
        POP();
        TOP() = VAR(static_cast<bool>((int)CAST(bool, _0) ^ byte.arg));
    } DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_ABSOLUTE_TOP)
        frame->jump_abs(_CAST(int, POPX()));
        DISPATCH();
    TARGET(POP_JUMP_IF_FALSE){
        if(!py_bool(TOP())) frame->jump_abs(byte.arg);
        POP();
    } DISPATCH();
    TARGET(POP_JUMP_IF_TRUE){
        if(py_bool(TOP())) frame->jump_abs(byte.arg);
        POP();
    } DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP){
        if(py_bool(TOP())) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP){
        if(!py_bool(TOP())) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(SHORTCUT_IF_FALSE_OR_POP){
        if(!py_bool(TOP())){                // [b, False]
            STACK_SHRINK(2);                // []
            PUSH(vm->False);                // [False]
            frame->jump_abs(byte.arg);
        } else POP();                       // [b]
    } DISPATCH();
    TARGET(LOOP_CONTINUE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(&s_data, byte.arg);
        DISPATCH();
    TARGET(GOTO) {
        StrName _name(byte.arg);
        int index = co->labels.try_get_likely_found(_name);
        if(index < 0) RuntimeError(_S("label ", _name.escape(), " not found"));
        frame->jump_abs_break(&s_data, index);
    } DISPATCH();
    /*****************************************/
    TARGET(FSTRING_EVAL){
        PyObject* _0 = co->consts[byte.arg];
        std::string_view string = CAST(Str&, _0).sv();
        auto it = _cached_codes.find(string);
        CodeObject_ code;
        if(it == _cached_codes.end()){
            code = vm->compile(string, "<eval>", EVAL_MODE, true);
            _cached_codes[string] = code;
        }else{
            code = it->second;
        }
        _0 = vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        PUSH(_0);
    } DISPATCH();
    TARGET(REPR)
        TOP() = py_repr(TOP());
        DISPATCH();
    TARGET(CALL){
        PyObject* _0 = vectorcall(
            byte.arg & 0xFF,          // ARGC
            (byte.arg>>8) & 0xFF,     // KWARGC
            true
        );
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
    } DISPATCH();
    TARGET(CALL_TP){
        PyObject* _0;
        PyObject* _1;
        PyObject* _2;
        // [callable, <self>, args: tuple, kwargs: dict | NULL]
        if(byte.arg){
            _2 = POPX();
            _1 = POPX();
            for(PyObject* obj: _CAST(Tuple&, _1)) PUSH(obj);
            _CAST(Dict&, _2).apply([this](PyObject* k, PyObject* v){
                PUSH(VAR(StrName(CAST(Str&, k)).index));
                PUSH(v);
            });
            _0 = vectorcall(
                _CAST(Tuple&, _1).size(),   // ARGC
                _CAST(Dict&, _2).size(),    // KWARGC
                true
            );
        }else{
            // no **kwargs
            _1 = POPX();
            for(PyObject* obj: _CAST(Tuple&, _1)) PUSH(obj);
            _0 = vectorcall(
                _CAST(Tuple&, _1).size(),   // ARGC
                0,                          // KWARGC
                true
            );
        }
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
    } DISPATCH();
    TARGET(RETURN_VALUE){
        PyObject* _0 = byte.arg == BC_NOARG ? POPX() : None;
        _pop_frame();
        if(frame == base_frame){       // [ frameBase<- ]
            return _0;
        }else{
            frame = top_frame();
            PUSH(_0);
            goto __NEXT_FRAME;
        }
    } DISPATCH();
    TARGET(YIELD_VALUE)
        return PY_OP_YIELD;
    /*****************************************/
    TARGET(LIST_APPEND){
        PyObject* _0 = POPX();
        CAST(List&, SECOND()).push_back(_0);
    } DISPATCH();
    TARGET(DICT_ADD) {
        PyObject* _0 = POPX();
        Tuple& t = CAST(Tuple&, _0);
        call_method(SECOND(), __setitem__, t[0], t[1]);
    } DISPATCH();
    TARGET(SET_ADD){
        PyObject* _0 = POPX();
        call_method(SECOND(), pk_id_add, _0);
    } DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = py_negate(TOP());
        DISPATCH();
    TARGET(UNARY_NOT){
        PyObject* _0 = TOP();
        if(_0==True) TOP()=False;
        else if(_0==False) TOP()=True;
        else TOP() = VAR(!py_bool(_0));
    } DISPATCH();
    TARGET(UNARY_STAR)
        TOP() = VAR(StarWrapper(byte.arg, TOP()));
        DISPATCH();
    TARGET(UNARY_INVERT){
        PyObject* _0;
        auto _ti = _inst_type_info(TOP());
        if(_ti->m__invert__) _0 = _ti->m__invert__(this, TOP());
        else _0 = call_method(TOP(), __invert__);
        TOP() = _0;
    } DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = py_iter(TOP());
        DISPATCH();
    TARGET(FOR_ITER){
        PyObject* _0 = py_next(TOP());
        if(_0 != StopIteration){
            PUSH(_0);
        }else{
            frame->jump_abs_break(&s_data, co->_get_block_codei(frame->_ip).end);
        }
    } DISPATCH();
    TARGET(FOR_ITER_STORE_FAST){
        PyObject* _0 = py_next(TOP());
        if(_0 != StopIteration){
            frame->_locals[byte.arg] = _0;
        }else{
            frame->jump_abs_break(&s_data, co->_get_block_codei(frame->_ip).end);
        }
    } DISPATCH()
    TARGET(FOR_ITER_STORE_GLOBAL){
        PyObject* _0 = py_next(TOP());
        if(_0 != StopIteration){
            frame->f_globals().set(StrName(byte.arg), _0);
        }else{
            frame->jump_abs_break(&s_data, co->_get_block_codei(frame->_ip).end);
        }
    } DISPATCH()
    TARGET(FOR_ITER_YIELD_VALUE){
        PyObject* _0 = py_next(TOP());
        if(_0 != StopIteration){
            PUSH(_0);
            return PY_OP_YIELD;
        }else{
            frame->jump_abs_break(&s_data, co->_get_block_codei(frame->_ip).end);
        }
    } DISPATCH()
    /*****************************************/
    TARGET(IMPORT_PATH){
        PyObject* _0 = co->consts[byte.arg];
        PUSH(py_import(CAST(Str&, _0)));
    } DISPATCH();
    TARGET(POP_IMPORT_STAR) {
        PyObject* _0 = POPX();        // pop the module
        PyObject* _1 = _0->attr().try_get(__all__);
        StrName _name;
        if(_1 != nullptr){
            for(PyObject* key: CAST(List&, _1)){
                _name = StrName::get(CAST(Str&, key).sv());
                PyObject* value = _0->attr().try_get_likely_found(_name);
                if(value == nullptr){
                    ImportError(_S("cannot import name ", _name.escape()));
                }else{
                    frame->f_globals().set(_name, value);
                }
            }
        }else{
            for(auto& [name, value]: _0->attr().items()){
                std::string_view s = name.sv();
                if(s.empty() || s[0] == '_') continue;
                frame->f_globals().set(name, value);
            }
        }
    } DISPATCH();
    /*****************************************/
    TARGET(UNPACK_SEQUENCE){
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* _0 = py_iter(POPX());
        for(int i=0; i<byte.arg; i++){
            PyObject* _1 = py_next(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        if(py_next(_0) != StopIteration) ValueError("too many values to unpack");
    } DISPATCH();
    TARGET(UNPACK_EX) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* _0 = py_iter(POPX());
        PyObject* _1;
        for(int i=0; i<byte.arg; i++){
            _1 = py_next(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        List extras;
        while(true){
            _1 = py_next(_0);
            if(_1 == StopIteration) break;
            extras.push_back(_1);
        }
        PUSH(VAR(extras));
    } DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CLASS){
        StrName _name(byte.arg);
        PyObject* _0 = POPX();   // super
        if(_0 == None) _0 = _t(tp_object);
        check_type(_0, tp_type);
        _curr_class = new_type_object(frame->_module, _name, PK_OBJ_GET(Type, _0));
    } DISPATCH();
    TARGET(END_CLASS) {
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        frame->_module->attr().set(_name, _curr_class);
        // call on_end_subclass
        PyTypeInfo* ti = &_all_types[PK_OBJ_GET(Type, _curr_class)];
        if(ti->base != tp_object){
            PyTypeInfo* base_ti = &_all_types[ti->base];
            if(base_ti->on_end_subclass) base_ti->on_end_subclass(this, ti);
        }
        _curr_class = nullptr;
    } DISPATCH();
    TARGET(STORE_CLASS_ATTR){
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        PyObject* _0 = POPX();
        if(is_type(_0, tp_function)){
            PK_OBJ_GET(Function, _0)._class = _curr_class;
        }
        _curr_class->attr().set(_name, _0);
    } DISPATCH();
    TARGET(BEGIN_CLASS_DECORATION){
        PUSH(_curr_class);
    } DISPATCH();
    TARGET(END_CLASS_DECORATION){
        _curr_class = POPX();
    } DISPATCH();
    TARGET(ADD_CLASS_ANNOTATION) {
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        Type type = PK_OBJ_GET(Type, _curr_class);
        _all_types[type].annotated_fields.push_back(_name);
    } DISPATCH();
    /*****************************************/
    TARGET(WITH_ENTER)
        PUSH(call_method(TOP(), __enter__));
        DISPATCH();
    TARGET(WITH_EXIT)
        call_method(TOP(), __exit__);
        POP();
        DISPATCH();
    /*****************************************/
    TARGET(EXCEPTION_MATCH) {
        PyObject* assumed_type = POPX();
        check_type(assumed_type, tp_type);
        PyObject* e_obj = TOP();
        bool ok = isinstance(e_obj, PK_OBJ_GET(Type, assumed_type));
        PUSH(VAR(ok));
    } DISPATCH();
    TARGET(RAISE) {
        if(is_type(TOP(), tp_type)){
            TOP() = call(TOP());
        }
        if(!isinstance(TOP(), tp_exception)){
            _builtin_error("TypeError", "exceptions must derive from Exception");
        }
        _error(POPX());
    } DISPATCH();
    TARGET(RAISE_ASSERT)
        if(byte.arg){
            PyObject* _0 = py_str(POPX());
            _builtin_error("AssertionError", CAST(Str, _0));
        }else{
            _builtin_error("AssertionError");
        }
        DISPATCH();
    TARGET(RE_RAISE) _raise(true); DISPATCH();
    TARGET(POP_EXCEPTION) _last_exception = POPX(); DISPATCH();
    /*****************************************/
    TARGET(FORMAT_STRING) {
        PyObject* _0 = POPX();
        const Str& spec = CAST(Str&, co->consts[byte.arg]);
        PUSH(_format_string(spec, _0));
    } DISPATCH();
    /*****************************************/
    TARGET(INC_FAST){
        PyObject** p = &frame->_locals[byte.arg];
        if(*p == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        *p = VAR(CAST(i64, *p) + 1);
    } DISPATCH();
    TARGET(DEC_FAST){
        PyObject** p = &frame->_locals[byte.arg];
        if(*p == PY_NULL) vm->NameError(co->varnames[byte.arg]);
        *p = VAR(CAST(i64, *p) - 1);
    } DISPATCH();
    TARGET(INC_GLOBAL){
        StrName _name(byte.arg);
        PyObject** p = frame->f_globals().try_get_2_likely_found(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) + 1);
    } DISPATCH();
    TARGET(DEC_GLOBAL){
        StrName _name(byte.arg);
        PyObject** p = frame->f_globals().try_get_2_likely_found(_name);
        if(p == nullptr) vm->NameError(_name);
        *p = VAR(CAST(i64, *p) - 1);
    } DISPATCH();
    /*****************************************/
    }

}

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL
#undef CEVAL_STEP_CALLBACK
/**********************************************************************/
            PK_UNREACHABLE()
        }catch(HandledException){
            continue;
        }catch(UnhandledException){
            PyObject* e_obj = POPX();
            Exception& _e = PK_OBJ_GET(Exception, e_obj);
            bool is_base_frame_to_be_popped = frame == base_frame;
            _pop_frame();
            if(callstack.empty()) throw _e;   // propagate to the top level
            frame = top_frame();
            PUSH(e_obj);
            if(is_base_frame_to_be_popped) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException){
            need_raise = true;
        }
    }
}

#undef TOP
#undef SECOND
#undef THIRD
#undef PEEK
#undef STACK_SHRINK
#undef PUSH
#undef POP
#undef POPX
#undef STACK_VIEW

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL

} // namespace pkpy
