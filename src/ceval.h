#pragma once

#include "common.h"
#include "namedict.h"
#include "vm.h"

namespace pkpy{

inline PyObject* VM::_run_top_frame(){
    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    // shared registers
    PyObject *_0, *_1, *_2;
    StrName _name;

    while(true){
#if DEBUG_EXTRA_CHECK
        if(frame.index < base_id) FATAL_ERROR();
#endif
        try{
            if(need_raise){ need_raise = false; _raise(); }
            // if(s_data.is_overflow()) StackOverflowError();
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 */
{
#define DISPATCH_OP_CALL() { frame = top_frame(); goto __NEXT_FRAME; }
__NEXT_FRAME:
    Bytecode byte = frame->next_bytecode();
    // cache
    const CodeObject* co = frame->co;
    const auto& co_consts = co->consts;
    const auto& co_blocks = co->blocks;

#if PK_ENABLE_COMPUTED_GOTO
static void* OP_LABELS[] = {
    #define OPCODE(name) &&CASE_OP_##name,
    #include "opcodes.h"
    #undef OPCODE
};

#define DISPATCH() { byte = frame->next_bytecode(); goto *OP_LABELS[byte.op];}
#define TARGET(op) CASE_OP_##op:
goto *OP_LABELS[byte.op];

#else
#define TARGET(op) case OP_##op:
#define DISPATCH() { byte = frame->next_bytecode(); goto __NEXT_STEP;}

__NEXT_STEP:;
#if DEBUG_CEVAL_STEP
    _log_s_data();
#endif
#if DEBUG_CEVAL_STEP_MIN
    std::cout << OP_NAMES[byte.op] << std::endl;
#endif
    switch (byte.op)
    {
#endif
    TARGET(NO_OP) DISPATCH();
    /*****************************************/
    TARGET(POP_TOP) POP(); DISPATCH();
    TARGET(DUP_TOP) PUSH(TOP()); DISPATCH();
    TARGET(ROT_TWO) std::swap(TOP(), SECOND()); DISPATCH();
    TARGET(PRINT_EXPR)
        if(TOP() != None) *_stdout << CAST(Str&, asRepr(TOP())) << '\n';
        POP();
        DISPATCH();
    /*****************************************/
    TARGET(LOAD_CONST)
        heap._auto_collect();
        PUSH(co_consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE) PUSH(None); DISPATCH();
    TARGET(LOAD_TRUE) PUSH(True); DISPATCH();
    TARGET(LOAD_FALSE) PUSH(False); DISPATCH();
    TARGET(LOAD_INTEGER) PUSH(VAR(byte.arg)); DISPATCH();
    TARGET(LOAD_ELLIPSIS) PUSH(Ellipsis); DISPATCH();
    TARGET(LOAD_BUILTIN_EVAL) PUSH(builtins->attr(m_eval)); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = co->func_decls[byte.arg];
        bool is_simple = decl->starred_arg==-1 && decl->kwargs.size()==0 && !decl->code->is_generator;
        PyObject* obj;
        if(decl->nested){
            obj = VAR(Function({decl, is_simple, frame->_module, frame->_locals.to_namedict()}));
        }else{
            obj = VAR(Function({decl, is_simple, frame->_module}));
        }
        PUSH(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) PUSH(PY_NULL); DISPATCH();
    /*****************************************/
    TARGET(LOAD_FAST) {
        heap._auto_collect();
        _0 = frame->_locals[byte.arg];
        if(_0 == nullptr) vm->NameError(co->varnames[byte.arg]);
        PUSH(_0);
    } DISPATCH();
    TARGET(LOAD_NAME)
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->_locals.try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    TARGET(LOAD_NONLOCAL) {
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->f_closure_try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    } DISPATCH();
    TARGET(LOAD_GLOBAL)
        heap._auto_collect();
        _name = StrName(byte.arg);
        _0 = frame->f_globals().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        _0 = vm->builtins->attr().try_get(_name);
        if(_0 != nullptr) { PUSH(_0); DISPATCH(); }
        vm->NameError(_name);
        DISPATCH();
    TARGET(LOAD_ATTR)
        TOP() = getattr(TOP(), StrName(byte.arg));
        DISPATCH();
    TARGET(LOAD_METHOD)
        TOP() = get_unbound_method(TOP(), StrName(byte.arg), &_0, true, true);
        PUSH(_0);
        DISPATCH();
    TARGET(LOAD_SUBSCR)
        _1 = POPX();
        _0 = TOP();
        TOP() = call_method(_0, __getitem__, _1);
        DISPATCH();
    TARGET(STORE_FAST)
        frame->_locals[byte.arg] = POPX();
        DISPATCH();
    TARGET(STORE_NAME)
        _name = StrName(byte.arg);
        _0 = POPX();
        if(frame->_callable != nullptr){
            bool ok = frame->_locals.try_set(_name, _0);
            if(!ok) vm->NameError(_name);
        }else{
            frame->f_globals().set(_name, _0);
        }
        DISPATCH();
    TARGET(STORE_GLOBAL)
        frame->f_globals().set(StrName(byte.arg), POPX());
        DISPATCH();
    TARGET(STORE_ATTR) {
        _0 = TOP();         // a
        _1 = SECOND();      // val
        setattr(_0, StrName(byte.arg), _1);
        STACK_SHRINK(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR)
        _2 = POPX();        // b
        _1 = POPX();        // a
        _0 = POPX();        // val
        call_method(_1, __setitem__, _2, _0);
        DISPATCH();
    TARGET(DELETE_FAST)
        _0 = frame->_locals[byte.arg];
        if(_0 == nullptr) vm->NameError(co->varnames[byte.arg]);
        frame->_locals[byte.arg] = nullptr;
        DISPATCH();
    TARGET(DELETE_NAME)
        _name = StrName(byte.arg);
        if(frame->_callable != nullptr){
            if(!frame->_locals.contains(_name)) vm->NameError(_name);
            frame->_locals.erase(_name);
        }else{
            if(!frame->f_globals().contains(_name)) vm->NameError(_name);
            frame->f_globals().erase(_name);
        }
        DISPATCH();
    TARGET(DELETE_GLOBAL)
        _name = StrName(byte.arg);
        if(frame->f_globals().contains(_name)){
            frame->f_globals().erase(_name);
        }else{
            NameError(_name);
        }
        DISPATCH();
    TARGET(DELETE_ATTR)
        _0 = POPX();
        _name = StrName(byte.arg);
        if(!_0->is_attr_valid()) TypeError("cannot delete attribute");
        if(!_0->attr().contains(_name)) AttributeError(_0, _name);
        _0->attr().erase(_name);
        DISPATCH();
    TARGET(DELETE_SUBSCR)
        _1 = POPX();
        _0 = POPX();
        call_method(_0, __delitem__, _1);
        DISPATCH();
    /*****************************************/
    TARGET(BUILD_LIST)
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_DICT) {
        PyObject* t = VAR(STACK_VIEW(byte.arg).to_tuple());
        PyObject* obj = call(builtins->attr(m_dict), t);
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_SET) {
        PyObject* t = VAR(STACK_VIEW(byte.arg).to_tuple());
        PyObject* obj = call(builtins->attr(m_set), t);
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_SLICE) {
        _2 = POPX();
        _1 = POPX();
        _0 = POPX();
        Slice s;
        if(_0 != None) s.start = CAST(int, _0);
        if(_1 != None) s.stop = CAST(int, _1);
        if(_2 != None) s.step = CAST(int, _2);
        PUSH(VAR(s));
    } DISPATCH();
    TARGET(BUILD_TUPLE)
        _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_STRING) {
        std::stringstream ss;
        auto view = STACK_VIEW(byte.arg);
        for(PyObject* obj : view) ss << CAST(Str&, asStr(obj));
        STACK_SHRINK(byte.arg);
        PUSH(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
    TARGET(BINARY_OP)
        _1 = POPX();  // b
        _0 = TOP();   // a
        TOP() = call_method(_0, BINARY_SPECIAL_METHODS[byte.arg], _1);
        DISPATCH();

#define INT_BINARY_OP(op, func)                             \
        if(is_both_int(TOP(), SECOND())){                   \
            i64 b = _CAST(i64, TOP());                      \
            i64 a = _CAST(i64, SECOND());                   \
            POP();                                          \
            TOP() = VAR(a op b);                            \
        }else{                                              \
            _1 = POPX();                                    \
            _0 = TOP();                                     \
            TOP() = call_method(_0, func, _1);              \
        }

    TARGET(BINARY_ADD)
        INT_BINARY_OP(+, __add__)
        DISPATCH()
    TARGET(BINARY_SUB)
        INT_BINARY_OP(-, __sub__)
        DISPATCH()
    TARGET(BINARY_MUL)
        INT_BINARY_OP(*, __mul__)
        DISPATCH()
    TARGET(BINARY_FLOORDIV)
        INT_BINARY_OP(/, __floordiv__)
        DISPATCH()
    TARGET(BINARY_MOD)
        INT_BINARY_OP(%, __mod__)
        DISPATCH()
    TARGET(COMPARE_LT)
        INT_BINARY_OP(<, __lt__)
        DISPATCH()
    TARGET(COMPARE_LE)
        INT_BINARY_OP(<=, __le__)
        DISPATCH()
    TARGET(COMPARE_EQ)
        INT_BINARY_OP(==, __eq__)
        DISPATCH()
    TARGET(COMPARE_NE)
        INT_BINARY_OP(!=, __ne__)
        DISPATCH()
    TARGET(COMPARE_GT)
        INT_BINARY_OP(>, __gt__)
        DISPATCH()
    TARGET(COMPARE_GE)
        INT_BINARY_OP(>=, __ge__)
        DISPATCH()
    TARGET(BITWISE_LSHIFT)
        INT_BINARY_OP(<<, __lshift__)
        DISPATCH()
    TARGET(BITWISE_RSHIFT)
        INT_BINARY_OP(>>, __rshift__)
        DISPATCH()
    TARGET(BITWISE_AND)
        INT_BINARY_OP(&, __and__)
        DISPATCH()
    TARGET(BITWISE_OR)
        INT_BINARY_OP(|, __or__)
        DISPATCH()
    TARGET(BITWISE_XOR)
        INT_BINARY_OP(^, __xor__)
        DISPATCH()
#undef INT_BINARY_OP

    TARGET(IS_OP)
        _1 = POPX();    // rhs
        _0 = TOP();     // lhs
        if(byte.arg == 1){
            TOP() = VAR(_0 != _1);
        }else{
            TOP() = VAR(_0 == _1);
        }
        DISPATCH();
    TARGET(CONTAINS_OP)
        // a in b -> b __contains__ a
        _0 = call_method(TOP(), __contains__, SECOND());
        POP();
        if(byte.arg == 1){
            TOP() = VAR(!CAST(bool, _0));
        }else{
            TOP() = VAR(CAST(bool, _0));
        }
        DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE)
        frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(POP_JUMP_IF_FALSE)
        if(!asBool(POPX())) frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP)
        if(asBool(TOP()) == true) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP)
        if(asBool(TOP()) == false) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(LOOP_CONTINUE) {
        int target = co_blocks[byte.block].start;
        frame->jump_abs(target);
    } DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(
            co_blocks[byte.block].end
        );
        DISPATCH();
    TARGET(GOTO) {
        StrName name(byte.arg);
        int index = co->labels.try_get(name);
        if(index < 0) _error("KeyError", fmt("label ", name.escape(), " not found"));
        frame->jump_abs_break(index);
    } DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CALL)
        PUSH(PY_BEGIN_CALL);
        DISPATCH();
    TARGET(CALL)
        _0 = vectorcall(
            byte.arg & 0xFFFF,          // ARGC
            (byte.arg>>16) & 0xFFFF,    // KWARGC
            true
        );
        if(_0 == PY_OP_CALL) DISPATCH_OP_CALL();
        PUSH(_0);
        DISPATCH();
    TARGET(RETURN_VALUE)
        _0 = POPX();
        _pop_frame();
        if(frame.index == base_id){       // [ frameBase<- ]
            return _0;
        }else{
            frame = top_frame();
            PUSH(_0);
            goto __NEXT_FRAME;
        }
    TARGET(YIELD_VALUE)
        return PY_OP_YIELD;
    /*****************************************/
    TARGET(LIST_APPEND) {
        PyObject* obj = POPX();
        List& list = CAST(List&, SECOND());
        list.push_back(obj);
    } DISPATCH();
    TARGET(DICT_ADD) {
        _0 = POPX();
        Tuple& t = CAST(Tuple&, _0);
        call_method(SECOND(), __setitem__, t[0], t[1]);
    } DISPATCH();
    TARGET(SET_ADD)
        _0 = POPX();
        call_method(SECOND(), m_add, _0);
        DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = num_negated(TOP());
        DISPATCH();
    TARGET(UNARY_NOT)
        TOP() = VAR(!asBool(TOP()));
        DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = asIter(TOP());
        check_type(TOP(), tp_iterator);
        DISPATCH();
    TARGET(FOR_ITER) {
#if DEBUG_EXTRA_CHECK
        BaseIter* it = PyIter_AS_C(TOP());
#else
        BaseIter* it = _PyIter_AS_C(TOP());
#endif
        PyObject* obj = it->next();
        if(obj != nullptr){
            PUSH(obj);
        }else{
            int target = co_blocks[byte.block].end;
            frame->jump_abs_break(target);
        }
    } DISPATCH();
    /*****************************************/
    TARGET(IMPORT_NAME) {
        StrName name(byte.arg);
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod == nullptr){
            Str source;
            auto it = _lazy_modules.find(name);
            if(it == _lazy_modules.end()){
                bool ok = false;
                source = _read_file_cwd(fmt(name, ".py"), &ok);
                if(!ok) _error("ImportError", fmt("module ", name.escape(), " not found"));
            }else{
                source = it->second;
                _lazy_modules.erase(it);
            }
            CodeObject_ code = compile(source, name.sv(), EXEC_MODE);
            PyObject* new_mod = new_module(name);
            _exec(code, new_mod);
            new_mod->attr()._try_perfect_rehash();
            PUSH(new_mod);
        }else{
            PUSH(ext_mod);
        }
    } DISPATCH();
    TARGET(IMPORT_STAR) {
        PyObject* obj = POPX();
        for(auto& [name, value]: obj->attr().items()){
            std::string_view s = name.sv();
            if(s.empty() || s[0] == '_') continue;
            frame->f_globals().set(name, value);
        }
    }; DISPATCH();
    /*****************************************/
    TARGET(UNPACK_SEQUENCE)
    TARGET(UNPACK_EX) {
        // asIter or iter->next may run bytecode, accidential gc may happen
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* obj = asIter(POPX());
        BaseIter* iter = PyIter_AS_C(obj);
        for(int i=0; i<byte.arg; i++){
            PyObject* item = iter->next();
            if(item == nullptr) ValueError("not enough values to unpack");
            PUSH(item);
        }
        // handle extra items
        if(byte.op == OP_UNPACK_EX){
            List extras;
            while(true){
                PyObject* item = iter->next();
                if(item == nullptr) break;
                extras.push_back(item);
            }
            PUSH(VAR(extras));
        }else{
            if(iter->next() != nullptr) ValueError("too many values to unpack");
        }
    } DISPATCH();
    TARGET(UNPACK_UNLIMITED) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        PyObject* obj = asIter(POPX());
        BaseIter* iter = PyIter_AS_C(obj);
        obj = iter->next();
        while(obj != nullptr){
            PUSH(obj);
            obj = iter->next();
        }
    } DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CLASS) {
        StrName name(byte.arg);
        PyObject* super_cls = POPX();
        if(super_cls == None) super_cls = _t(tp_object);
        check_type(super_cls, tp_type);
        PyObject* cls = new_type_object(frame->_module, name, OBJ_GET(Type, super_cls));
        PUSH(cls);
    } DISPATCH();
    TARGET(END_CLASS) {
        PyObject* cls = POPX();
        cls->attr()._try_perfect_rehash();
    }; DISPATCH();
    TARGET(STORE_CLASS_ATTR) {
        StrName name(byte.arg);
        PyObject* obj = POPX();
        PyObject* cls = TOP();
        cls->attr().set(name, obj);
    } DISPATCH();
    /*****************************************/
    // // TODO: using "goto" inside with block may cause __exit__ not called
    // TARGET(WITH_ENTER) call(frame->pop_value(this), __enter__, no_arg()); DISPATCH();
    // TARGET(WITH_EXIT) call(frame->pop_value(this), __exit__, no_arg()); DISPATCH();
    /*****************************************/
    TARGET(ASSERT) {
        PyObject* obj = TOP();
        Str msg;
        if(is_type(obj, tp_tuple)){
            auto& t = CAST(Tuple&, obj);
            if(t.size() != 2) ValueError("assert tuple must have 2 elements");
            obj = t[0];
            msg = CAST(Str&, asStr(t[1]));
        }
        bool ok = asBool(obj);
        POP();
        if(!ok) _error("AssertionError", msg);
    } DISPATCH();
    TARGET(EXCEPTION_MATCH) {
        const auto& e = CAST(Exception&, TOP());
        StrName name(byte.arg);
        PUSH(VAR(e.match_type(name)));
    } DISPATCH();
    TARGET(RAISE) {
        PyObject* obj = POPX();
        Str msg = obj == None ? "" : CAST(Str, asStr(obj));
        _error(StrName(byte.arg), msg);
    } DISPATCH();
    TARGET(RE_RAISE) _raise(); DISPATCH();
#if !PK_ENABLE_COMPUTED_GOTO
#if DEBUG_EXTRA_CHECK
    default: throw std::runtime_error(fmt(OP_NAMES[byte.op], " is not implemented"));
#else
    default: UNREACHABLE();
#endif
    }
#endif
}

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL
/**********************************************************************/
            UNREACHABLE();
        }catch(HandledException& e){
            continue;
        }catch(UnhandledException& e){
            PyObject* obj = POPX();
            Exception& _e = CAST(Exception&, obj);
            _e.st_push(frame->snapshot());
            _pop_frame();
            if(callstack.empty()){
#if DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            PUSH(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
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

} // namespace pkpy