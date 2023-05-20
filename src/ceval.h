#pragma once

#include "common.h"
#include "namedict.h"
#include "vm.h"

namespace pkpy{

inline PyObject* VM::_run_top_frame(){
    DEF_SNAME(add);
    DEF_SNAME(set);
    DEF_SNAME(__enter__);
    DEF_SNAME(__exit__);
    DEF_SNAME(__doc__);

    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

    // shared registers
    PyObject *_0, *_1, *_2;
    const PyTypeInfo* _ti;
    StrName _name;

    while(true){
#if DEBUG_EXTRA_CHECK
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
        if(TOP() != None){
            _stdout(this, CAST(Str&, py_repr(TOP())));
            _stdout(this, "\n");
        }
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
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = co->func_decls[byte.arg];
        bool is_simple = decl->starred_arg==-1 && decl->kwargs.size()==0 && !decl->code->is_generator;
        int argc = decl->args.size();
        PyObject* obj;
        if(decl->nested){
            obj = VAR(Function({decl, is_simple, argc, frame->_module, frame->_locals.to_namedict()}));
        }else{
            obj = VAR(Function({decl, is_simple, argc, frame->_module}));
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
        _1 = POPX();    // b
        _0 = TOP();     // a
        _ti = _inst_type_info(_0);
        if(_ti->m__getitem__){
            TOP() = _ti->m__getitem__(this, _0, _1);
        }else{
            TOP() = call_method(_0, __getitem__, _1);
        }
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
        _ti = _inst_type_info(_1);
        if(_ti->m__setitem__){
            _ti->m__setitem__(this, _1, _2, _0);
        }else{
            call_method(_1, __setitem__, _2, _0);
        }
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
        if(is_tagged(_0) || !_0->is_attr_valid()) TypeError("cannot delete attribute");
        if(!_0->attr().contains(_name)) AttributeError(_0, _name);
        _0->attr().erase(_name);
        DISPATCH();
    TARGET(DELETE_SUBSCR)
        _1 = POPX();
        _0 = POPX();
        _ti = _inst_type_info(_0);
        if(_ti->m__delitem__){
            _ti->m__delitem__(this, _0, _1);
        }else{
            call_method(_0, __delitem__, _1);
        }
        DISPATCH();
    /*****************************************/
    TARGET(BUILD_LIST)
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_DICT)
        if(byte.arg == 0){
            PUSH(VAR(Dict(this)));
            DISPATCH();
        }
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(_t(tp_dict), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_SET)
        _0 = VAR(STACK_VIEW(byte.arg).to_list());
        _0 = call(builtins->attr(set), _0);
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_SLICE)
        _2 = POPX();    // step
        _1 = POPX();    // stop
        _0 = POPX();    // start
        PUSH(VAR(Slice(_0, _1, _2)));
        DISPATCH();
    TARGET(BUILD_TUPLE)
        _0 = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(_0);
        DISPATCH();
    TARGET(BUILD_STRING) {
        std::stringstream ss;
        ArgsView view = STACK_VIEW(byte.arg);
        for(PyObject* obj : view) ss << CAST(Str&, py_str(obj));
        STACK_SHRINK(byte.arg);
        PUSH(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
#define PREDICT_INT_OP(op)                              \
    if(is_both_int(TOP(), SECOND())){                   \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        TOP() = VAR(_CAST(i64, _0) op _CAST(i64, _1));  \
        DISPATCH();                                     \
    }

#define BINARY_OP_SPECIAL(func)                         \
        _1 = POPX();                                    \
        _0 = TOP();                                     \
        _ti = _inst_type_info(_0);                      \
        if(_ti->m##func){                               \
            TOP() = VAR(_ti->m##func(this, _0, _1));    \
        }else{                                          \
            TOP() = call_method(_0, func, _1);          \
        }

    TARGET(BINARY_TRUEDIV)
        if(is_tagged(SECOND())){
            f64 lhs = num_to_float(SECOND());
            f64 rhs = num_to_float(TOP());
            POP();
            TOP() = VAR(lhs / rhs);
            DISPATCH();
        }
        BINARY_OP_SPECIAL(__truediv__);
        DISPATCH();
    TARGET(BINARY_POW)
        BINARY_OP_SPECIAL(__pow__);
        DISPATCH();
    TARGET(BINARY_ADD)
        PREDICT_INT_OP(+);
        BINARY_OP_SPECIAL(__add__);
        DISPATCH()
    TARGET(BINARY_SUB)
        PREDICT_INT_OP(-);
        BINARY_OP_SPECIAL(__sub__);
        DISPATCH()
    TARGET(BINARY_MUL)
        BINARY_OP_SPECIAL(__mul__);
        DISPATCH()
    TARGET(BINARY_FLOORDIV)
        PREDICT_INT_OP(/);
        BINARY_OP_SPECIAL(__floordiv__);
        DISPATCH()
    TARGET(BINARY_MOD)
        PREDICT_INT_OP(%);
        BINARY_OP_SPECIAL(__mod__);
        DISPATCH()
    TARGET(COMPARE_LT)
        BINARY_OP_SPECIAL(__lt__);
        DISPATCH()
    TARGET(COMPARE_LE)
        BINARY_OP_SPECIAL(__le__);
        DISPATCH()
    TARGET(COMPARE_EQ)
        _1 = POPX();
        _0 = TOP();
        TOP() = VAR(py_equals(_0, _1));
        DISPATCH()
    TARGET(COMPARE_NE)
        _1 = POPX();
        _0 = TOP();
        TOP() = VAR(py_not_equals(_0, _1));
        DISPATCH()
    TARGET(COMPARE_GT)
        BINARY_OP_SPECIAL(__gt__);
        DISPATCH()
    TARGET(COMPARE_GE)
        BINARY_OP_SPECIAL(__ge__);
        DISPATCH()
    TARGET(BITWISE_LSHIFT)
        PREDICT_INT_OP(<<);
        BINARY_OP_SPECIAL(__lshift__);
        DISPATCH()
    TARGET(BITWISE_RSHIFT)
        PREDICT_INT_OP(>>);
        BINARY_OP_SPECIAL(__rshift__);
        DISPATCH()
    TARGET(BITWISE_AND)
        PREDICT_INT_OP(&);
        BINARY_OP_SPECIAL(__and__);
        DISPATCH()
    TARGET(BITWISE_OR)
        PREDICT_INT_OP(|);
        BINARY_OP_SPECIAL(__or__);
        DISPATCH()
    TARGET(BITWISE_XOR)
        PREDICT_INT_OP(^);
        BINARY_OP_SPECIAL(__xor__);
        DISPATCH()
    TARGET(BINARY_MATMUL)
        BINARY_OP_SPECIAL(__matmul__);
        DISPATCH();

#undef BINARY_OP_SPECIAL
#undef PREDICT_INT_OP

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
        _ti = _inst_type_info(TOP());
        if(_ti->m__contains__){
            _0 = VAR(_ti->m__contains__(this, TOP(), SECOND()));
        }else{
            _0 = call_method(TOP(), __contains__, SECOND());
        }
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
        if(!py_bool(POPX())) frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP)
        if(py_bool(TOP()) == true) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP)
        if(py_bool(TOP()) == false) frame->jump_abs(byte.arg);
        else POP();
        DISPATCH();
    TARGET(LOOP_CONTINUE)
        frame->jump_abs(co_blocks[byte.block].start);
        DISPATCH();
    TARGET(LOOP_BREAK)
        frame->jump_abs_break(co_blocks[byte.block].end);
        DISPATCH();
    TARGET(GOTO) {
        _name = StrName(byte.arg);
        int index = co->labels.try_get(_name);
        if(index < 0) _error("KeyError", fmt("label ", _name.escape(), " not found"));
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
    TARGET(LIST_APPEND)
        _0 = POPX();
        CAST(List&, SECOND()).push_back(_0);
        DISPATCH();
    TARGET(DICT_ADD) {
        _0 = POPX();
        Tuple& t = CAST(Tuple&, _0);
        call_method(SECOND(), __setitem__, t[0], t[1]);
    } DISPATCH();
    TARGET(SET_ADD)
        _0 = POPX();
        call_method(SECOND(), add, _0);
        DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = py_negate(TOP());
        DISPATCH();
    TARGET(UNARY_NOT)
        TOP() = VAR(!py_bool(TOP()));
        DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = py_iter(TOP());
        DISPATCH();
    TARGET(FOR_ITER)
        _0 = PyIterNext(TOP());
        if(_0 != StopIteration){
            PUSH(_0);
        }else{
            frame->jump_abs_break(co_blocks[byte.block].end);
        }
        DISPATCH();
    /*****************************************/
    TARGET(IMPORT_NAME) {
        StrName name(byte.arg);
        PyObject* ext_mod = _modules.try_get(name);
        if(ext_mod == nullptr){
            Str source;
            auto it = _lazy_modules.find(name);
            if(it == _lazy_modules.end()){
                Bytes b = _read_file_cwd(fmt(name, ".py"));
                if(!b) _error("ImportError", fmt("module ", name.escape(), " not found"));
                source = Str(b.str());
            }else{
                source = it->second;
                _lazy_modules.erase(it);
            }
            CodeObject_ code = compile(source, Str(name.sv())+".py", EXEC_MODE);
            PyObject* new_mod = new_module(name);
            _exec(code, new_mod);
            new_mod->attr()._try_perfect_rehash();
            PUSH(new_mod);
        }else{
            PUSH(ext_mod);
        }
    } DISPATCH();
    TARGET(IMPORT_STAR)
        _0 = POPX();
        for(auto& [name, value]: _0->attr().items()){
            std::string_view s = name.sv();
            if(s.empty() || s[0] == '_') continue;
            frame->f_globals().set(name, value);
        }
        DISPATCH();
    /*****************************************/
    TARGET(UNPACK_SEQUENCE)
    TARGET(UNPACK_EX) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        _0 = py_iter(POPX());
        for(int i=0; i<byte.arg; i++){
            _1 = PyIterNext(_0);
            if(_1 == StopIteration) ValueError("not enough values to unpack");
            PUSH(_1);
        }
        // handle extra items
        if(byte.op == OP_UNPACK_EX){
            List extras;
            while(true){
                _1 = PyIterNext(_0);
                if(_1 == StopIteration) break;
                extras.push_back(_1);
            }
            PUSH(VAR(extras));
        }else{
            if(PyIterNext(_0) != StopIteration) ValueError("too many values to unpack");
        }
    } DISPATCH();
    TARGET(UNPACK_UNLIMITED) {
        auto _lock = heap.gc_scope_lock();  // lock the gc via RAII!!
        _0 = py_iter(POPX());
        _1 = PyIterNext(_0);
        while(_1 != StopIteration){
            PUSH(_1);
            _1 = PyIterNext(_0);
        }
    } DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CLASS)
        _name = StrName(byte.arg);
        _0 = POPX();   // super
        if(_0 == None) _0 = _t(tp_object);
        check_non_tagged_type(_0, tp_type);
        _1 = new_type_object(frame->_module, _name, OBJ_GET(Type, _0));
        PUSH(_1);
        DISPATCH();
    TARGET(END_CLASS)
        _0 = POPX();
        _0->attr()._try_perfect_rehash();
        DISPATCH();
    TARGET(STORE_CLASS_ATTR)
        _name = StrName(byte.arg);
        _0 = POPX();
        TOP()->attr().set(_name, _0);
        DISPATCH();
    /*****************************************/
    // TODO: using "goto" inside with block may cause __exit__ not called
    TARGET(WITH_ENTER)
        call_method(POPX(), __enter__);
        DISPATCH();
    TARGET(WITH_EXIT)
        call_method(POPX(), __exit__);
        DISPATCH();
    /*****************************************/
    TARGET(ASSERT) {
        _0 = TOP();
        Str msg;
        if(is_type(_0, tp_tuple)){
            auto& t = CAST(Tuple&, _0);
            if(t.size() != 2) ValueError("assert tuple must have 2 elements");
            _0 = t[0];
            msg = CAST(Str&, py_str(t[1]));
        }
        bool ok = py_bool(_0);
        POP();
        if(!ok) _error("AssertionError", msg);
    } DISPATCH();
    TARGET(EXCEPTION_MATCH) {
        const auto& e = CAST(Exception&, TOP());
        _name = StrName(byte.arg);
        PUSH(VAR(e.match_type(_name)));
    } DISPATCH();
    TARGET(RAISE) {
        _0 = POPX();
        Str msg = _0 == None ? "" : CAST(Str, py_str(_0));
        _error(StrName(byte.arg), msg);
    } DISPATCH();
    TARGET(RE_RAISE) _raise(); DISPATCH();
    /*****************************************/
    TARGET(SETUP_DOCSTRING)
        TOP()->attr().set(__doc__, co_consts[byte.arg]);
        DISPATCH();
    TARGET(FORMAT_STRING) {
        _0 = POPX();
        const Str& spec = CAST(Str&, co_consts[byte.arg]);
        PUSH(format(spec, _0));
    } DISPATCH();
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

#undef DISPATCH
#undef TARGET
#undef DISPATCH_OP_CALL

} // namespace pkpy