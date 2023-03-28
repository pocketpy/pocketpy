#pragma once

#include "vm.h"
#include "ref.h"

namespace pkpy{

inline PyObject* VM::run_frame(Frame* frame){
    while(frame->has_next_bytecode()){
        const Bytecode& byte = frame->next_bytecode();
        switch (byte.op)
        {
        case OP_NO_OP: continue;
        case OP_SETUP_DECORATOR: continue;
        case OP_LOAD_CONST: frame->push(frame->co->consts[byte.arg]); continue;
        case OP_LOAD_FUNCTION: {
            PyObject* obj = frame->co->consts[byte.arg];
            Function f = CAST(Function, obj);  // copy
            f._module = frame->_module;
            frame->push(VAR(f));
        } continue;
        case OP_SETUP_CLOSURE: {
            Function& f = CAST(Function&, frame->top());    // reference
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
        case OP_BUILD_ATTR_REF: case OP_BUILD_ATTR: {
            auto& attr = frame->co->names[byte.arg];
            PyObject* obj = frame->pop_value(this);
            AttrRef ref = AttrRef(obj, NameRef(attr));
            if(byte.op == OP_BUILD_ATTR) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_BUILD_INDEX: {
            PyObject* index = frame->pop_value(this);
            auto ref = IndexRef(frame->pop_value(this), index);
            if(byte.arg > 0) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_FAST_INDEX: case OP_FAST_INDEX_REF: {
            auto& a = frame->co->names[byte.arg & 0xFFFF];
            auto& x = frame->co->names[(byte.arg >> 16) & 0xFFFF];
            auto ref = IndexRef(NameRef(a).get(this, frame), NameRef(x).get(this, frame));
            if(byte.op == OP_FAST_INDEX) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } continue;
        case OP_ROT_TWO: ::std::swap(frame->top(), frame->top_1()); continue;
        case OP_STORE_REF: {
            PyRef_AS_C(frame->top_1())->set(this, frame, frame->top_value(this));
            frame->_pop(); frame->_pop();
        } continue;
        case OP_DELETE_REF: 
            PyRef_AS_C(frame->top())->del(this, frame);
            frame->_pop();
            continue;
        case OP_BUILD_TUPLE: {
            Args items = frame->pop_n_values_reversed(this, byte.arg);
            frame->push(VAR(std::move(items)));
        } continue;
        case OP_BUILD_TUPLE_REF: {
            Args items = frame->pop_n_reversed(byte.arg);
            frame->push(PyRef(TupleRef(std::move(items))));
        } continue;
        case OP_BUILD_STRING: {
            Args items = frame->pop_n_values_reversed(this, byte.arg);
            StrStream ss;
            for(int i=0; i<items.size(); i++) ss << CAST(Str, asStr(items[i]));
            frame->push(VAR(ss.str()));
        } continue;
        case OP_LOAD_EVAL_FN: frame->push(builtins->attr(m_eval)); continue;
        case OP_BEGIN_CLASS: {
            auto& name = frame->co->names[byte.arg];
            PyObject* clsBase = frame->pop_value(this);
            if(clsBase == None) clsBase = _t(tp_object);
            check_type(clsBase, tp_type);
            PyObject* cls = new_type_object(frame->_module, name.first, OBJ_GET(Type, clsBase));
            frame->push(cls);
        } continue;
        case OP_END_CLASS: {
            PyObject* cls = frame->pop();
            cls->attr()._try_perfect_rehash();
        }; continue;
        case OP_STORE_CLASS_ATTR: {
            auto& name = frame->co->names[byte.arg];
            PyObject* obj = frame->pop_value(this);
            PyObject* cls = frame->top();
            cls->attr().set(name.first, std::move(obj));
        } continue;
        case OP_RETURN_VALUE: return frame->pop_value(this);
        case OP_PRINT_EXPR: {
            PyObject* expr = frame->top_value(this);
            if(expr != None) *_stdout << CAST(Str, asRepr(expr)) << '\n';
        } continue;
        case OP_POP_TOP: frame->_pop(); continue;
        case OP_BINARY_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_BITWISE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_INPLACE_BINARY_OP: {
            Args args(2);
            args[1] = frame->pop();
            args[0] = frame->top_value(this);
            PyObject* ret = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_INPLACE_BITWISE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            PyObject* ret = fast_call(BITWISE_SPECIAL_METHODS[byte.arg], std::move(args));
            PyRef_AS_C(frame->top())->set(this, frame, std::move(ret));
            frame->_pop();
        } continue;
        case OP_COMPARE_OP: {
            Args args(2);
            args[1] = frame->pop_value(this);
            args[0] = frame->top_value(this);
            frame->top() = fast_call(CMP_SPECIAL_METHODS[byte.arg], std::move(args));
        } continue;
        case OP_IS_OP: {
            PyObject* rhs = frame->pop_value(this);
            bool ret_c = rhs == frame->top_value(this);
            if(byte.arg == 1) ret_c = !ret_c;
            frame->top() = VAR(ret_c);
        } continue;
        case OP_CONTAINS_OP: {
            PyObject* rhs = frame->pop_value(this);
            bool ret_c = CAST(bool, call(rhs, __contains__, Args{frame->pop_value(this)}));
            if(byte.arg == 1) ret_c = !ret_c;
            frame->push(VAR(ret_c));
        } continue;
        case OP_UNARY_NEGATIVE:
            frame->top() = num_negated(frame->top_value(this));
            continue;
        case OP_UNARY_NOT: {
            PyObject* obj = frame->pop_value(this);
            PyObject* obj_bool = asBool(obj);
            frame->push(VAR(!_CAST(bool, obj_bool)));
        } continue;
        case OP_POP_JUMP_IF_FALSE:
            if(!_CAST(bool, asBool(frame->pop_value(this)))) frame->jump_abs(byte.arg);
            continue;
        case OP_LOAD_NONE: frame->push(None); continue;
        case OP_LOAD_TRUE: frame->push(True); continue;
        case OP_LOAD_FALSE: frame->push(False); continue;
        case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); continue;
        case OP_ASSERT: {
            PyObject* _msg = frame->pop_value(this);
            Str msg = CAST(Str, asStr(_msg));
            PyObject* expr = frame->pop_value(this);
            if(asBool(expr) != True) _error("AssertionError", msg);
        } continue;
        case OP_EXCEPTION_MATCH: {
            const auto& e = CAST(Exception&, frame->top());
            StrName name = frame->co->names[byte.arg].first;
            frame->push(VAR(e.match_type(name)));
        } continue;
        case OP_RAISE: {
            PyObject* obj = frame->pop_value(this);
            Str msg = obj == None ? "" : CAST(Str, asStr(obj));
            StrName type = frame->co->names[byte.arg].first;
            _error(type, msg);
        } continue;
        case OP_RE_RAISE: _raise(); continue;
        case OP_BUILD_LIST:
            frame->push(VAR(frame->pop_n_values_reversed(this, byte.arg).to_list()));
            continue;
        case OP_BUILD_MAP: {
            Args items = frame->pop_n_values_reversed(this, byte.arg*2);
            PyObject* obj = call(builtins->attr("dict"), no_arg());
            for(int i=0; i<items.size(); i+=2){
                call(obj, __setitem__, Args{items[i], items[i+1]});
            }
            frame->push(obj);
        } continue;
        case OP_BUILD_SET: {
            PyObject* list = VAR(
                frame->pop_n_values_reversed(this, byte.arg).to_list()
            );
            PyObject* obj = call(builtins->attr("set"), Args{list});
            frame->push(obj);
        } continue;
        case OP_LIST_APPEND: {
            PyObject* obj = frame->pop_value(this);
            List& list = CAST(List&, frame->top_1());
            list.push_back(std::move(obj));
        } continue;
        case OP_MAP_ADD: {
            PyObject* value = frame->pop_value(this);
            PyObject* key = frame->pop_value(this);
            call(frame->top_1(), __setitem__, Args{key, value});
        } continue;
        case OP_SET_ADD: {
            PyObject* obj = frame->pop_value(this);
            call(frame->top_1(), "add", Args{obj});
        } continue;
        case OP_DUP_TOP_VALUE: frame->push(frame->top_value(this)); continue;
        case OP_UNARY_STAR: {
            if(byte.arg > 0){   // rvalue
                frame->top() = VAR(StarWrapper(frame->top_value(this), true));
            }else{
                PyRef_AS_C(frame->top()); // check ref
                frame->top() = VAR(StarWrapper(frame->top(), false));
            }
        } continue;
        case OP_CALL_KWARGS_UNPACK: case OP_CALL_KWARGS: {
            int ARGC = byte.arg & 0xFFFF;
            int KWARGC = (byte.arg >> 16) & 0xFFFF;
            Args kwargs = frame->pop_n_values_reversed(this, KWARGC*2);
            Args args = frame->pop_n_values_reversed(this, ARGC);
            if(byte.op == OP_CALL_KWARGS_UNPACK) unpack_args(args);
            PyObject* callable = frame->pop_value(this);
            PyObject* ret = call(callable, std::move(args), kwargs, true);
            if(ret == _py_op_call) return ret;
            frame->push(std::move(ret));
        } continue;
        case OP_CALL_UNPACK: case OP_CALL: {
            Args args = frame->pop_n_values_reversed(this, byte.arg);
            if(byte.op == OP_CALL_UNPACK) unpack_args(args);
            PyObject* callable = frame->pop_value(this);
            PyObject* ret = call(callable, std::move(args), no_arg(), true);
            if(ret == _py_op_call) return ret;
            frame->push(std::move(ret));
        } continue;
        case OP_JUMP_ABSOLUTE: frame->jump_abs(byte.arg); continue;
        case OP_SAFE_JUMP_ABSOLUTE: frame->jump_abs_safe(byte.arg); continue;
        case OP_GOTO: {
            StrName label = frame->co->names[byte.arg].first;
            auto it = frame->co->labels.find(label);
            if(it == frame->co->labels.end()) _error("KeyError", "label " + label.str().escape(true) + " not found");
            frame->jump_abs_safe(it->second);
        } continue;
        case OP_GET_ITER: {
            PyObject* obj = frame->pop_value(this);
            PyObject* iter = asIter(obj);
            check_type(frame->top(), tp_ref);
            PyIter_AS_C(iter)->loop_var = frame->pop();
            frame->push(std::move(iter));
        } continue;
        case OP_FOR_ITER: {
            BaseIter* it = PyIter_AS_C(frame->top());
            PyObject* obj = it->next();
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
            PyObject* expr = frame->top_value(this);
            if(asBool(expr)==False) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_JUMP_IF_TRUE_OR_POP: {
            PyObject* expr = frame->top_value(this);
            if(asBool(expr)==True) frame->jump_abs(byte.arg);
            else frame->pop_value(this);
        } continue;
        case OP_BUILD_SLICE: {
            PyObject* stop = frame->pop_value(this);
            PyObject* start = frame->pop_value(this);
            Slice s;
            if(start != None) { s.start = CAST(int, start);}
            if(stop != None) { s.stop = CAST(int, stop);}
            frame->push(VAR(s));
        } continue;
        case OP_IMPORT_NAME: {
            StrName name = frame->co->names[byte.arg].first;
            PyObject* ext_mod = _modules.try_get(name);
            if(ext_mod == nullptr){
                Str source;
                auto it2 = _lazy_modules.find(name);
                if(it2 == _lazy_modules.end()){
                    bool ok = false;
                    source = _read_file_cwd(name.str() + ".py", &ok);
                    if(!ok) _error("ImportError", "module " + name.str().escape(true) + " not found");
                }else{
                    source = it2->second;
                    _lazy_modules.erase(it2);
                }
                CodeObject_ code = compile(source, name.str(), EXEC_MODE);
                PyObject* new_mod = new_module(name);
                _exec(code, new_mod);
                frame->push(new_mod);
                new_mod->attr()._try_perfect_rehash();
            }else{
                frame->push(ext_mod);
            }
        } continue;
        case OP_STORE_ALL_NAMES: {
            PyObject* obj = frame->pop_value(this);
            for(auto& [name, value]: obj->attr().items()){
                Str s = name.str();
                if(s.empty() || s[0] == '_') continue;
                frame->f_globals().set(name, value);
            }
        }; continue;
        case OP_YIELD_VALUE: return _py_op_yield;
        // TODO: using "goto" inside with block may cause __exit__ not called
        case OP_WITH_ENTER: call(frame->pop_value(this), __enter__, no_arg()); continue;
        case OP_WITH_EXIT: call(frame->pop_value(this), __exit__, no_arg()); continue;
        case OP_TRY_BLOCK_ENTER: frame->on_try_block_enter(); continue;
        case OP_TRY_BLOCK_EXIT: frame->on_try_block_exit(); continue;
        default: throw std::runtime_error(Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
        }
    }

    if(frame->co->src->mode == EVAL_MODE || frame->co->src->mode == JSON_MODE){
        if(frame->_data.size() != 1) throw std::runtime_error("_data.size() != 1 in EVAL/JSON_MODE");
        return frame->pop_value(this);
    }
#if PK_EXTRA_CHECK
    if(!frame->_data.empty()) throw std::runtime_error("_data.size() != 0 in EXEC_MODE");
#endif
    return None;
}

} // namespace pkpy