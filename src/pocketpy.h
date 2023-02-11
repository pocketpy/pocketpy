#pragma once

#include "vm.h"
#include "compiler.h"
#include "repl.h"
#include "iter.h"

#define CPP_LAMBDA(x) ([](VM* vm, const pkpy::Args& args) { return x; })
#define CPP_NOT_IMPLEMENTED() ([](VM* vm, const pkpy::Args& args) { vm->NotImplementedError(); return vm->None; })

CodeObject_ VM::compile(Str source, Str filename, CompileMode mode) {
    Compiler compiler(this, source.c_str(), filename, mode);
    try{
        return compiler.compile();
    }catch(pkpy::Exception& e){
        _error(e);
        return nullptr;
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, const pkpy::Args& args){                 \
        if(args[0]->is_type(vm->tp_int) && args[1]->is_type(vm->tp_int)){                             \
            return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1]));                       \
        }else{                                                                                          \
            return vm->PyFloat(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                 \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, const pkpy::Args& args){                 \
        bool _0 = args[0]->is_type(vm->tp_int) || args[0]->is_type(vm->tp_float);                     \
        bool _1 = args[1]->is_type(vm->tp_int) || args[1]->is_type(vm->tp_float);                     \
        if(!_0 || !_1){                                                                                 \
            if constexpr(is_eq) return vm->PyBool(args[0].get() op args[1].get());                      \
            vm->TypeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        return vm->PyBool(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                      \
    });
    

void init_builtins(VM* _vm) {
    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

    BIND_NUM_LOGICAL_OPT(__lt__, <, false)
    BIND_NUM_LOGICAL_OPT(__le__, <=, false)
    BIND_NUM_LOGICAL_OPT(__gt__, >, false)
    BIND_NUM_LOGICAL_OPT(__ge__, >=, false)
    BIND_NUM_LOGICAL_OPT(__eq__, ==, true)
    BIND_NUM_LOGICAL_OPT(__ne__, !=, true)

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bind_builtin_func<1>("__sys_stdout_write", [](VM* vm, const pkpy::Args& args) {
        (*vm->_stdout) << vm->PyStr_AS_C(args[0]);
        return vm->None;
    });

    _vm->bind_builtin_func<0>("super", [](VM* vm, const pkpy::Args& args) {
        auto it = vm->top_frame()->f_locals().find(m_self);
        if(it == vm->top_frame()->f_locals().end()) vm->TypeError("super() can only be called in a class method");
        return vm->new_object(vm->tp_super, it->second);
    });

    _vm->bind_builtin_func<1>("eval", [](VM* vm, const pkpy::Args& args) {
        CodeObject_ code = vm->compile(vm->PyStr_AS_C(args[0]), "<eval>", EVAL_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    _vm->bind_builtin_func<1>("exec", [](VM* vm, const pkpy::Args& args) {
        CodeObject_ code = vm->compile(vm->PyStr_AS_C(args[0]), "<exec>", EXEC_MODE);
        vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, const pkpy::Args& args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit((int)vm->PyInt_AS_C(args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", CPP_LAMBDA(vm->asRepr(args[0])));
    _vm->bind_builtin_func<1>("hash", CPP_LAMBDA(vm->PyInt(vm->hash(args[0]))));
    _vm->bind_builtin_func<1>("len", CPP_LAMBDA(vm->call(args[0], __len__, pkpy::no_arg())));

    _vm->bind_builtin_func<1>("chr", [](VM* vm, const pkpy::Args& args) {
        i64 i = vm->PyInt_AS_C(args[0]);
        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
        return vm->PyStr(std::string(1, (char)i));
    });

    _vm->bind_builtin_func<1>("ord", [](VM* vm, const pkpy::Args& args) {
        Str s = vm->PyStr_AS_C(args[0]);
        if (s.size() != 1) vm->TypeError("ord() expected an ASCII character");
        return vm->PyInt((i64)(s.c_str()[0]));
    });

    _vm->bind_builtin_func<2>("hasattr", [](VM* vm, const pkpy::Args& args) {
        return vm->PyBool(vm->getattr(args[0], vm->PyStr_AS_C(args[1]), false) != nullptr);
    });

    _vm->bind_builtin_func<3>("setattr", [](VM* vm, const pkpy::Args& args) {
        PyVar obj = args[0];
        vm->setattr(obj, vm->PyStr_AS_C(args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_builtin_func<2>("getattr", [](VM* vm, const pkpy::Args& args) {
        Str name = vm->PyStr_AS_C(args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, const pkpy::Args& args) {
        std::stringstream ss;
        ss << std::hex << vm->PyInt_AS_C(args[0]);
        return vm->PyStr("0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, const pkpy::Args& args) {
        std::vector<Str> names;
        for (auto& [k, _] : args[0]->attribs) names.push_back(k);
        for (auto& [k, _] : args[0]->type->attribs) {
            if (k.find("__") == 0) continue;
            if (std::find(names.begin(), names.end(), k) == names.end()) names.push_back(k);
        }
        pkpy::List ret;
        for (const auto& name : names) ret.push_back(vm->PyStr(name));
        std::sort(ret.begin(), ret.end(), [vm](const PyVar& a, const PyVar& b) {
            return vm->PyStr_AS_C(a) < vm->PyStr_AS_C(b);
        });
        return vm->PyList(ret);
    });

    _vm->bind_method<0>("object", "__repr__", [](VM* vm, const pkpy::Args& args) {
        PyVar _self = args[0];
        std::stringstream ss;
        ss << std::hex << (uintptr_t)_self.get();
        Str s = "<" + OBJ_TP_NAME(_self) + " object at 0x" + ss.str() + ">";
        return vm->PyStr(s);
    });

    _vm->bind_method<1>("object", "__eq__", CPP_LAMBDA(vm->PyBool(args[0] == args[1])));
    _vm->bind_method<1>("object", "__ne__", CPP_LAMBDA(vm->PyBool(args[0] != args[1])));

    _vm->bind_static_method<1>("type", "__new__", CPP_LAMBDA(args[0]->type));

    _vm->bind_static_method<-1>("range", "__new__", [](VM* vm, const pkpy::Args& args) {
        pkpy::Range r;
        switch (args.size()) {
            case 1: r.stop = vm->PyInt_AS_C(args[0]); break;
            case 2: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); break;
            case 3: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); r.step = vm->PyInt_AS_C(args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return vm->PyRange(r);
    });

    _vm->bind_method<0>("range", "__iter__", CPP_LAMBDA(
        vm->PyIter(pkpy::make_shared<BaseIter, RangeIter>(vm, args[0]))
    ));

    _vm->bind_method<0>("NoneType", "__repr__", CPP_LAMBDA(vm->PyStr("None")));
    _vm->bind_method<0>("NoneType", "__json__", CPP_LAMBDA(vm->PyStr("null")));

    _vm->_bind_methods<1>({"int", "float"}, "__truediv__", [](VM* vm, const pkpy::Args& args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->ZeroDivisionError();
        return vm->PyFloat(vm->num_to_float(args[0]) / rhs);
    });

    _vm->_bind_methods<1>({"int", "float"}, "__pow__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->tp_int) && args[1]->is_type(vm->tp_int)){
            return vm->PyInt((i64)round(pow(vm->PyInt_AS_C(args[0]), vm->PyInt_AS_C(args[1]))));
        }else{
            return vm->PyFloat((f64)pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bind_static_method<1>("int", "__new__", [](VM* vm, const pkpy::Args& args) {
        if (args[0]->is_type(vm->tp_int)) return args[0];
        if (args[0]->is_type(vm->tp_float)) return vm->PyInt((i64)vm->PyFloat_AS_C(args[0]));
        if (args[0]->is_type(vm->tp_bool)) return vm->PyInt(vm->PyBool_AS_C(args[0]) ? 1 : 0);
        if (args[0]->is_type(vm->tp_str)) {
            const Str& s = vm->PyStr_AS_C(args[0]);
            try{
                size_t parsed = 0;
                i64 val = std::stoll(s, &parsed, 10);
                if(parsed != s.size()) throw std::invalid_argument("");
                return vm->PyInt(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for int(): '" + s + "'");
            }
        }
        vm->TypeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<1>("int", "__floordiv__", [](VM* vm, const pkpy::Args& args) {
        i64 rhs = vm->PyInt_AS_C(args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args[0]) / rhs);
    });

    _vm->bind_method<1>("int", "__mod__", [](VM* vm, const pkpy::Args& args) {
        i64 rhs = vm->PyInt_AS_C(args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args[0]) % rhs);
    });

    _vm->bind_method<0>("int", "__repr__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

    _vm->bind_method<0>("int", "__json__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

#define __INT_BITWISE_OP(name,op) \
    _vm->bind_method<1>("int", #name, [](VM* vm, const pkpy::Args& args) {                    \
        return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1]));     \
    });

    __INT_BITWISE_OP(__lshift__, <<)
    __INT_BITWISE_OP(__rshift__, >>)
    __INT_BITWISE_OP(__and__, &)
    __INT_BITWISE_OP(__or__, |)
    __INT_BITWISE_OP(__xor__, ^)

#undef __INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bind_static_method<1>("float", "__new__", [](VM* vm, const pkpy::Args& args) {
        if (args[0]->is_type(vm->tp_int)) return vm->PyFloat((f64)vm->PyInt_AS_C(args[0]));
        if (args[0]->is_type(vm->tp_float)) return args[0];
        if (args[0]->is_type(vm->tp_bool)) return vm->PyFloat(vm->PyBool_AS_C(args[0]) ? 1.0 : 0.0);
        if (args[0]->is_type(vm->tp_str)) {
            const Str& s = vm->PyStr_AS_C(args[0]);
            if(s == "inf") return vm->PyFloat(INFINITY);
            if(s == "-inf") return vm->PyFloat(-INFINITY);
            try{
                f64 val = std::stod(s);
                return vm->PyFloat(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->TypeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<0>("float", "__repr__", [](VM* vm, const pkpy::Args& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) return vm->PyStr(std::to_string(val));
        StrStream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return vm->PyStr(s);
    });

    _vm->bind_method<0>("float", "__json__", [](VM* vm, const pkpy::Args& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
        return vm->PyStr(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bind_static_method<1>("str", "__new__", CPP_LAMBDA(vm->asStr(args[0])));

    _vm->bind_method<1>("str", "__add__", [](VM* vm, const pkpy::Args& args) {
        const Str& lhs = vm->PyStr_AS_C(args[0]);
        const Str& rhs = vm->PyStr_AS_C(args[1]);
        return vm->PyStr(lhs + rhs);
    });

    _vm->bind_method<0>("str", "__len__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyInt(_self.u8_length());
    });

    _vm->bind_method<1>("str", "__contains__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        const Str& _other = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_other) != Str::npos);
    });

    _vm->bind_method<0>("str", "__str__", CPP_LAMBDA(args[0]));

    _vm->bind_method<0>("str", "__iter__", CPP_LAMBDA(
        vm->PyIter(pkpy::make_shared<BaseIter, StringIter>(vm, args[0]))
    ));

    _vm->bind_method<0>("str", "__repr__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.escape(true));
    });

    _vm->bind_method<0>("str", "__json__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.escape(false));
    });

    _vm->bind_method<1>("str", "__eq__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->tp_str) && args[1]->is_type(vm->tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) == vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bind_method<1>("str", "__ne__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->tp_str) && args[1]->is_type(vm->tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) != vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] != args[1]);
    });

    _vm->bind_method<1>("str", "__getitem__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self (vm->PyStr_AS_C(args[0]));

        if(args[1]->is_type(vm->tp_slice)){
            pkpy::Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.u8_length());
            return vm->PyStr(_self.u8_substr(s.start, s.stop));
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.u8_length());
        return vm->PyStr(_self.u8_getitem(_index));
    });

    _vm->bind_method<1>("str", "__gt__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self (vm->PyStr_AS_C(args[0]));
        const Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self > _obj);
    });

    _vm->bind_method<1>("str", "__lt__", [](VM* vm, const pkpy::Args& args) {
        const Str& _self (vm->PyStr_AS_C(args[0]));
        const Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self < _obj);
    });

    _vm->bind_method<2>("str", "replace", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        const Str& _old = vm->PyStr_AS_C(args[1]);
        const Str& _new = vm->PyStr_AS_C(args[2]);
        Str _copy = _self;
        // replace all occurences of _old with _new in _copy
        size_t pos = 0;
        while ((pos = _copy.find(_old, pos)) != std::string::npos) {
            _copy.replace(pos, _old.length(), _new);
            pos += _new.length();
        }
        return vm->PyStr(_copy);
    });

    _vm->bind_method<1>("str", "startswith", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        const Str& _prefix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_prefix) == 0);
    });

    _vm->bind_method<1>("str", "endswith", [](VM* vm, const pkpy::Args& args) {
        const Str& _self = vm->PyStr_AS_C(args[0]);
        const Str& _suffix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.rfind(_suffix) == _self.length() - _suffix.length());
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, const pkpy::Args& args) {
        const Str& self = vm->PyStr_AS_C(args[0]);
        StrStream ss;
        if(args[1]->is_type(vm->tp_list)){
            const pkpy::List& a = vm->PyList_AS_C(args[1]);
            for(int i = 0; i < a.size(); i++){
                if(i > 0) ss << self;
                ss << vm->PyStr_AS_C(vm->asStr(a[i]));
            }
        }else if(args[1]->is_type(vm->tp_tuple)){
            const pkpy::Tuple& a = vm->PyTuple_AS_C(args[1]);
            for(int i = 0; i < a.size(); i++){
                if(i > 0) ss << self;
                ss << vm->PyStr_AS_C(vm->asStr(a[i]));
            }
        }else{
            vm->TypeError("can only join a list or tuple");
        }
        return vm->PyStr(ss.str());
    });

    /************ PyList ************/
    _vm->bind_method<1>("list", "append", [](VM* vm, const pkpy::Args& args) {
        pkpy::List& self = vm->PyList_AS_C(args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<1>("list", "__mul__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::List& self = vm->PyList_AS_C(args[0]);
        int n = (int)vm->PyInt_AS_C(args[1]);
        pkpy::List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.insert(result.end(), self.begin(), self.end());
        return vm->PyList(std::move(result));
    });

    _vm->bind_method<2>("list", "insert", [](VM* vm, const pkpy::Args& args) {
        pkpy::List& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        if(_index < 0) _index += _self.size();
        if(_index < 0) _index = 0;
        if(_index > _self.size()) _index = _self.size();
        _self.insert(_self.begin() + _index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "clear", [](VM* vm, const pkpy::Args& args) {
        vm->PyList_AS_C(args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>("list", "copy", [](VM* vm, const pkpy::Args& args) {
        return vm->PyList(vm->PyList_AS_C(args[0]));
    });

    _vm->bind_method<1>("list", "__add__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::List& _self = vm->PyList_AS_C(args[0]);
        const pkpy::List& _obj = vm->PyList_AS_C(args[1]);
        pkpy::List _new_list = _self;
        _new_list.insert(_new_list.end(), _obj.begin(), _obj.end());
        return vm->PyList(_new_list);
    });

    _vm->bind_method<0>("list", "__len__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::List& _self = vm->PyList_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bind_method<0>("list", "__iter__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyIter(pkpy::make_shared<BaseIter, ArrayIter<pkpy::List>>(vm, args[0]));
    });

    _vm->bind_method<1>("list", "__getitem__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::List& self = vm->PyList_AS_C(args[0]);

        if(args[1]->is_type(vm->tp_slice)){
            pkpy::Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(self.size());
            pkpy::List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return vm->PyList(std::move(new_list));
        }

        int index = (int)vm->PyInt_AS_C(args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<2>("list", "__setitem__", [](VM* vm, const pkpy::Args& args) {
        pkpy::List& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.size());
        _self[_index] = args[2];
        return vm->None;
    });

    _vm->bind_method<1>("list", "__delitem__", [](VM* vm, const pkpy::Args& args) {
        pkpy::List& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.size());
        _self.erase(_self.begin() + _index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bind_static_method<1>("tuple", "__new__", [](VM* vm, const pkpy::Args& args) {
        pkpy::List _list = vm->PyList_AS_C(vm->call(vm->builtins->attribs["list"], args));
        return vm->PyTuple(std::move(_list));
    });

    _vm->bind_method<0>("tuple", "__iter__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyIter(pkpy::make_shared<BaseIter, ArrayIter<pkpy::Args>>(vm, args[0]));
    });

    _vm->bind_method<1>("tuple", "__getitem__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::Tuple& self = vm->PyTuple_AS_C(args[0]);

        if(args[1]->is_type(vm->tp_slice)){
            pkpy::Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(self.size());
            pkpy::List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return vm->PyTuple(std::move(new_list));
        }

        int index = (int)vm->PyInt_AS_C(args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<0>("tuple", "__len__", [](VM* vm, const pkpy::Args& args) {
        const pkpy::Tuple& self = vm->PyTuple_AS_C(args[0]);
        return vm->PyInt(self.size());
    });

    /************ PyBool ************/
    _vm->bind_static_method<1>("bool", "__new__", CPP_LAMBDA(vm->asBool(args[0])));

    _vm->bind_method<0>("bool", "__repr__", [](VM* vm, const pkpy::Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "True" : "False");
    });

    _vm->bind_method<0>("bool", "__json__", [](VM* vm, const pkpy::Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "true" : "false");
    });

    _vm->bind_method<1>("bool", "__xor__", [](VM* vm, const pkpy::Args& args) {
        bool _self = vm->PyBool_AS_C(args[0]);
        bool _obj = vm->PyBool_AS_C(args[1]);
        return vm->PyBool(_self ^ _obj);
    });

    _vm->bind_method<0>("ellipsis", "__repr__", CPP_LAMBDA(vm->PyStr("Ellipsis")));
}

#include "builtins.h"

#ifdef _WIN32
#define __EXPORT __declspec(dllexport)
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used))
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define __EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define __EXPORT
#endif


void add_module_time(VM* vm){
    PyVar mod = vm->new_module("time");
    vm->bind_func<0>(mod, "time", [](VM* vm, const pkpy::Args& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return vm->PyFloat(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void add_module_sys(VM* vm){
    PyVar mod = vm->new_module("sys");
    vm->setattr(mod, "version", vm->PyStr(PK_VERSION));

    vm->bind_func<1>(mod, "getrefcount", CPP_LAMBDA(vm->PyInt(args[0].use_count())));
    vm->bind_func<0>(mod, "getrecursionlimit", CPP_LAMBDA(vm->PyInt(vm->maxRecursionDepth)));

    vm->bind_func<1>(mod, "setrecursionlimit", [](VM* vm, const pkpy::Args& args) {
        vm->maxRecursionDepth = (int)vm->PyInt_AS_C(args[0]);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyVar mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, const pkpy::Args& args) {
        const Str& expr = vm->PyStr_AS_C(args[0]);
        CodeObject_ code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    vm->bind_func<1>(mod, "dumps", CPP_LAMBDA(vm->call(args[0], __json__)));
}

void add_module_math(VM* vm){
    PyVar mod = vm->new_module("math");
    vm->setattr(mod, "pi", vm->PyFloat(3.1415926535897932384));
    vm->setattr(mod, "e" , vm->PyFloat(2.7182818284590452354));

    vm->bind_func<1>(mod, "log", CPP_LAMBDA(vm->PyFloat(std::log(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log10", CPP_LAMBDA(vm->PyFloat(std::log10(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log2", CPP_LAMBDA(vm->PyFloat(std::log2(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sin", CPP_LAMBDA(vm->PyFloat(std::sin(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "cos", CPP_LAMBDA(vm->PyFloat(std::cos(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "tan", CPP_LAMBDA(vm->PyFloat(std::tan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isnan", CPP_LAMBDA(vm->PyBool(std::isnan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isinf", CPP_LAMBDA(vm->PyBool(std::isinf(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "fabs", CPP_LAMBDA(vm->PyFloat(std::fabs(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "floor", CPP_LAMBDA(vm->PyInt((i64)std::floor(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "ceil", CPP_LAMBDA(vm->PyInt((i64)std::ceil(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sqrt", CPP_LAMBDA(vm->PyFloat(std::sqrt(vm->num_to_float(args[0])))));
}

void add_module_dis(VM* vm){
    PyVar mod = vm->new_module("dis");
    vm->bind_func<1>(mod, "dis", [](VM* vm, const pkpy::Args& args) {
        CodeObject_ code = vm->PyFunction_AS_C(args[0])->code;
        (*vm->_stdout) << vm->disassemble(code);
        return vm->None;
    });
}

struct ReMatch {
    PY_CLASS(re, Match)

    i64 start;
    i64 end;
    std::smatch m;
    ReMatch(i64 start, i64 end, std::smatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_method<-1>(type, "__init__", CPP_NOT_IMPLEMENTED());
        vm->bind_method<0>(type, "start", CPP_LAMBDA(vm->PyInt(vm->py_cast<ReMatch>(args[0]).start)));
        vm->bind_method<0>(type, "end", CPP_LAMBDA(vm->PyInt(vm->py_cast<ReMatch>(args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, const pkpy::Args& args) {
            auto& self = vm->py_cast<ReMatch>(args[0]);
            return vm->PyTuple({ vm->PyInt(self.start), vm->PyInt(self.end) });
        });

        vm->bind_method<1>(type, "group", [](VM* vm, const pkpy::Args& args) {
            auto& self = vm->py_cast<ReMatch>(args[0]);
            int index = (int)vm->PyInt_AS_C(args[1]);
            index = vm->normalized_index(index, self.m.size());
            return vm->PyStr(self.m[index].str());
        });
    }
};

PyVar _regex_search(const Str& pattern, const Str& string, bool fromStart, VM* vm){
    std::regex re(pattern);
    std::smatch m;
    if(std::regex_search(string, m, re)){
        if(fromStart && m.position() != 0) return vm->None;
        i64 start = string._to_u8_index(m.position());
        i64 end = string._to_u8_index(m.position() + m.length());
        return vm->new_object<ReMatch>(start, end, m);
    }
    return vm->None;
};

void add_module_re(VM* vm){
    PyVar mod = vm->new_module("re");
    vm->register_class<ReMatch>(mod);

    vm->bind_func<2>(mod, "match", [](VM* vm, const pkpy::Args& args) {
        const Str& pattern = vm->PyStr_AS_C(args[0]);
        const Str& string = vm->PyStr_AS_C(args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, const pkpy::Args& args) {
        const Str& pattern = vm->PyStr_AS_C(args[0]);
        const Str& string = vm->PyStr_AS_C(args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, const pkpy::Args& args) {
        const Str& pattern = vm->PyStr_AS_C(args[0]);
        const Str& repl = vm->PyStr_AS_C(args[1]);
        const Str& string = vm->PyStr_AS_C(args[2]);
        std::regex re(pattern);
        return vm->PyStr(std::regex_replace(string, re, repl));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, const pkpy::Args& args) {
        const Str& pattern = vm->PyStr_AS_C(args[0]);
        const Str& string = vm->PyStr_AS_C(args[1]);
        std::regex re(pattern);
        std::sregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::sregex_token_iterator end;
        pkpy::List vec;
        for(; it != end; ++it){
            vec.push_back(vm->PyStr(it->str()));
        }
        return vm->PyList(vec);
    });
}

void add_module_random(VM* vm){
    PyVar mod = vm->new_module("random");
    std::srand(std::time(0));
    vm->bind_func<1>(mod, "seed", [](VM* vm, const pkpy::Args& args) {
        std::srand((unsigned int)vm->PyInt_AS_C(args[0]));
        return vm->None;
    });

    vm->bind_func<0>(mod, "random", CPP_LAMBDA(vm->PyFloat((f64)std::rand() / RAND_MAX)));
    vm->bind_func<2>(mod, "randint", [](VM* vm, const pkpy::Args& args) {
        i64 a = vm->PyInt_AS_C(args[0]);
        i64 b = vm->PyInt_AS_C(args[1]);
        if(a > b) std::swap(a, b);
        return vm->PyInt(a + std::rand() % (b - a + 1));
    });

    vm->bind_func<2>(mod, "uniform", [](VM* vm, const pkpy::Args& args) {
        f64 a = vm->PyFloat_AS_C(args[0]);
        f64 b = vm->PyFloat_AS_C(args[1]);
        if(a > b) std::swap(a, b);
        return vm->PyFloat(a + (b - a) * std::rand() / RAND_MAX);
    });

    CodeObject_ code = vm->compile(kRandomCode, "random.py", EXEC_MODE);
    vm->_exec(code, mod, pkpy::make_shared<pkpy::NameDict>());
}


class _PkExported{
public:
    virtual ~_PkExported() = default;
    virtual void* get() = 0;
};

static std::vector<_PkExported*> _pk_lookup_table;

template<typename T>
class PkExported : public _PkExported{
    T* _ptr;
public:
    template<typename... Args>
    PkExported(Args&&... args) {
        _ptr = new T(std::forward<Args>(args)...);
        _pk_lookup_table.push_back(this);
    }
    
    ~PkExported() override { delete _ptr; }
    void* get() override { return _ptr; }
    operator T*() { return _ptr; }
};

#define PKPY_ALLOCATE(T, ...) *(new PkExported<T>(__VA_ARGS__))


extern "C" {
    __EXPORT
    /// Delete a pointer allocated by `pkpy_xxx_xxx`.
    /// It can be `VM*`, `REPL*`, `char*`, etc.
    /// 
    /// !!!
    /// If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
    /// !!!
    void pkpy_delete(void* p){
        for(int i = 0; i < _pk_lookup_table.size(); i++){
            if(_pk_lookup_table[i]->get() == p){
                delete _pk_lookup_table[i];
                _pk_lookup_table.erase(_pk_lookup_table.begin() + i);
                return;
            }
        }
        free(p);
    }

    __EXPORT
    /// Run a given source on a virtual machine.
    void pkpy_vm_exec(VM* vm, const char* source){
        vm->exec(source, "main.py", EXEC_MODE);
    }

    __EXPORT
    /// Get a global variable of a virtual machine.
    /// 
    /// Return `__repr__` of the result.
    /// If the variable is not found, return `nullptr`.
    char* pkpy_vm_get_global(VM* vm, const char* name){
        auto it = vm->_main->attribs.find(name);
        if(it == vm->_main->attribs.end()) return nullptr;
        try{
            Str _repr = vm->PyStr_AS_C(vm->asRepr(it->second));
            return strdup(_repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Evaluate an expression.
    /// 
    /// Return `__repr__` of the result.
    /// If there is any error, return `nullptr`.
    char* pkpy_vm_eval(VM* vm, const char* source){
        PyVarOrNull ret = vm->exec(source, "<eval>", EVAL_MODE);
        if(ret == nullptr) return nullptr;
        try{
            Str _repr = vm->PyStr_AS_C(vm->asRepr(ret));
            return strdup(_repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Create a REPL, using the given virtual machine as the backend.
    REPL* pkpy_new_repl(VM* vm){
        return PKPY_ALLOCATE(REPL, vm);
    }

    __EXPORT
    /// Input a source line to an interactive console. Return true if need more lines.
    bool pkpy_repl_input(REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    /// Add a source module into a virtual machine.
    void pkpy_vm_add_module(VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    __EXPORT
    /// Create a virtual machine.
    VM* pkpy_new_vm(bool use_stdio){
        VM* vm = PKPY_ALLOCATE(VM, use_stdio);
        init_builtins(vm);
        add_module_sys(vm);
        add_module_time(vm);
        add_module_json(vm);
        add_module_math(vm);
        add_module_re(vm);
        add_module_dis(vm);
        add_module_random(vm);

        CodeObject_ code = vm->compile(kBuiltinsCode, "<builtins>", EXEC_MODE);
        vm->_exec(code, vm->builtins, pkpy::make_shared<pkpy::NameDict>());
        return vm;
    }

    __EXPORT
    /// Read the standard output and standard error as string of a virtual machine.
    /// The `vm->use_stdio` should be `false`.
    /// After this operation, both stream will be cleared.
    ///
    /// Return a json representing the result.
    char* pkpy_vm_read_output(VM* vm){
        if(vm->use_stdio) return nullptr;
        StrStream* s_out = (StrStream*)(vm->_stdout);
        StrStream* s_err = (StrStream*)(vm->_stderr);
        Str _stdout = s_out->str();
        Str _stderr = s_err->str();
        StrStream ss;
        ss << '{' << "\"stdout\": " << _stdout.escape(false);
        ss << ", " << "\"stderr\": " << _stderr.escape(false) << '}';
        s_out->str(""); s_err->str("");
        return strdup(ss.str().c_str());
    }

    typedef i64 (*f_int_t)(char*);
    typedef f64 (*f_float_t)(char*);
    typedef bool (*f_bool_t)(char*);
    typedef char* (*f_str_t)(char*);
    typedef void (*f_None_t)(char*);

    static f_int_t f_int = nullptr;
    static f_float_t f_float = nullptr;
    static f_bool_t f_bool = nullptr;
    static f_str_t f_str = nullptr;
    static f_None_t f_None = nullptr;

    __EXPORT
    /// Setup the callback functions.
    void pkpy_setup_callbacks(f_int_t f_int, f_float_t f_float, f_bool_t f_bool, f_str_t f_str, f_None_t f_None){
        ::f_int = f_int;
        ::f_float = f_float;
        ::f_bool = f_bool;
        ::f_str = f_str;
        ::f_None = f_None;
    }

    __EXPORT
    /// Bind a function to a virtual machine.
    char* pkpy_vm_bind(VM* vm, const char* mod, const char* name, int ret_code){
        if(!f_int || !f_float || !f_bool || !f_str || !f_None) return nullptr;
        static int kGlobalBindId = 0;
        for(int i=0; mod[i]; i++) if(mod[i] == ' ') return nullptr;
        for(int i=0; name[i]; i++) if(name[i] == ' ') return nullptr;
        std::string f_header = std::string(mod) + '.' + name + '#' + std::to_string(kGlobalBindId++);
        PyVar obj = vm->_modules.contains(mod) ? vm->_modules[mod] : vm->new_module(mod);
        vm->bind_func<-1>(obj, name, [ret_code, f_header](VM* vm, const pkpy::Args& args){
            StrStream ss;
            ss << f_header;
            for(int i=0; i<args.size(); i++){
                ss << ' ';
                PyVar x = vm->call(args[i], __json__);
                ss << vm->PyStr_AS_C(x);
            }
            char* packet = strdup(ss.str().c_str());
            switch(ret_code){
                case 'i': return vm->PyInt(f_int(packet));
                case 'f': return vm->PyFloat(f_float(packet));
                case 'b': return vm->PyBool(f_bool(packet));
                case 's': {
                    char* p = f_str(packet);
                    if(p == nullptr) return vm->None;
                    return vm->PyStr(p); // no need to free(p)
                }
                case 'N': f_None(packet); return vm->None;
            }
            free(packet);
            UNREACHABLE();
            return vm->None;
        });
        return strdup(f_header.c_str());
    }
}