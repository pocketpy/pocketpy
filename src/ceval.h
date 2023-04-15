#pragma once

#include "common.h"
#include "namedict.h"
#include "vm.h"

namespace pkpy{

inline PyObject* VM::_run_top_frame(){
    FrameId frame = top_frame();
    const int base_id = frame.index;
    bool need_raise = false;

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
 * For example, POPX() returns a strong reference which may be dangerous
 * `Args` containing strong references is safe if it is passed to `call` or `fast_call`
 */
{

/* Stack manipulation macros */
// https://github.com/python/cpython/blob/3.9/Python/ceval.c#L1123
#define TOP()             (frame->_s.top())
#define SECOND()          (frame->_s.second())
#define PEEK(n)           (frame->_s.peek(n))
#define STACK_SHRINK(n)   (frame->_s.shrink(n))
#define PUSH(v)           (frame->_s.push(v))
#define POP()             (frame->_s.pop())
#define POPX()            (frame->_s.popx())
#define STACK_VIEW(n)     (frame->_s.view(n))

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
    std::cout << frame->stack_info() << " " << OP_NAMES[byte.op] << std::endl;
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
    TARGET(PRINT_EXPR) {
        PyObject* obj = TOP();  // use top() to avoid accidental gc
        if(obj != None) *_stdout << CAST(Str&, asRepr(obj)) << '\n';
        POP();
    } DISPATCH();
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
        PyObject* obj;
        if(decl->nested){
            obj = VAR(Function({decl, frame->_module, frame->_locals}));
        }else{
            obj = VAR(Function({decl, frame->_module}));
        }
        PUSH(obj);
    } DISPATCH();
    TARGET(LOAD_NULL) PUSH(_py_null); DISPATCH();
    /*****************************************/
    TARGET(LOAD_FAST) {
        heap._auto_collect();
        PyObject* val = frame->_locals[byte.arg];
        if(val == nullptr) vm->NameError(co->varnames[byte.arg]);
        PUSH(val);
    } DISPATCH();
    TARGET(LOAD_NAME) {
        heap._auto_collect();
        StrName name(byte.arg);
        PyObject* val;
        val = frame->_locals.try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = frame->_closure.try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = frame->f_globals().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = vm->builtins->attr().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        vm->NameError(name);
    } DISPATCH();
    TARGET(LOAD_NONLOCAL) {
        heap._auto_collect();
        StrName name(byte.arg);
        PyObject* val;
        val = frame->_closure.try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = frame->f_globals().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = vm->builtins->attr().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        vm->NameError(name);
    } DISPATCH();
    TARGET(LOAD_GLOBAL) {
        heap._auto_collect();
        StrName name(byte.arg);
        PyObject* val = frame->f_globals().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        val = vm->builtins->attr().try_get(name);
        if(val != nullptr) { PUSH(val); DISPATCH(); }
        vm->NameError(name);
    } DISPATCH();
    TARGET(LOAD_ATTR) {
        PyObject* a = TOP();
        StrName name(byte.arg);
        TOP() = getattr(a, name);
    } DISPATCH();
    TARGET(LOAD_METHOD) {
        PyObject* a = TOP();
        StrName name(byte.arg);
        PyObject* self;
        TOP() = get_unbound_method(a, name, &self, true, true);
        PUSH(self);
    } DISPATCH();
    TARGET(LOAD_SUBSCR) {
        Args args(2);
        args[1] = POPX();    // b
        args[0] = TOP();     // a
        TOP() = fast_call(__getitem__, std::move(args));
    } DISPATCH();
    TARGET(STORE_FAST)
        frame->_locals[byte.arg] = POPX();
        DISPATCH();
    TARGET(STORE_NAME) {
        StrName name(byte.arg);
        PyObject* val = POPX();
        if(frame->_locals.is_valid()){
            bool ok = frame->_locals.try_set(name, val);
            if(!ok) vm->NameError(name);
        }else{
            frame->f_globals().set(name, val);
        }
    } DISPATCH();
    TARGET(STORE_GLOBAL) {
        StrName name(byte.arg);
        frame->f_globals().set(name, POPX());
    } DISPATCH();
    TARGET(STORE_ATTR) {
        StrName name(byte.arg);
        PyObject* a = TOP();
        PyObject* val = SECOND();
        setattr(a, name, val);
        STACK_SHRINK(2);
    } DISPATCH();
    TARGET(STORE_SUBSCR) {
        Args args(3);
        args[1] = POPX();    // b
        args[0] = POPX();    // a
        args[2] = POPX();    // val
        fast_call(__setitem__, std::move(args));
    } DISPATCH();
    TARGET(DELETE_FAST) {
        PyObject* val = frame->_locals[byte.arg];
        if(val == nullptr) vm->NameError(co->varnames[byte.arg]);
        frame->_locals[byte.arg] = nullptr;
    } DISPATCH();
    TARGET(DELETE_NAME) {
        StrName name(byte.arg);
        if(frame->_locals.is_valid()){
            if(!frame->_locals.contains(name)) vm->NameError(name);
            frame->_locals.erase(name);
        }else{
            if(!frame->f_globals().contains(name)) vm->NameError(name);
            frame->f_globals().erase(name);
        }
    } DISPATCH();
    TARGET(DELETE_GLOBAL) {
        StrName name(byte.arg);
        if(frame->f_globals().contains(name)){
            frame->f_globals().erase(name);
        }else{
            NameError(name);
        }
    } DISPATCH();
    TARGET(DELETE_ATTR) {
        PyObject* a = POPX();
        StrName name(byte.arg);
        if(!a->is_attr_valid()) TypeError("cannot delete attribute");
        if(!a->attr().contains(name)) AttributeError(a, name);
        a->attr().erase(name);
    } DISPATCH();
    TARGET(DELETE_SUBSCR) {
        PyObject* b = POPX();
        PyObject* a = POPX();
        fast_call(__delitem__, Args{a, b});
    } DISPATCH();
    /*****************************************/
    TARGET(BUILD_LIST) {
        PyObject* obj = VAR(STACK_VIEW(byte.arg).to_list());
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_DICT) {
        PyObject* t = VAR(STACK_VIEW(byte.arg).to_tuple());
        PyObject* obj = call(builtins->attr(m_dict), Args{t});
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_SET) {
        PyObject* t = VAR(STACK_VIEW(byte.arg).to_tuple());
        PyObject* obj = call(builtins->attr(m_set), Args{t});
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_SLICE) {
        PyObject* step = POPX();
        PyObject* stop = POPX();
        PyObject* start = POPX();
        Slice s;
        if(start != None) s.start = CAST(int, start);
        if(stop != None) s.stop = CAST(int, stop);
        if(step != None) s.step = CAST(int, step);
        PUSH(VAR(s));
    } DISPATCH();
    TARGET(BUILD_TUPLE) {
        PyObject* obj = VAR(STACK_VIEW(byte.arg).to_tuple());
        STACK_SHRINK(byte.arg);
        PUSH(obj);
    } DISPATCH();
    TARGET(BUILD_STRING) {
        std::stringstream ss;
        auto view = STACK_VIEW(byte.arg);
        for(PyObject* obj : view) ss << CAST(Str&, asStr(obj));
        STACK_SHRINK(byte.arg);
        PUSH(VAR(ss.str()));
    } DISPATCH();
    /*****************************************/
    TARGET(BINARY_OP) {
        Args args(2);
        args[1] = POPX();    // lhs
        args[0] = TOP();     // rhs
        TOP() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
    } DISPATCH();

#define INT_BINARY_OP(op, func) \
        if(is_both_int(TOP(), SECOND())){               \
            i64 b = _CAST(i64, TOP());                  \
            i64 a = _CAST(i64, SECOND());               \
            POP();                                      \
            TOP() = VAR(a op b);                        \
        }else{                                          \
            Args args(2);                               \
            args[1] = POPX();                    \
            args[0] = TOP();                            \
            TOP() = fast_call(func, std::move(args));   \
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
        PyObject* rhs = POPX();
        PyObject* lhs = TOP();
        bool ret_c = lhs == rhs;
        if(byte.arg == 1) ret_c = !ret_c;
        TOP() = VAR(ret_c);
    } DISPATCH();
    TARGET(CONTAINS_OP) {
        Args args(2);
        args[0] = POPX();
        args[1] = TOP();
        PyObject* ret = fast_call(__contains__, std::move(args));
        bool ret_c = CAST(bool, ret);
        if(byte.arg == 1) ret_c = !ret_c;
        TOP() = VAR(ret_c);
    } DISPATCH();
    /*****************************************/
    TARGET(JUMP_ABSOLUTE) frame->jump_abs(byte.arg); DISPATCH();
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
    TARGET(LOOP_BREAK) {
        int target = co_blocks[byte.block].end;
        frame->jump_abs_break(target);
    } DISPATCH();
    TARGET(GOTO) {
        StrName name(byte.arg);
        int index = co->labels->try_get(name);
        if(index < 0) _error("KeyError", fmt("label ", name.escape(), " not found"));
        frame->jump_abs_break(index);
    } DISPATCH();
    /*****************************************/
    TARGET(CALL)
    TARGET(CALL_UNPACK) {
        int ARGC = byte.arg;
        PyObject* callable = PEEK(ARGC+2);
        bool method_call = PEEK(ARGC+1) != _py_null;

        // fast path
        if(byte.op==OP_CALL && is_type(callable, tp_function)){
            PyObject* ret = _py_call(callable, STACK_VIEW(ARGC + int(method_call)), {});
            STACK_SHRINK(ARGC + 2);
            if(ret == nullptr) { DISPATCH_OP_CALL(); }
            else PUSH(ret);      // a generator
            DISPATCH();
        }
        Args args = STACK_VIEW(ARGC + int(method_call)).to_tuple();
        if(byte.op == OP_CALL_UNPACK) unpack_args(args);
        PyObject* ret = call(callable, std::move(args), no_arg(), true);
        STACK_SHRINK(ARGC + 2);
        if(ret == _py_op_call) { DISPATCH_OP_CALL(); }
        PUSH(ret);
    } DISPATCH();
    TARGET(CALL_KWARGS)
    TARGET(CALL_KWARGS_UNPACK) {
        // TODO: poor performance, refactor needed
        int ARGC = byte.arg & 0xFFFF;
        int KWARGC = (byte.arg >> 16) & 0xFFFF;
        Args kwargs = STACK_VIEW(KWARGC*2).to_tuple();
        STACK_SHRINK(KWARGC*2);

        bool method_call = PEEK(ARGC+1) != _py_null;
        if(method_call) ARGC++;         // add self into args
        Args args = STACK_VIEW(ARGC).to_tuple();
        STACK_SHRINK(ARGC);
        if(!method_call) POP();

        if(byte.op == OP_CALL_KWARGS_UNPACK) unpack_args(args);
        PyObject* callable = POPX();
        PyObject* ret = call(callable, std::move(args), kwargs, true);
        if(ret == _py_op_call) { DISPATCH_OP_CALL(); }
        PUSH(ret);
    } DISPATCH();
    TARGET(RETURN_VALUE) {
        PyObject* __ret = POPX();
        if(frame.index == base_id){       // [ frameBase<- ]
            callstack.pop();
            return __ret;
        }else{
            callstack.pop();
            frame = top_frame();
            PUSH(__ret);
            goto __NEXT_FRAME;
        }
    }
    TARGET(YIELD_VALUE) return _py_op_yield;
    /*****************************************/
    TARGET(LIST_APPEND) {
        PyObject* obj = POPX();
        List& list = CAST(List&, SECOND());
        list.push_back(obj);
    } DISPATCH();
    TARGET(DICT_ADD) {
        PyObject* kv = POPX();
        Tuple& t = CAST(Tuple& ,kv);
        fast_call(__setitem__, Args{SECOND(), t[0], t[1]});
    } DISPATCH();
    TARGET(SET_ADD) {
        PyObject* obj = POPX();
        fast_call(m_add, Args{SECOND(), obj});
    } DISPATCH();
    /*****************************************/
    TARGET(UNARY_NEGATIVE)
        TOP() = num_negated(TOP());
        DISPATCH();
    TARGET(UNARY_NOT)
        TOP() = VAR(!asBool(TOP()));
        DISPATCH();
    TARGET(UNARY_STAR)
        TOP() = VAR(StarWrapper(TOP()));
        DISPATCH();
    /*****************************************/
    TARGET(GET_ITER)
        TOP() = asIter(TOP());
        DISPATCH();
    TARGET(FOR_ITER) {
        BaseIter* it = PyIter_AS_C(TOP());
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
    }; DISPATCH();
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
/**********************************************************************/
            UNREACHABLE();
        }catch(HandledException& e){
            continue;
        }catch(UnhandledException& e){
            PyObject* obj = POPX();
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
            PUSH(obj);
            if(frame.index < base_id) throw ToBeRaisedException();
            need_raise = true;
        }catch(ToBeRaisedException& e){
            need_raise = true;
        }
    }
}

} // namespace pkpy