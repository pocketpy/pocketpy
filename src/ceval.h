#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy{

inline PyObject* VM::_run_top_frame(){
    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;
    PyObject* __ret;

    while(true){
#if DEBUG_EXTRA_CHECK
        if(frame->id < base_id) FATAL_ERROR();
#endif
        try{
            if(need_raise){ need_raise = false; _raise(); }
/**********************************************************************/
/* NOTE: 
 * Be aware of accidental gc!
 * DO NOT leave any strong reference of PyObject* in the C stack
 * For example, frame->popx() returns a strong reference which may be dangerous
 * `Args` containing strong references is safe if it is passed to `call` or `fast_call`
 */
{
    Bytecode byte = frame->next_bytecode();

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
    std::cout << frame->stack_info() << " " << OP_NAMES[byte.op] << std::endl;
#endif
    switch (byte.op)
    {
#endif
    TARGET(NO_OP) DISPATCH();
    /*****************************************/
    TARGET(POP_TOP) frame->pop(); DISPATCH();
    TARGET(DUP_TOP) frame->push(frame->top()); DISPATCH();
    TARGET(ROT_TWO) std::swap(frame->top(), frame->top_1()); DISPATCH();
    TARGET(PRINT_EXPR) {
        PyObject* obj = frame->top();  // use top() to avoid accidental gc
        if(obj != None) *_stdout << CAST(Str&, asRepr(obj)) << '\n';
        frame->pop();
    } DISPATCH();
    /*****************************************/
    TARGET(LOAD_CONST)
        heap._auto_collect();
        frame->push(frame->co->consts[byte.arg]);
        DISPATCH();
    TARGET(LOAD_NONE) frame->push(None); DISPATCH();
    TARGET(LOAD_TRUE) frame->push(True); DISPATCH();
    TARGET(LOAD_FALSE) frame->push(False); DISPATCH();
    TARGET(LOAD_ELLIPSIS) frame->push(Ellipsis); DISPATCH();
    TARGET(LOAD_BUILTIN_EVAL) frame->push(builtins->attr(m_eval)); DISPATCH();
    TARGET(LOAD_FUNCTION) {
        FuncDecl_ decl = frame->co->func_decls[byte.arg];
        PyObject* obj = VAR(Function({decl, frame->_module, frame->_locals}));
        frame->push(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) frame->push(_py_null); DISPATCH();
    /*****************************************/
    TARGET(LOAD_NAME) {
        heap._auto_collect();
        StrName name = frame->co->names[byte.arg];
        PyObject* val;
        val = frame->f_locals().try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        val = frame->f_closure_try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        val = frame->f_globals().try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        val = vm->builtins->attr().try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        vm->NameError(name);
    } DISPATCH();
    TARGET(LOAD_GLOBAL) {
        heap._auto_collect();
        StrName name = frame->co->names[byte.arg];
        PyObject* val = frame->f_globals().try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        val = vm->builtins->attr().try_get(name);
        if(val != nullptr) { frame->push(val); DISPATCH(); }
        vm->NameError(name);
    } DISPATCH();
    TARGET(LOAD_ATTR) {
        PyObject* a = frame->top();
        StrName name = frame->co->names[byte.arg];
        frame->top() = getattr(a, name);
    } DISPATCH();
    TARGET(LOAD_METHOD) {
        PyObject* a = frame->top();
        StrName name = frame->co->names[byte.arg];
        PyObject* self;
        frame->top() = get_unbound_method(a, name, &self, true, true);
        frame->push(self);
    } DISPATCH();
    TARGET(LOAD_SUBSCR) {
        Args args(2);
        args[1] = frame->popx();    // b
        args[0] = frame->top();     // a
        frame->top() = fast_call(__getitem__, std::move(args));
    } DISPATCH();
    TARGET(STORE_LOCAL) {
        StrName name = frame->co->names[byte.arg];
        frame->f_locals().set(name, frame->popx());
    } DISPATCH();
    TARGET(STORE_GLOBAL) {
        StrName name = frame->co->names[byte.arg];
        frame->f_globals().set(name, frame->popx());
    } DISPATCH();
    TARGET(STORE_ATTR) {
        StrName name = frame->co->names[byte.arg];
        PyObject* a = frame->top();
        PyObject* val = frame->top_1();
        setattr(a, name, val);
        frame->pop_n(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR) {
        Args args(3);
        args[1] = frame->popx();    // b
        args[0] = frame->popx();    // a
        args[2] = frame->popx();    // val
        fast_call(__setitem__, std::move(args));
    } DISPATCH();
    TARGET(DELETE_LOCAL) {
        StrName name = frame->co->names[byte.arg];
        if(frame->f_locals().contains(name)){
            frame->f_locals().erase(name);
        }else{
            NameError(name);
        }
    } DISPATCH();
    TARGET(DELETE_GLOBAL) {
        StrName name = frame->co->names[byte.arg];
        if(frame->f_globals().contains(name)){
            frame->f_globals().erase(name);
        }else{
            NameError(name);
        }
    } DISPATCH();
    TARGET(DELETE_ATTR) {
        PyObject* a = frame->popx();
        StrName name = frame->co->names[byte.arg];
        if(!a->is_attr_valid()) TypeError("cannot delete attribute");
        if(!a->attr().contains(name)) AttributeError(a, name);
        a->attr().erase(name);
    } DISPATCH();
    TARGET(DELETE_SUBSCR) {
        PyObject* b = frame->popx();
        PyObject* a = frame->popx();
        fast_call(__delitem__, Args{a, b});
    } DISPATCH();
    /*****************************************/
    TARGET(BUILD_LIST)
        frame->push(VAR(frame->popx_n_reversed(byte.arg).to_list()));
        DISPATCH();
    TARGET(BUILD_DICT) {
        PyObject* t = VAR(frame->popx_n_reversed(byte.arg));
        PyObject* obj = call(builtins->attr(m_dict), Args{t});
        frame->push(obj);
    } DISPATCH();
    TARGET(BUILD_SET) {
        PyObject* t = VAR(frame->popx_n_reversed(byte.arg));
        PyObject* obj = call(builtins->attr(m_set), Args{t});
        frame->push(obj);
    } DISPATCH();
    TARGET(BUILD_SLICE) {
        PyObject* step = frame->popx();
        PyObject* stop = frame->popx();
        PyObject* start = frame->popx();
        Slice s;
        if(start != None) s.start = CAST(int, start);
        if(stop != None) s.stop = CAST(int, stop);
        if(step != None) s.step = CAST(int, step);
        frame->push(VAR(s));
    } DISPATCH();
    TARGET(BUILD_TUPLE) {
        Tuple items = frame->popx_n_reversed(byte.arg);
        frame->push(VAR(std::move(items)));
    } DISPATCH();
    TARGET(BUILD_STRING) {
        std::stringstream ss;   // asStr() may run extra bytecode
        for(int i=byte.arg-1; i>=0; i--) ss << CAST(Str&, asStr(frame->top_n(i)));
        frame->pop_n(byte.arg);
        frame->push(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
    TARGET(BINARY_OP) {
        Args args(2);
        args[1] = frame->popx();    // lhs
        args[0] = frame->top();     // rhs
        frame->top() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
    } DISPATCH();

#define INT_BINARY_OP(op, func) \
        if(is_both_int(frame->top(), frame->top_1())){      \
            i64 b = _CAST(i64, frame->top());               \
            i64 a = _CAST(i64, frame->top_1());             \
            frame->pop();                                   \
            frame->top() = VAR(a op b);                     \
        }else{                                              \
            Args args(2);                                   \
            args[1] = frame->popx();                        \
            args[0] = frame->top();                         \
            frame->top() = fast_call(func, std::move(args));\
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
    TARGET(IS_OP) {
        PyObject* rhs = frame->popx();
        PyObject* lhs = frame->top();
        bool ret_c = lhs == rhs;
        if(byte.arg == 1) ret_c = !ret_c;
        frame->top() = VAR(ret_c);
    } DISPATCH();
    TARGET(CONTAINS_OP) {
        Args args(2);
        args[0] = frame->popx();
        args[1] = frame->top();
        PyObject* ret = fast_call(__contains__, std::move(args));
        bool ret_c = CAST(bool, ret);
        if(byte.arg == 1) ret_c = !ret_c;
        frame->top() = VAR(ret_c);
    } DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE) frame->jump_abs(byte.arg); DISPATCH();
    TARGET(POP_JUMP_IF_FALSE)
        if(!asBool(frame->popx())) frame->jump_abs(byte.arg);
        DISPATCH();
    TARGET(JUMP_IF_TRUE_OR_POP)
        if(asBool(frame->top()) == true) frame->jump_abs(byte.arg);
        else frame->pop();
        DISPATCH();
    TARGET(JUMP_IF_FALSE_OR_POP)
        if(asBool(frame->top()) == false) frame->jump_abs(byte.arg);
        else frame->pop();
        DISPATCH();
    TARGET(LOOP_CONTINUE) {
        int target = frame->co->blocks[byte.block].start;
        frame->jump_abs(target);
    } DISPATCH();
    TARGET(LOOP_BREAK) {
        int target = frame->co->blocks[byte.block].end;
        frame->jump_abs_break(target);
    } DISPATCH();
    TARGET(GOTO) {
        StrName label = frame->co->names[byte.arg];
        auto it = frame->co->labels.find(label);
        if(it == frame->co->labels.end()) _error("KeyError", fmt("label ", label.escape(), " not found"));
        frame->jump_abs_break(it->second);
    } DISPATCH();
    /*****************************************/
    TARGET(CALL)
    TARGET(CALL_UNPACK) {
        int ARGC = byte.arg;

        bool method_call = frame->top_n(ARGC) != _py_null;
        if(method_call) ARGC++;         // add self into args
        Args args = frame->popx_n_reversed(ARGC);
        if(!method_call) frame->pop();

        if(byte.op == OP_CALL_UNPACK) unpack_args(args);
        PyObject* callable = frame->popx();
        PyObject* ret = call(callable, std::move(args), no_arg(), true);
        if(ret == _py_op_call) { __ret=ret; goto __PY_OP_CALL; }
        frame->push(ret);
    } DISPATCH();
    TARGET(CALL_KWARGS)
    TARGET(CALL_KWARGS_UNPACK) {
        int ARGC = byte.arg & 0xFFFF;
        int KWARGC = (byte.arg >> 16) & 0xFFFF;
        Args kwargs = frame->popx_n_reversed(KWARGC*2);

        bool method_call = frame->top_n(ARGC) != _py_null;
        if(method_call) ARGC++;         // add self into args
        Args args = frame->popx_n_reversed(ARGC);
        if(!method_call) frame->pop();

        if(byte.op == OP_CALL_KWARGS_UNPACK) unpack_args(args);
        PyObject* callable = frame->popx();
        PyObject* ret = call(callable, std::move(args), kwargs, true);
        if(ret == _py_op_call) { __ret=ret; goto __PY_OP_CALL; }
        frame->push(ret);
    } DISPATCH();
    TARGET(RETURN_VALUE) { __ret=frame->popx(); goto __PY_SIMPLE_RETURN; }
    TARGET(YIELD_VALUE) return _py_op_yield;
    /*****************************************/
    TARGET(LIST_APPEND) {
        PyObject* obj = frame->popx();
        List& list = CAST(List&, frame->top_1());
        list.push_back(obj);
    } DISPATCH();
    TARGET(DICT_ADD) {
        PyObject* kv = frame->popx();
        Tuple& t = CAST(Tuple& ,kv);
        fast_call(__setitem__, Args{frame->top_1(), t[0], t[1]});
    } DISPATCH();
    TARGET(SET_ADD) {
        PyObject* obj = frame->popx();
        fast_call(m_add, Args{frame->top_1(), obj});
    } DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        frame->top() = num_negated(frame->top());
        DISPATCH();
    TARGET(UNARY_NOT)
        frame->top() = VAR(!asBool(frame->top()));
        DISPATCH();
    TARGET(UNARY_STAR)
        frame->top() = VAR(StarWrapper(frame->top()));
        DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        frame->top() = asIter(frame->top());
        DISPATCH();
    TARGET(FOR_ITER) {
        BaseIter* it = PyIter_AS_C(frame->top());
        PyObject* obj = it->next();
        if(obj != nullptr){
            frame->push(obj);
        }else{
            int target = frame->co->blocks[byte.block].end;
            frame->jump_abs_break(target);
        }
    } DISPATCH();
    /*****************************************/
    TARGET(IMPORT_NAME) {
        StrName name = frame->co->names[byte.arg];
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
            frame->push(new_mod);
        }else{
            frame->push(ext_mod);
        }
    } DISPATCH();
    TARGET(IMPORT_STAR) {
        PyObject* obj = frame->popx();
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
        PyObject* obj = asIter(frame->popx());
        BaseIter* iter = PyIter_AS_C(obj);
        for(int i=0; i<byte.arg; i++){
            PyObject* item = iter->next();
            if(item == nullptr) ValueError("not enough values to unpack");
            frame->push(item);
        }
        // handle extra items
        if(byte.op == OP_UNPACK_EX){
            List extras;
            while(true){
                PyObject* item = iter->next();
                if(item == nullptr) break;
                extras.push_back(item);
            }
            frame->push(VAR(extras));
        }else{
            if(iter->next() != nullptr) ValueError("too many values to unpack");
        }
    }; DISPATCH();
    /*****************************************/
    TARGET(BEGIN_CLASS) {
        StrName name = frame->co->names[byte.arg];
        PyObject* super_cls = frame->popx();
        if(super_cls == None) super_cls = _t(tp_object);
        check_type(super_cls, tp_type);
        PyObject* cls = new_type_object(frame->_module, name, OBJ_GET(Type, super_cls));
        frame->push(cls);
    } DISPATCH();
    TARGET(END_CLASS) {
        PyObject* cls = frame->popx();
        cls->attr()._try_perfect_rehash();
    }; DISPATCH();
    TARGET(STORE_CLASS_ATTR) {
        StrName name = frame->co->names[byte.arg];
        PyObject* obj = frame->popx();
        PyObject* cls = frame->top();
        cls->attr().set(name, obj);
    } DISPATCH();
    /*****************************************/
    // // TODO: using "goto" inside with block may cause __exit__ not called
    // TARGET(WITH_ENTER) call(frame->pop_value(this), __enter__, no_arg()); DISPATCH();
    // TARGET(WITH_EXIT) call(frame->pop_value(this), __exit__, no_arg()); DISPATCH();
    /*****************************************/
    TARGET(ASSERT) {
        PyObject* obj = frame->top();
        Str msg;
        if(is_type(obj, tp_tuple)){
            auto& t = CAST(Tuple&, obj);
            if(t.size() != 2) ValueError("assert tuple must have 2 elements");
            obj = t[0];
            msg = CAST(Str&, asStr(t[1]));
        }
        bool ok = asBool(obj);
        frame->pop();
        if(!ok) _error("AssertionError", msg);
    } DISPATCH();
    TARGET(EXCEPTION_MATCH) {
        const auto& e = CAST(Exception&, frame->top());
        StrName name = frame->co->names[byte.arg];
        frame->push(VAR(e.match_type(name)));
    } DISPATCH();
    TARGET(RAISE) {
        PyObject* obj = frame->popx();
        Str msg = obj == None ? "" : CAST(Str, asStr(obj));
        StrName type = frame->co->names[byte.arg];
        _error(type, msg);
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
/**********************************************************************/
__PY_SIMPLE_RETURN:
            if(frame.index == base_id){       // [ frameBase<- ]
                callstack.pop();
                return __ret;
            }else{
                callstack.pop();
                frame = top_frame();
                frame->push(__ret);
            }
            continue;
__PY_OP_CALL:
            frame = top_frame();            // [ frameBase, newFrame<- ]
            continue;
        }catch(HandledException& e){
            continue;
        }catch(UnhandledException& e){
            PyObject* obj = frame->popx();
            Exception& _e = CAST(Exception&, obj);
            _e.st_push(frame->snapshot());
            callstack.pop();
            if(callstack.empty()){
#if DEBUG_FULL_EXCEPTION
                std::cerr << _e.summary() << std::endl;
#endif
                throw _e;
            }
            frame = top_frame();
            frame->push(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
            need_raise = true;
        }
    }
}

} // namespace pkpy