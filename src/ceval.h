#pragma once

#include "vm.h"

PyVar VM::run_frame(Frame* frame){
    while(frame->has_next_bytecode()){
        const Bytecode& byte = frame->next_bytecode();
        // if(true || frame->_module != builtins){
        //     printf("%d: %s (%d) %s\n",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 frame->_ip, OP_NAMES[byte.op], byte.arg, frame->stack_info().c_str());
        // }
        switch (byte.op)
        {
        case OP_NO_OP: continue;
        case OP_LOAD_CONST: frame->push(frame->co->consts[byte.arg]); continue;
        case OP_LOAD_FUNCTION: {
            const PyVar obj = frame->co->consts[byte.arg];
            pkpy::Function f = PyFunction_AS_C(obj);  // copy
            f._module = frame->_module;
            frame->push(PyFunction(f));
        } continue;
        case OP_SETUP_CLOSURE: {
            pkpy::Function& f = PyFunction_AS_C(frame->top());    // reference
            f._closure = frame->_locals;
        } continue;
        case OP_LOAD_NAME_REF: {
            frame->push(PyRef(NameRef(frame->co->names[byte.arg])));
        } continue;
        case OP_LOAD_NAME: {
            frame->push(NameRef(frame->co->names[byte.arg]).get(this, frame));
        } continue;
        case OP_STORE_NAME: {
            auto& p = frame->co->names[byte.arg];
            NameRef(p).set(this, frame, frame->pop());
        } continue;
        case OP_BUILD_ATTR: {
            int name = byte.arg >> 1;
            bool _rvalue = byte.arg % 2 == 1;
            auto& attr = frame->co->names[name];
            PyVar obj = frame->pop_value(this);
            AttrRef ref = AttrRef(obj, NameRef(attr));
            if(_rvalue) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_BUILD_INDEX: {
            PyVar index = frame->pop_value(this);
            auto ref = IndexRef(frame->pop_value(this), index);
            if(byte.arg == 0) frame->push(PyRef(ref));
            else frame->push(ref.get(this, frame));
        } continue;
        case OP_FAST_INDEX: case OP_FAST_INDEX_REF: {
            auto& a = frame->co->names[byte.arg & 0xFFFF];
            auto& x = frame->co->names[(byte.arg >> 16) & 0xFFFF];
            auto ref = IndexRef(NameRef(a).get(this, frame), NameRef(x).get(this, frame));
            if(byte.op == OP_FAST_INDEX) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_STORE_REF: {
            // PyVar obj = frame->pop_value(this);
            // PyVarRef r = frame->pop();
            // PyRef_AS_C(r)->set(this, frame, std::move(obj));
            PyRef_AS_C(frame->top_1())->set(this, frame, frame->top_value(this));
            frame->_pop(); frame->_pop();
        } continue;
        case OP_DELETE_REF: 
            PyRef_AS_C(frame->top())->del(this, frame);
            frame->_pop();
            continue;
        case OP_BUILD_SMART_TUPLE: {
            pkpy::Args items = frame->pop_n_reversed(byte.arg);
            bool done = false;
            for(int i=0; i<items.size(); i++){
                if(!is_type(items[i], tp_ref)) {
                    done = true;
                    for(int j=i; j<items.size(); j++) frame->try_deref(this, items[j]);
                    frame->push(PyTuple(std::move(items)));
                    break;
                }
            }
            if(!done) frame->push(PyRef(TupleRef(std::move(items))));
        } continue;
        case OP_BUILD_STRING: {
            pkpy::Args items = frame->pop_n_values_reversed(this, byte.arg);
            StrStream ss;
            for(int i=0; i<items.size(); i++) ss << PyStr_AS_C(asStr(items[i]));
            frame->push(PyStr(ss.str()));
        } continue;
        case OP_LOAD_EVAL_FN: frame->push(builtins->attr(m_eval)); continue;
        case OP_LIST_APPEND: {
            pkpy::Args args(2);
            args[1] = frame->pop_value(this);            // obj
            args[0] = frame->top_value_offset(this, -2);     // list
            fast_call(m_append, std::move(args));
        } continue;
        case OP_BUILD_CLASS: {
            const Str& clsName = frame->co->names[byte.arg].first;
            PyVar clsBase = frame->pop_value(this);
            if(clsBase == None) clsBase = _t(tp_object);
            check_type(clsBase, tp_type);
            PyVar cls = new_type_object(frame->_module, clsName, clsBase);
            while(true){
                PyVar fn = frame->pop_value(this);
                if(fn == None) break;
                const pkpy::Function& f = PyFunction_AS_C(fn);
                setattr(cls, f.name, fn);
            }
        } continue;
        case OP_RETURN_VALUE: return frame->pop_value(this);
        case OP_PRINT_EXPR: {
            const PyVar expr = frame->top_value(this);
            if(expr == None) continue;
            *_stdout << PyStr_AS_C(asRepr(expr)) << '\n';
        } continue;
        case OP_POP_TOP: frame->_pop(); continue;
        case OP_BINARY_OP: {
            pkpy::Args args(2);
            args[1] = frame->pop();
            args[0] = frame->top();
            frame->top() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_BITWISE_OP: {
            pkpy::Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_INPLACE_BINARY_OP: {
            pkpy::Args args(2);
            args[1] = frame->pop();
            args[0] = frame->top_value(this);
            PyVar ret = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_INPLACE_BITWISE_OP: {
            pkpy::Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            PyVar ret = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_COMPARE_OP: {
            pkpy::Args args(2);
            args[1] = frame->pop();
            args[0] = frame->top();
            frame->top() = fast_call(CMP_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_IS_OP: {
            PyVar rhs = frame->pop_value(this);
            bool ret_c = rhs == frame->top_value(this);
            if(byte.arg == 1) ret_c = !ret_c;
            frame->top() = PyBool(ret_c);
        } continue;
        case OP_CONTAINS_OP: {
            PyVar rhs = frame->pop_value(this);
            bool ret_c = PyBool_AS_C(call(rhs, __contains__, pkpy::one_arg(frame->pop_value(this))));
            if(byte.arg == 1) ret_c = !ret_c;
            frame->push(PyBool(ret_c));
        } continue;
        case OP_UNARY_NEGATIVE:
            frame->top() = num_negated(frame->top_value(this));
            continue;
        case OP_UNARY_NOT: {
            PyVar obj = frame->pop_value(this);
            const PyVar& obj_bool = asBool(obj);
            frame->push(PyBool(!PyBool_AS_C(obj_bool)));
        } continue;
        case OP_POP_JUMP_IF_FALSE:
            if(!PyBool_AS_C(asBool(frame->pop_value(this)))) frame->jump_abs(byte.arg);
            continue;
        case OP_LOAD_NONE: frame->push(None); continue;
        case OP_LOAD_TRUE: frame->push(True); continue;
        case OP_LOAD_FALSE: frame->push(False); continue;
        case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); continue;
        case OP_ASSERT: {
            PyVar _msg = frame->pop_value(this);
            Str msg = PyStr_AS_C(asStr(_msg));
            PyVar expr = frame->pop_value(this);
            if(asBool(expr) != True) _error("AssertionError", msg);
        } continue;
        case OP_EXCEPTION_MATCH: {
            const auto& e = PyException_AS_C(frame->top());
            Str name = frame->co->names[byte.arg].first;
            frame->push(PyBool(e.match_type(name)));
        } continue;
        case OP_RAISE: {
            PyVar obj = frame->pop_value(this);
            Str msg = obj == None ? "" : PyStr_AS_C(asStr(obj));
            Str type = frame->co->names[byte.arg].first;
            _error(type, msg);
        } continue;
        case OP_RE_RAISE: _raise(); continue;
        case OP_BUILD_LIST:
            frame->push(PyList(frame->pop_n_values_reversed(this, byte.arg).move_to_list()));
            continue;
        case OP_BUILD_MAP: {
            pkpy::Args items = frame->pop_n_values_reversed(this, byte.arg*2);
            PyVar obj = call(builtins->attr("dict"));
            for(int i=0; i<items.size(); i+=2){
                call(obj, __setitem__, pkpy::two_args(items[i], items[i+1]));
            }
            frame->push(obj);
        } continue;
        case OP_BUILD_SET: {
            PyVar list = PyList(
                frame->pop_n_values_reversed(this, byte.arg).move_to_list()
            );
            PyVar obj = call(builtins->attr("set"), pkpy::one_arg(list));
            frame->push(obj);
        } continue;
        case OP_DUP_TOP_VALUE: frame->push(frame->top_value(this)); continue;
        case OP_CALL: {
            int ARGC = byte.arg & 0xFFFF;
            int KWARGC = (byte.arg >> 16) & 0xFFFF;
            pkpy::Args kwargs(0);
            if(KWARGC > 0) kwargs = frame->pop_n_values_reversed(this, KWARGC*2);
            pkpy::Args args = frame->pop_n_values_reversed(this, ARGC);
            PyVar callable = frame->pop_value(this);
            PyVar ret = call(callable, std::move(args), kwargs, true);
            if(ret == _py_op_call) return ret;
            frame->push(std::move(ret));
        } continue;
        case OP_JUMP_ABSOLUTE: frame->jump_abs(byte.arg); continue;
        case OP_SAFE_JUMP_ABSOLUTE: frame->jump_abs_safe(byte.arg); continue;
        case OP_GOTO: {
            const Str& label = frame->co->names[byte.arg].first;
            int* target = frame->co->labels.try_get(label);
            if(target == nullptr) _error("KeyError", "label '" + label + "' not found");
            frame->jump_abs_safe(*target);
        } continue;
        case OP_GET_ITER: {
            PyVar obj = frame->pop_value(this);
            PyVar iter = asIter(obj);
            check_type(frame->top(), tp_ref);
            PyIter_AS_C(iter)->loop_var = frame->pop();
            frame->push(std::move(iter));
        } continue;
        case OP_FOR_ITER: {
            auto& it = PyIter_AS_C(frame->top());
            PyVar obj = it->next();
            if(obj != nullptr){
                PyRef_AS_C(it->loop_var)->set(this, frame, std::move(obj));
            }else{
                int blockEnd = frame->co->blocks[byte.block].end;
                frame->jump_abs_safe(blockEnd);
            }
        } continue;
        case OP_LOOP_CONTINUE: {
            int blockStart = frame->co->blocks[byte.block].start;
            frame->jump_abs(blockStart);
        } continue;
        case OP_LOOP_BREAK: {
            int blockEnd = frame->co->blocks[byte.block].end;
            frame->jump_abs_safe(blockEnd);
        } continue;
        case OP_JUMP_IF_FALSE_OR_POP: {
            const PyVar expr = frame->top_value(this);
            if(asBool(expr)==False) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_JUMP_IF_TRUE_OR_POP: {
            const PyVar expr = frame->top_value(this);
            if(asBool(expr)==True) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_BUILD_SLICE: {
            PyVar stop = frame->pop_value(this);
            PyVar start = frame->pop_value(this);
            pkpy::Slice s;
            if(start != None) {check_type(start, tp_int); s.start = (int)PyInt_AS_C(start);}
            if(stop != None) {check_type(stop, tp_int); s.stop = (int)PyInt_AS_C(stop);}
            frame->push(PySlice(s));
        } continue;
        case OP_IMPORT_NAME: {
            const Str& name = frame->co->names[byte.arg].first;
            auto it = _modules.find(name);
            if(it == _modules.end()){
                auto it2 = _lazy_modules.find(name);
                if(it2 == _lazy_modules.end()){
                    _error("ImportError", "module " + name.escape(true) + " not found");
                }else{
                    const Str& source = it2->second;
                    CodeObject_ code = compile(source, name, EXEC_MODE);
                    PyVar _m = new_module(name);
                    _exec(code, _m);
                    frame->push(_m);
                    _lazy_modules.erase(it2);
                }
            }else{
                frame->push(it->second);
            }
        } continue;
        case OP_YIELD_VALUE: return _py_op_yield;
        // TODO: using "goto" inside with block may cause __exit__ not called
        case OP_WITH_ENTER: call(frame->pop_value(this), __enter__); continue;
        case OP_WITH_EXIT: call(frame->pop_value(this), __exit__); continue;
        case OP_TRY_BLOCK_ENTER: frame->on_try_block_enter(); continue;
        case OP_TRY_BLOCK_EXIT: frame->on_try_block_exit(); continue;
        default: throw std::runtime_error(Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
        }
    }

    if(frame->co->src->mode == EVAL_MODE || frame->co->src->mode == JSON_MODE){
        if(frame->_data.size() != 1) throw std::runtime_error("_data.size() != 1 in EVAL/JSON_MODE");
        return frame->pop_value(this);
    }

    if(!frame->_data.empty()) throw std::runtime_error("_data.size() != 0 in EXEC_MODE");
    return None;
}
