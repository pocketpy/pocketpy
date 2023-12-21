#include "pocketpy/ceval.h"

namespace pkpy{

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
            else BinaryOptError(op);                                            \
            if(ret == NotImplemented) BinaryOptError(op);                       \
        }


bool VM::py_lt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__lt__, "<", __gt__);
    return CAST(bool, ret);
}

bool VM::py_le(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__le__, "<=", __ge__);
    return CAST(bool, ret);
}

bool VM::py_gt(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__gt__, ">", __lt__);
    return CAST(bool, ret);
}

bool VM::py_ge(PyObject* _0, PyObject* _1){
    BINARY_F_COMPARE(__ge__, ">=", __le__);
    return CAST(bool, ret);
}

#undef BINARY_F_COMPARE

static i64 _py_sint(PyObject* obj) noexcept {
    return (i64)(PK_BITS(obj) >> 2);
}

PyObject* VM::_run_top_frame(){
    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    while(true){
#if PK_DEBUG_EXTRA_CHECK
        if(frame.index < base_id) FATAL_ERROR();
#endif
        try{
            if(need_raise){ need_raise = false; _raise(); }
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 */
{

#if PK_ENABLE_CEVAL_CALLBACK
#define CEVAL_STEP() byte = frame->next_bytecode(); if(_ceval_on_step) _ceval_on_step(this, frame.get(), byte)
#else
#define CEVAL_STEP() byte = frame->next_bytecode()
#endif

#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    Bytecode CEVAL_STEP();
    // cache
    const CodeObject* co = frame->co;
    const auto& co_consts = co->consts;

#if PK_ENABLE_COMPUTED_GOTO
static void* OP_LABELS[] = {
    #define OPCODE(name) &&CASE_OP_##name,
    #include "pocketpy/opcodes.h"
    #undef OPCODE
};

#define DISPATCH() { CEVAL_STEP(); goto *OP_LABELS[byte.op];}
#define TARGET(op) CASE_OP_##op:
goto *OP_LABELS[byte.op];

#else
#define TARGET(op) case OP_##op:
#define DISPATCH() { CEVAL_STEP(); goto __NEXT_STEP;}

__NEXT_STEP:;
#if PK_DEBUG_CEVAL_STEP
    _log_s_data();
#endif
    switch (byte.op)
    {
#endif
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
        PUSH(co_consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE) PUSH(None); DISPATCH();
    TARGET(LOAD_TRUE) PUSH(True); DISPATCH();
    TARGET(LOAD_FALSE) PUSH(False); DISPATCH();
    TARGET(LOAD_INTEGER) PUSH(VAR((int16_t)byte.arg)); DISPATCH();
    TARGET(LOAD_ELLIPSIS) PUSH(Ellipsis); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = co->func_decls[byte.arg];
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
        if(heap._should_auto_collect()) heap._auto_collect();
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
        PK_LOCAL_STATIC const StrName m_long("long");
        PyObject* _0 = builtins->attr().try_get_likely_found(m_long);
        if(_0 == nullptr) AttributeError(builtins, m_long);
        TOP() = call(_0, TOP());
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
#define PREDICT_INT_OP(op)                              \
    if(is_small_int(TOP()) && is_small_int(SECOND())){  \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        if constexpr(#op[0] == '/' || #op[0] == '%'){   \
            if(_py_sint(_1) == 0) ZeroDivisionError();  \
        }                                               \
        TOP() = VAR(_py_sint(_0) op _py_sint(_1));      \
        DISPATCH();                                     \
    }

#define BINARY_OP_SPECIAL(func)                         \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
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
            else BinaryOptError(op);                                \
            if(TOP() == NotImplemented) BinaryOptError(op);         \
        }

    TARGET(BINARY_TRUEDIV){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__truediv__);
        if(TOP() == NotImplemented) BinaryOptError("/");
    } DISPATCH();
    TARGET(BINARY_POW){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__pow__);
        if(TOP() == NotImplemented) BinaryOptError("**");
    } DISPATCH();
    TARGET(BINARY_ADD){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(+);
        BINARY_OP_SPECIAL(__add__);
        BINARY_OP_RSPECIAL("+", __radd__);
    } DISPATCH()
    TARGET(BINARY_SUB){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(-);
        BINARY_OP_SPECIAL(__sub__);
        BINARY_OP_RSPECIAL("-", __rsub__);
    } DISPATCH()
    TARGET(BINARY_MUL){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__mul__);
        BINARY_OP_RSPECIAL("*", __rmul__);
    } DISPATCH()
    TARGET(BINARY_FLOORDIV){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(/);
        BINARY_OP_SPECIAL(__floordiv__);
        if(TOP() == NotImplemented) BinaryOptError("//");
    } DISPATCH()
    TARGET(BINARY_MOD){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(%);
        BINARY_OP_SPECIAL(__mod__);
        if(TOP() == NotImplemented) BinaryOptError("%");
    } DISPATCH()
    TARGET(COMPARE_LT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_lt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_LE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
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
        TOP() = VAR(!py_eq(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GT){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_gt(_0, _1));
    } DISPATCH()
    TARGET(COMPARE_GE){
        PyObject* _1 = POPX();
        PyObject* _0 = TOP();
        TOP() = VAR(py_ge(_0, _1));
    } DISPATCH()
    TARGET(BITWISE_LSHIFT){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(<<);
        BINARY_OP_SPECIAL(__lshift__);
        if(TOP() == NotImplemented) BinaryOptError("<<");
    } DISPATCH()
    TARGET(BITWISE_RSHIFT){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(>>);
        BINARY_OP_SPECIAL(__rshift__);
        if(TOP() == NotImplemented) BinaryOptError(">>");
    } DISPATCH()
    TARGET(BITWISE_AND){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(&);
        BINARY_OP_SPECIAL(__and__);
        if(TOP() == NotImplemented) BinaryOptError("&");
    } DISPATCH()
    TARGET(BITWISE_OR){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(|);
        BINARY_OP_SPECIAL(__or__);
        if(TOP() == NotImplemented) BinaryOptError("|");
    } DISPATCH()
    TARGET(BITWISE_XOR){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        PREDICT_INT_OP(^);
        BINARY_OP_SPECIAL(__xor__);
        if(TOP() == NotImplemented) BinaryOptError("^");
    } DISPATCH()
    TARGET(BINARY_MATMUL){
        PyObject* _0; PyObject* _1; const PyTypeInfo* _ti;
        BINARY_OP_SPECIAL(__matmul__);
        if(TOP() == NotImplemented) BinaryOptError("@");
    } DISPATCH();

#undef BINARY_OP_SPECIAL
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
    TARGET(POP_JUMP_IF_FALSE){
        PyObject* _0 = POPX();
        if(_0==False || !py_bool(_0)) frame->jump_abs(byte.arg);
    } DISPATCH();
    TARGET(POP_JUMP_IF_TRUE){
        PyObject* _0 = POPX();
        if(_0==True || py_bool(_0)) frame->jump_abs(byte.arg);
    } DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP){
        PyObject* _0 = TOP();
        if(_0==True || py_bool(_0)) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP){
        PyObject* _0 = TOP();
        if(_0==False || !py_bool(_0)) frame->jump_abs(byte.arg);
        else POP();
    } DISPATCH();
    TARGET(SHORTCUT_IF_FALSE_OR_POP){
        PyObject* _0 = TOP();
        if(_0==False || !py_bool(_0)){      // [b, False]
            STACK_SHRINK(2);                // []
            PUSH(vm->False);                // [False]
            frame->jump_abs(byte.arg);
        } else POP();                       // [b]
    } DISPATCH();
    TARGET(LOOP_CONTINUE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(byte.arg);
        DISPATCH();
    TARGET(GOTO) {
        StrName _name(byte.arg);
        int index = co->labels.try_get_likely_found(_name);
        if(index < 0) _error("KeyError", fmt("label ", _name.escape(), " not found"));
        frame->jump_abs_break(index);
    } DISPATCH();
    /*****************************************/
    TARGET(FSTRING_EVAL){
        PyObject* _0 = co_consts[byte.arg];
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
        PyObject* _0 = POPX();
        _pop_frame();
        if(frame.index == base_id){       // [ frameBase<- ]
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
            frame->jump_abs_break(byte.arg);
        }
    } DISPATCH();
    /*****************************************/
    TARGET(IMPORT_PATH){
        PyObject* _0 = co_consts[byte.arg];
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
                    ImportError(fmt("cannot import name ", _name.escape()));
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
        check_non_tagged_type(_0, tp_type);
        _curr_class = new_type_object(frame->_module, _name, PK_OBJ_GET(Type, _0));
    } DISPATCH();
    TARGET(END_CLASS) {
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        frame->_module->attr().set(_name, _curr_class);
        _curr_class = nullptr;
    } DISPATCH();
    TARGET(STORE_CLASS_ATTR){
        PK_ASSERT(_curr_class != nullptr);
        StrName _name(byte.arg);
        PyObject* _0 = POPX();
        if(is_non_tagged_type(_0, tp_function)){
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
        _type_info(type)->annotated_fields.push_back(_name);
    } DISPATCH();
    /*****************************************/
    TARGET(WITH_ENTER)
        call_method(POPX(), __enter__);
        DISPATCH();
    TARGET(WITH_EXIT)
        call_method(POPX(), __exit__);
        DISPATCH();
    /*****************************************/
    TARGET(EXCEPTION_MATCH) {
        const auto& e = CAST(Exception&, TOP());
        PUSH(VAR(e.match_type(StrName(byte.arg))));
    } DISPATCH();
    TARGET(RAISE) {
        PyObject* _0 = POPX();
        Str msg = _0 == None ? "" : CAST(Str, py_str(_0));
        _error(StrName(byte.arg), msg);
    } DISPATCH();
    TARGET(RAISE_ASSERT)
        if(byte.arg){
            PyObject* _0 = py_str(POPX());
            _error("AssertionError", CAST(Str, _0));
        }else{
            _error("AssertionError", "");
        }
        DISPATCH();
    TARGET(RE_RAISE) _raise(true); DISPATCH();
    TARGET(POP_EXCEPTION) _last_exception = POPX(); DISPATCH();
    /*****************************************/
    TARGET(FORMAT_STRING) {
        PyObject* _0 = POPX();
        const Str& spec = CAST(Str&, co_consts[byte.arg]);
        PUSH(format(spec, _0));
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

#if !PK_ENABLE_COMPUTED_GOTO
        static_assert(OP_DEC_GLOBAL == 110);
        case 111: case 112: case 113: case 114: case 115: case 116: case 117: case 118: case 119: case 120: case 121: case 122: case 123: case 124: case 125: case 126: case 127: case 128: case 129: case 130: case 131: case 132: case 133: case 134: case 135: case 136: case 137: case 138: case 139: case 140: case 141: case 142: case 143: case 144: case 145: case 146: case 147: case 148: case 149: case 150: case 151: case 152: case 153: case 154: case 155: case 156: case 157: case 158: case 159: case 160: case 161: case 162: case 163: case 164: case 165: case 166: case 167: case 168: case 169: case 170: case 171: case 172: case 173: case 174: case 175: case 176: case 177: case 178: case 179: case 180: case 181: case 182: case 183: case 184: case 185: case 186: case 187: case 188: case 189: case 190: case 191: case 192: case 193: case 194: case 195: case 196: case 197: case 198: case 199: case 200: case 201: case 202: case 203: case 204: case 205: case 206: case 207: case 208: case 209: case 210: case 211: case 212: case 213: case 214: case 215: case 216: case 217: case 218: case 219: case 220: case 221: case 222: case 223: case 224: case 225: case 226: case 227: case 228: case 229: case 230: case 231: case 232: case 233: case 234: case 235: case 236: case 237: case 238: case 239: case 240: case 241: case 242: case 243: case 244: case 245: case 246: case 247: case 248: case 249: case 250: case 251: case 252: case 253: case 254: case 255: FATAL_ERROR(); break;
    }
#endif
}

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL
#undef CEVAL_STEP
/**********************************************************************/
            UNREACHABLE();
        }catch(HandledException& e){
            PK_UNUSED(e);
            continue;
        }catch(UnhandledException& e){
            PK_UNUSED(e);
            PyObject* obj = POPX();
            Exception& _e = CAST(Exception&, obj);
            _pop_frame();
            if(callstack.empty()){
#if PK_DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            PUSH(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
            PK_UNUSED(e);
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
