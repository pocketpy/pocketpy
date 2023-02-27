#pragma once

#include "ceval.h"
#include "compiler.h"
#include "repl.h"
#include "iter.h"
#include "cffi.h"

namespace pkpy {

CodeObject_ VM::compile(Str source, Str filename, CompileMode mode) {
    Compiler compiler(this, source.c_str(), filename, mode);
    try{
        return compiler.compile();
    }catch(Exception& e){
        // std::cout << e.summary() << std::endl;
        _error(e);
        return nullptr;
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, Args& args){                         \
        if(is_both_int(args[0], args[1])){                                                              \
            return py_object(vm, _py_cast_v<i64>(vm, args[0]) op _py_cast_v<i64>(vm, args[1]));                     \
        }else{                                                                                          \
            return py_object(vm, vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                 \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->_bind_methods<1>({"int","float"}, #name, [](VM* vm, Args& args){                         \
        if(!is_both_int_or_float(args[0], args[1])){                                                    \
            if constexpr(is_eq) return vm->PyBool(args[0] op args[1]);                                  \
            vm->TypeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        if(is_both_int(args[0], args[1]))                                                               \
            return vm->PyBool(_py_cast_v<i64>(vm, args[0]) op _py_cast_v<i64>(vm, args[1]));                    \
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

    _vm->bind_builtin_func<1>("__sys_stdout_write", [](VM* vm, Args& args) {
        (*vm->_stdout) << py_cast<Str>(vm, args[0]);
        return vm->None;
    });

    // _vm->bind_builtin_func<1>("test", [](VM* vm, Args& args) {
    //     args[0]->attr().print_stats();
    //     return vm->None;
    // });

    _vm->bind_builtin_func<0>("super", [](VM* vm, Args& args) {
        const PyVar* self = vm->top_frame()->f_locals().try_get(m_self);
        if(self == nullptr) vm->TypeError("super() can only be called in a class");
        return vm->new_object(vm->tp_super, *self);
    });

    _vm->bind_builtin_func<1>("id", [](VM* vm, Args& args) {
        const PyVar& obj = args[0];
        if(obj.is_tagged()) return py_object(vm, (i64)0);
        return py_object(vm, obj.bits);
    });

    _vm->bind_builtin_func<1>("eval", [](VM* vm, Args& args) {
        CodeObject_ code = vm->compile(py_cast<Str>(vm, args[0]), "<eval>", EVAL_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    _vm->bind_builtin_func<1>("exec", [](VM* vm, Args& args) {
        CodeObject_ code = vm->compile(py_cast<Str>(vm, args[0]), "<exec>", EXEC_MODE);
        vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, Args& args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit((int)py_cast_v<i64>(vm, args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", CPP_LAMBDA(vm->asRepr(args[0])));
    _vm->bind_builtin_func<1>("len", CPP_LAMBDA(vm->call(args[0], __len__, no_arg())));

    _vm->bind_builtin_func<1>("hash", [](VM* vm, Args& args){
        i64 value = vm->hash(args[0]);
        if(((value << 2) >> 2) != value) value >>= 2;
        return py_object(vm, value);
    });

    _vm->bind_builtin_func<1>("chr", [](VM* vm, Args& args) {
        i64 i = py_cast_v<i64>(vm, args[0]);
        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
        return py_object(vm, std::string(1, (char)i));
    });

    _vm->bind_builtin_func<1>("ord", [](VM* vm, Args& args) {
        Str s = py_cast<Str>(vm, args[0]);
        if (s.size() != 1) vm->TypeError("ord() expected an ASCII character");
        return py_object(vm, (i64)(s.c_str()[0]));
    });

    _vm->bind_builtin_func<2>("hasattr", [](VM* vm, Args& args) {
        return vm->PyBool(vm->getattr(args[0], py_cast<Str>(vm, args[1]), false) != nullptr);
    });

    _vm->bind_builtin_func<3>("setattr", [](VM* vm, Args& args) {
        vm->setattr(args[0], py_cast<Str>(vm, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_builtin_func<2>("getattr", [](VM* vm, Args& args) {
        Str name = py_cast<Str>(vm, args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, Args& args) {
        std::stringstream ss;
        ss << std::hex << py_cast_v<i64>(vm, args[0]);
        return py_object(vm, "0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, Args& args) {
        std::set<StrName> names;
        if(args[0]->is_attr_valid()){
            std::vector<StrName> keys = args[0]->attr().keys();
            names.insert(keys.begin(), keys.end());
        }
        const NameDict& t_attr = vm->_t(args[0])->attr();
        std::vector<StrName> keys = t_attr.keys();
        names.insert(keys.begin(), keys.end());
        List ret;
        for (StrName name : names) ret.push_back(py_object(vm, name.str()));
        return py_object(vm, std::move(ret));
    });

    _vm->bind_method<0>("object", "__repr__", [](VM* vm, Args& args) {
        PyVar self = args[0];
        std::uintptr_t addr = self.is_tagged() ? 0 : (uintptr_t)self.get();
        StrStream ss;
        ss << std::hex << addr;
        Str s = "<" + OBJ_NAME(vm->_t(self)) + " object at 0x" + ss.str() + ">";
        return py_object(vm, s);
    });

    _vm->bind_method<1>("object", "__eq__", CPP_LAMBDA(vm->PyBool(args[0] == args[1])));
    _vm->bind_method<1>("object", "__ne__", CPP_LAMBDA(vm->PyBool(args[0] != args[1])));

    _vm->bind_static_method<1>("type", "__new__", CPP_LAMBDA(vm->_t(args[0])));

    _vm->bind_static_method<-1>("range", "__new__", [](VM* vm, Args& args) {
        Range r;
        switch (args.size()) {
            case 1: r.stop = py_cast_v<i64>(vm, args[0]); break;
            case 2: r.start = py_cast_v<i64>(vm, args[0]); r.stop = py_cast_v<i64>(vm, args[1]); break;
            case 3: r.start = py_cast_v<i64>(vm, args[0]); r.stop = py_cast_v<i64>(vm, args[1]); r.step = py_cast_v<i64>(vm, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return py_object(vm, r);
    });

    _vm->bind_method<0>("range", "__iter__", CPP_LAMBDA(
        vm->PyIter(RangeIter(vm, args[0]))
    ));

    _vm->bind_method<0>("NoneType", "__repr__", CPP_LAMBDA(py_object(vm, "None")));
    _vm->bind_method<0>("NoneType", "__json__", CPP_LAMBDA(py_object(vm, "null")));

    _vm->_bind_methods<1>({"int", "float"}, "__truediv__", [](VM* vm, Args& args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->ZeroDivisionError();
        return py_object(vm, vm->num_to_float(args[0]) / rhs);
    });

    _vm->_bind_methods<1>({"int", "float"}, "__pow__", [](VM* vm, Args& args) {
        if(is_both_int(args[0], args[1])){
            i64 lhs = _py_cast_v<i64>(vm, args[0]);
            i64 rhs = _py_cast_v<i64>(vm, args[1]);
            bool flag = false;
            if(rhs < 0) {flag = true; rhs = -rhs;}
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            if(flag) return py_object(vm, (f64)(1.0 / ret));
            return py_object(vm, ret);
        }else{
            return py_object(vm, (f64)std::pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bind_static_method<1>("int", "__new__", [](VM* vm, Args& args) {
        if (is_type(args[0], vm->tp_int)) return args[0];
        if (is_type(args[0], vm->tp_float)) return py_object(vm, (i64)py_cast_v<f64>(vm, args[0]));
        if (is_type(args[0], vm->tp_bool)) return py_object(vm, vm->_PyBool_AS_C(args[0]) ? 1 : 0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = py_cast<Str>(vm, args[0]);
            try{
                size_t parsed = 0;
                i64 val = S_TO_INT(s, &parsed, 10);
                if(parsed != s.size()) throw std::invalid_argument("<?>");
                return py_object(vm, val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for int(): " + s.escape(true));
            }
        }
        vm->TypeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<1>("int", "__floordiv__", [](VM* vm, Args& args) {
        i64 rhs = py_cast_v<i64>(vm, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return py_object(vm, py_cast_v<i64>(vm, args[0]) / rhs);
    });

    _vm->bind_method<1>("int", "__mod__", [](VM* vm, Args& args) {
        i64 rhs = py_cast_v<i64>(vm, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return py_object(vm, py_cast_v<i64>(vm, args[0]) % rhs);
    });

    _vm->bind_method<0>("int", "__repr__", CPP_LAMBDA(py_object(vm, std::to_string(py_cast_v<i64>(vm, args[0])))));
    _vm->bind_method<0>("int", "__json__", CPP_LAMBDA(py_object(vm, std::to_string(py_cast_v<i64>(vm, args[0])))));

#define INT_BITWISE_OP(name,op) \
    _vm->bind_method<1>("int", #name, CPP_LAMBDA(py_object(vm, py_cast_v<i64>(vm, args[0]) op py_cast_v<i64>(vm, args[1]))));

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bind_static_method<1>("float", "__new__", [](VM* vm, Args& args) {
        if (is_type(args[0], vm->tp_int)) return py_object(vm, (f64)py_cast_v<i64>(vm, args[0]));
        if (is_type(args[0], vm->tp_float)) return args[0];
        if (is_type(args[0], vm->tp_bool)) return py_object(vm, vm->_PyBool_AS_C(args[0]) ? 1.0 : 0.0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = py_cast<Str>(vm, args[0]);
            if(s == "inf") return py_object(vm, INFINITY);
            if(s == "-inf") return py_object(vm, -INFINITY);
            try{
                f64 val = S_TO_FLOAT(s);
                return py_object(vm, val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->TypeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<0>("float", "__repr__", [](VM* vm, Args& args) {
        f64 val = py_cast_v<f64>(vm, args[0]);
        if(std::isinf(val) || std::isnan(val)) return py_object(vm, std::to_string(val));
        StrStream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1-2) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return py_object(vm, s);
    });

    _vm->bind_method<0>("float", "__json__", [](VM* vm, Args& args) {
        f64 val = py_cast_v<f64>(vm, args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
        return py_object(vm, std::to_string(val));
    });

    /************ PyString ************/
    _vm->bind_static_method<1>("str", "__new__", CPP_LAMBDA(vm->asStr(args[0])));

    _vm->bind_method<1>("str", "__add__", [](VM* vm, Args& args) {
        const Str& lhs = py_cast<Str>(vm, args[0]);
        const Str& rhs = py_cast<Str>(vm, args[1]);
        return py_object(vm, lhs + rhs);
    });

    _vm->bind_method<0>("str", "__len__", [](VM* vm, Args& args) {
        const Str& self = py_cast<Str>(vm, args[0]);
        return py_object(vm, self.u8_length());
    });

    _vm->bind_method<1>("str", "__contains__", [](VM* vm, Args& args) {
        const Str& self = py_cast<Str>(vm, args[0]);
        const Str& other = py_cast<Str>(vm, args[1]);
        return vm->PyBool(self.find(other) != Str::npos);
    });

    _vm->bind_method<0>("str", "__str__", CPP_LAMBDA(args[0]));
    _vm->bind_method<0>("str", "__iter__", CPP_LAMBDA(vm->PyIter(StringIter(vm, args[0]))));

    _vm->bind_method<0>("str", "__repr__", [](VM* vm, Args& args) {
        const Str& _self = py_cast<Str>(vm, args[0]);
        return py_object(vm, _self.escape(true));
    });

    _vm->bind_method<0>("str", "__json__", [](VM* vm, Args& args) {
        const Str& _self = py_cast<Str>(vm, args[0]);
        return py_object(vm, _self.escape(false));
    });

    _vm->bind_method<1>("str", "__eq__", [](VM* vm, Args& args) {
        if(is_type(args[0], vm->tp_str) && is_type(args[1], vm->tp_str))
            return vm->PyBool(py_cast<Str>(vm, args[0]) == py_cast<Str>(vm, args[1]));
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bind_method<1>("str", "__ne__", [](VM* vm, Args& args) {
        if(is_type(args[0], vm->tp_str) && is_type(args[1], vm->tp_str))
            return vm->PyBool(py_cast<Str>(vm, args[0]) != py_cast<Str>(vm, args[1]));
        return vm->PyBool(args[0] != args[1]);
    });

    _vm->bind_method<1>("str", "__getitem__", [](VM* vm, Args& args) {
        const Str& self (py_cast<Str>(vm, args[0]));

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _py_cast_v<Slice>(vm, args[1]);
            s.normalize(self.u8_length());
            return py_object(vm, self.u8_substr(s.start, s.stop));
        }

        int index = (int)py_cast_v<i64>(vm, args[1]);
        index = vm->normalized_index(index, self.u8_length());
        return py_object(vm, self.u8_getitem(index));
    });

    _vm->bind_method<1>("str", "__gt__", [](VM* vm, Args& args) {
        const Str& self (py_cast<Str>(vm, args[0]));
        const Str& obj (py_cast<Str>(vm, args[1]));
        return vm->PyBool(self > obj);
    });

    _vm->bind_method<1>("str", "__lt__", [](VM* vm, Args& args) {
        const Str& self (py_cast<Str>(vm, args[0]));
        const Str& obj (py_cast<Str>(vm, args[1]));
        return vm->PyBool(self < obj);
    });

    _vm->bind_method<2>("str", "replace", [](VM* vm, Args& args) {
        const Str& _self = py_cast<Str>(vm, args[0]);
        const Str& _old = py_cast<Str>(vm, args[1]);
        const Str& _new = py_cast<Str>(vm, args[2]);
        Str _copy = _self;
        // replace all occurences of _old with _new in _copy
        size_t pos = 0;
        while ((pos = _copy.find(_old, pos)) != std::string::npos) {
            _copy.replace(pos, _old.length(), _new);
            pos += _new.length();
        }
        return py_object(vm, _copy);
    });

    _vm->bind_method<1>("str", "startswith", [](VM* vm, Args& args) {
        const Str& _self = py_cast<Str>(vm, args[0]);
        const Str& _prefix = py_cast<Str>(vm, args[1]);
        return vm->PyBool(_self.find(_prefix) == 0);
    });

    _vm->bind_method<1>("str", "endswith", [](VM* vm, Args& args) {
        const Str& _self = py_cast<Str>(vm, args[0]);
        const Str& _suffix = py_cast<Str>(vm, args[1]);
        return vm->PyBool(_self.rfind(_suffix) == _self.length() - _suffix.length());
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, Args& args) {
        const Str& self = py_cast<Str>(vm, args[0]);
        StrStream ss;
        PyVar obj = vm->asList(args[1]);
        const List& list = py_cast<List>(vm, obj);
        for (int i = 0; i < list.size(); ++i) {
            if (i > 0) ss << self;
            ss << py_cast<Str>(vm, list[i]);
        }
        return py_object(vm, ss.str());
    });

    /************ PyList ************/
    _vm->bind_method<1>("list", "append", [](VM* vm, Args& args) {
        List& self = py_cast<List>(vm, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "reverse", [](VM* vm, Args& args) {
        List& self = py_cast<List>(vm, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind_method<1>("list", "__mul__", [](VM* vm, Args& args) {
        const List& self = py_cast<List>(vm, args[0]);
        int n = (int)py_cast_v<i64>(vm, args[1]);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.insert(result.end(), self.begin(), self.end());
        return py_object(vm, std::move(result));
    });

    _vm->bind_method<2>("list", "insert", [](VM* vm, Args& args) {
        List& self = py_cast<List>(vm, args[0]);
        int index = (int)py_cast_v<i64>(vm, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(self.begin() + index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "clear", [](VM* vm, Args& args) {
        py_cast<List>(vm, args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>("list", "copy", CPP_LAMBDA(py_object(vm, py_cast<List>(vm, args[0]))));

    _vm->bind_method<1>("list", "__add__", [](VM* vm, Args& args) {
        const List& self = py_cast<List>(vm, args[0]);
        const List& obj = py_cast<List>(vm, args[1]);
        List new_list = self;
        new_list.insert(new_list.end(), obj.begin(), obj.end());
        return py_object(vm, new_list);
    });

    _vm->bind_method<0>("list", "__len__", [](VM* vm, Args& args) {
        const List& self = py_cast<List>(vm, args[0]);
        return py_object(vm, self.size());
    });

    _vm->bind_method<0>("list", "__iter__", [](VM* vm, Args& args) {
        return vm->PyIter(ArrayIter<List>(vm, args[0]));
    });

    _vm->bind_method<1>("list", "__getitem__", [](VM* vm, Args& args) {
        const List& self = py_cast<List>(vm, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _py_cast_v<Slice>(vm, args[1]);
            s.normalize(self.size());
            List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return py_object(vm, std::move(new_list));
        }

        int index = (int)py_cast_v<i64>(vm, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<2>("list", "__setitem__", [](VM* vm, Args& args) {
        List& self = py_cast<List>(vm, args[0]);
        int index = (int)py_cast_v<i64>(vm, args[1]);
        index = vm->normalized_index(index, self.size());
        self[index] = args[2];
        return vm->None;
    });

    _vm->bind_method<1>("list", "__delitem__", [](VM* vm, Args& args) {
        List& self = py_cast<List>(vm, args[0]);
        int index = (int)py_cast_v<i64>(vm, args[1]);
        index = vm->normalized_index(index, self.size());
        self.erase(self.begin() + index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bind_static_method<1>("tuple", "__new__", [](VM* vm, Args& args) {
        List list = py_cast<List>(vm, vm->asList(args[0]));
        return py_object(vm, std::move(list));
    });

    _vm->bind_method<0>("tuple", "__iter__", [](VM* vm, Args& args) {
        return vm->PyIter(ArrayIter<Args>(vm, args[0]));
    });

    _vm->bind_method<1>("tuple", "__getitem__", [](VM* vm, Args& args) {
        const Tuple& self = py_cast<Tuple>(vm, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            Slice s = _py_cast_v<Slice>(vm, args[1]);
            s.normalize(self.size());
            List new_list;
            for(size_t i = s.start; i < s.stop; i++) new_list.push_back(self[i]);
            return py_object(vm, std::move(new_list));
        }

        int index = (int)py_cast_v<i64>(vm, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<0>("tuple", "__len__", [](VM* vm, Args& args) {
        const Tuple& self = py_cast<Tuple>(vm, args[0]);
        return py_object(vm, self.size());
    });

    /************ PyBool ************/
    _vm->bind_static_method<1>("bool", "__new__", CPP_LAMBDA(vm->asBool(args[0])));

    _vm->bind_method<0>("bool", "__repr__", [](VM* vm, Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return py_object(vm, val ? "True" : "False");
    });

    _vm->bind_method<0>("bool", "__json__", [](VM* vm, Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return py_object(vm, val ? "true" : "false");
    });

    _vm->bind_method<1>("bool", "__xor__", [](VM* vm, Args& args) {
        bool self = vm->PyBool_AS_C(args[0]);
        bool other = vm->PyBool_AS_C(args[1]);
        return vm->PyBool(self ^ other);
    });

    _vm->bind_method<0>("ellipsis", "__repr__", CPP_LAMBDA(py_object(vm, "Ellipsis")));
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
    vm->bind_func<0>(mod, "time", [](VM* vm, Args& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return py_object(vm, std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void add_module_sys(VM* vm){
    PyVar mod = vm->new_module("sys");
    vm->setattr(mod, "version", py_object(vm, PK_VERSION));

    vm->bind_func<1>(mod, "getrefcount", CPP_LAMBDA(py_object(vm, args[0].use_count())));
    vm->bind_func<0>(mod, "getrecursionlimit", CPP_LAMBDA(py_object(vm, vm->recursionlimit)));

    vm->bind_func<1>(mod, "setrecursionlimit", [](VM* vm, Args& args) {
        vm->recursionlimit = (int)py_cast_v<i64>(vm, args[0]);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyVar mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, Args& args) {
        const Str& expr = py_cast<Str>(vm, args[0]);
        CodeObject_ code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    vm->bind_func<1>(mod, "dumps", CPP_LAMBDA(vm->call(args[0], __json__)));
}

void add_module_math(VM* vm){
    PyVar mod = vm->new_module("math");
    vm->setattr(mod, "pi", py_object(vm, 3.1415926535897932384));
    vm->setattr(mod, "e" , py_object(vm, 2.7182818284590452354));

    vm->bind_func<1>(mod, "log", CPP_LAMBDA(py_object(vm, std::log(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log10", CPP_LAMBDA(py_object(vm, std::log10(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log2", CPP_LAMBDA(py_object(vm, std::log2(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sin", CPP_LAMBDA(py_object(vm, std::sin(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "cos", CPP_LAMBDA(py_object(vm, std::cos(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "tan", CPP_LAMBDA(py_object(vm, std::tan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isnan", CPP_LAMBDA(vm->PyBool(std::isnan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isinf", CPP_LAMBDA(vm->PyBool(std::isinf(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "fabs", CPP_LAMBDA(py_object(vm, std::fabs(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "floor", CPP_LAMBDA(py_object(vm, (i64)std::floor(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "ceil", CPP_LAMBDA(py_object(vm, (i64)std::ceil(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sqrt", CPP_LAMBDA(py_object(vm, std::sqrt(vm->num_to_float(args[0])))));
}

void add_module_dis(VM* vm){
    PyVar mod = vm->new_module("dis");
    vm->bind_func<1>(mod, "dis", [](VM* vm, Args& args) {
        PyVar f = args[0];
        if(is_type(f, vm->tp_bound_method)) f = py_cast<BoundMethod>(vm, args[0]).method;
        CodeObject_ code = py_cast<Function>(vm, f).code;
        (*vm->_stdout) << vm->disassemble(code);
        return vm->None;
    });
}

struct FileIO {
    PY_CLASS(io, FileIO)

    Str file;
    Str mode;
    std::fstream _fs;

    FileIO(VM* vm, Str file, Str mode): file(file), mode(mode) {
        if(mode == "rt" || mode == "r"){
            _fs.open(file, std::ios::in);
        }else if(mode == "wt" || mode == "w"){
            _fs.open(file, std::ios::out);
        }else if(mode == "at" || mode == "a"){
            _fs.open(file, std::ios::app);
        }
        if(!_fs.is_open()) vm->IOError(strerror(errno));
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<2>(type, "__new__", [](VM* vm, Args& args){
            return vm->new_object<FileIO>(
                vm, py_cast<Str>(vm, args[0]), py_cast<Str>(vm, args[1])
            );
        });

        vm->bind_method<0>(type, "read", [](VM* vm, Args& args){
            FileIO& io = vm->_cast<FileIO>(args[0]);
            std::string buffer;
            io._fs >> buffer;
            return py_object(vm, buffer);
        });

        vm->bind_method<1>(type, "write", [](VM* vm, Args& args){
            FileIO& io = vm->_cast<FileIO>(args[0]);
            io._fs << py_cast<Str>(vm, args[1]);
            return vm->None;
        });

        vm->bind_method<0>(type, "close", [](VM* vm, Args& args){
            FileIO& io = vm->_cast<FileIO>(args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__exit__", [](VM* vm, Args& args){
            FileIO& io = vm->_cast<FileIO>(args[0]);
            io._fs.close();
            return vm->None;
        });

        vm->bind_method<0>(type, "__enter__", CPP_LAMBDA(vm->None));
    }
};
void add_module_io(VM* vm){
    PyVar mod = vm->new_module("io");
    PyVar type = vm->register_class<FileIO>(mod);
    vm->bind_builtin_func<2>("open", [type](VM* vm, const Args& args){
        return vm->call(type, args);
    });
}

void add_module_os(VM* vm){}

struct ReMatch {
    PY_CLASS(re, Match)

    i64 start;
    i64 end;
    std::smatch m;
    ReMatch(i64 start, i64 end, std::smatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_method<-1>(type, "__init__", CPP_NOT_IMPLEMENTED());
        vm->bind_method<0>(type, "start", CPP_LAMBDA(py_object(vm, vm->_cast<ReMatch>(args[0]).start)));
        vm->bind_method<0>(type, "end", CPP_LAMBDA(py_object(vm, vm->_cast<ReMatch>(args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, Args& args) {
            auto& self = vm->_cast<ReMatch>(args[0]);
            return py_object(vm, two_args(py_object(vm, self.start), py_object(vm, self.end)));
        });

        vm->bind_method<1>(type, "group", [](VM* vm, Args& args) {
            auto& self = vm->_cast<ReMatch>(args[0]);
            int index = (int)py_cast_v<i64>(vm, args[1]);
            index = vm->normalized_index(index, self.m.size());
            return py_object(vm, self.m[index].str());
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

    vm->bind_func<2>(mod, "match", [](VM* vm, Args& args) {
        const Str& pattern = py_cast<Str>(vm, args[0]);
        const Str& string = py_cast<Str>(vm, args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, Args& args) {
        const Str& pattern = py_cast<Str>(vm, args[0]);
        const Str& string = py_cast<Str>(vm, args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, Args& args) {
        const Str& pattern = py_cast<Str>(vm, args[0]);
        const Str& repl = py_cast<Str>(vm, args[1]);
        const Str& string = py_cast<Str>(vm, args[2]);
        std::regex re(pattern);
        return py_object(vm, std::regex_replace(string, re, repl));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, Args& args) {
        const Str& pattern = py_cast<Str>(vm, args[0]);
        const Str& string = py_cast<Str>(vm, args[1]);
        std::regex re(pattern);
        std::sregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::sregex_token_iterator end;
        List vec;
        for(; it != end; ++it){
            vec.push_back(py_object(vm, it->str()));
        }
        return py_object(vm, vec);
    });
}

void add_module_random(VM* vm){
    PyVar mod = vm->new_module("random");
    std::srand(std::time(0));
    vm->bind_func<1>(mod, "seed", [](VM* vm, Args& args) {
        std::srand((unsigned int)py_cast_v<i64>(vm, args[0]));
        return vm->None;
    });

    vm->bind_func<0>(mod, "random", CPP_LAMBDA(py_object(vm, std::rand() / (f64)RAND_MAX)));
    vm->bind_func<2>(mod, "randint", [](VM* vm, Args& args) {
        i64 a = py_cast_v<i64>(vm, args[0]);
        i64 b = py_cast_v<i64>(vm, args[1]);
        if(a > b) std::swap(a, b);
        return py_object(vm, a + std::rand() % (b - a + 1));
    });

    vm->bind_func<2>(mod, "uniform", [](VM* vm, Args& args) {
        f64 a = py_cast_v<f64>(vm, args[0]);
        f64 b = py_cast_v<f64>(vm, args[1]);
        if(a > b) std::swap(a, b);
        return py_object(vm, a + (b - a) * std::rand() / (f64)RAND_MAX);
    });

    CodeObject_ code = vm->compile(kRandomCode, "random.py", EXEC_MODE);
    vm->_exec(code, mod);
}

void add_module_functools(VM* vm){
    PyVar mod = vm->new_module("functools");
    CodeObject_ code = vm->compile(kFuncToolsCode, "functools.py", EXEC_MODE);
    vm->_exec(code, mod);
}

void VM::post_init(){
    init_builtins(this);
    add_module_sys(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_re(this);
    add_module_dis(this);
    add_module_random(this);
    add_module_io(this);
    add_module_os(this);
    add_module_functools(this);
    add_module_c(this);

    CodeObject_ code = compile(kBuiltinsCode, "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
}

}   // namespace pkpy

/*************************GLOBAL NAMESPACE*************************/

class PkExportedBase{
public:
    virtual ~PkExportedBase() = default;
    virtual void* get() = 0;
};

static std::vector<PkExportedBase*> _pk_lookup_table;
template<typename T>
class PkExported : public PkExportedBase{
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
    void pkpy_vm_exec(pkpy::VM* vm, const char* source){
        vm->exec(source, "main.py", pkpy::EXEC_MODE);
    }

    __EXPORT
    /// Get a global variable of a virtual machine.
    /// 
    /// Return `__repr__` of the result.
    /// If the variable is not found, return `nullptr`.
    char* pkpy_vm_get_global(pkpy::VM* vm, const char* name){
        pkpy::PyVar* val = vm->_main->attr().try_get(name);
        if(val == nullptr) return nullptr;
        try{
            pkpy::Str& _repr = pkpy::py_cast<pkpy::Str>(vm, vm->asRepr(*val));
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
    char* pkpy_vm_eval(pkpy::VM* vm, const char* source){
        pkpy::PyVarOrNull ret = vm->exec(source, "<eval>", pkpy::EVAL_MODE);
        if(ret == nullptr) return nullptr;
        try{
            pkpy::Str& _repr = pkpy::py_cast<pkpy::Str>(vm, vm->asRepr(ret));
            return strdup(_repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Create a REPL, using the given virtual machine as the backend.
    pkpy::REPL* pkpy_new_repl(pkpy::VM* vm){
        return PKPY_ALLOCATE(pkpy::REPL, vm);
    }

    __EXPORT
    /// Input a source line to an interactive console. Return true if need more lines.
    bool pkpy_repl_input(pkpy::REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    /// Add a source module into a virtual machine.
    void pkpy_vm_add_module(pkpy::VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    __EXPORT
    /// Create a virtual machine.
    pkpy::VM* pkpy_new_vm(bool use_stdio){
        return PKPY_ALLOCATE(pkpy::VM, use_stdio);
    }

    __EXPORT
    /// Read the standard output and standard error as string of a virtual machine.
    /// The `vm->use_stdio` should be `false`.
    /// After this operation, both stream will be cleared.
    ///
    /// Return a json representing the result.
    char* pkpy_vm_read_output(pkpy::VM* vm){
        if(vm->use_stdio) return nullptr;
        pkpy::StrStream* s_out = (pkpy::StrStream*)(vm->_stdout);
        pkpy::StrStream* s_err = (pkpy::StrStream*)(vm->_stderr);
        pkpy::Str _stdout = s_out->str();
        pkpy::Str _stderr = s_err->str();
        pkpy::StrStream ss;
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
    void pkpy_setup_callbacks(f_int_t _f_int, f_float_t _f_float, f_bool_t _f_bool, f_str_t _f_str, f_None_t _f_None){
        f_int = _f_int;
        f_float = _f_float;
        f_bool = _f_bool;
        f_str = _f_str;
        f_None = _f_None;
    }

    __EXPORT
    /// Bind a function to a virtual machine.
    char* pkpy_vm_bind(pkpy::VM* vm, const char* mod, const char* name, int ret_code){
        if(!f_int || !f_float || !f_bool || !f_str || !f_None) return nullptr;
        static int kGlobalBindId = 0;
        for(int i=0; mod[i]; i++) if(mod[i] == ' ') return nullptr;
        for(int i=0; name[i]; i++) if(name[i] == ' ') return nullptr;
        std::string f_header = std::string(mod) + '.' + name + '#' + std::to_string(kGlobalBindId++);
        pkpy::PyVar obj = vm->_modules.contains(mod) ? vm->_modules[mod] : vm->new_module(mod);
        vm->bind_func<-1>(obj, name, [ret_code, f_header](pkpy::VM* vm, const pkpy::Args& args){
            pkpy::StrStream ss;
            ss << f_header;
            for(int i=0; i<args.size(); i++){
                ss << ' ';
                pkpy::PyVar x = vm->call(args[i], pkpy::__json__);
                ss << pkpy::py_cast<pkpy::Str>(vm, x);
            }
            char* packet = strdup(ss.str().c_str());
            switch(ret_code){
                case 'i': return py_object(vm, f_int(packet));
                case 'f': return py_object(vm, f_float(packet));
                case 'b': return vm->PyBool(f_bool(packet));
                case 's': {
                    char* p = f_str(packet);
                    if(p == nullptr) return vm->None;
                    return py_object(vm, p); // no need to free(p)
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