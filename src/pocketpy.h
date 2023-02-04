#pragma once

#include "vm.h"
#include "compiler.h"
#include "repl.h"

_Code VM::compile(_Str source, _Str filename, CompileMode mode) {
    Compiler compiler(this, source.c_str(), filename, mode);
    try{
        return compiler.__fillCode();
    }catch(_Exception& e){
        _error(e);
        return nullptr;
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bindMethodMulti<1>({"int","float"}, #name, [](VM* vm, const pkpy::Args& args){                 \
        if(args[0]->is_type(vm->_tp_int) && args[1]->is_type(vm->_tp_int)){                             \
            return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1]));                       \
        }else{                                                                                          \
            return vm->PyFloat(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                 \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->bindMethodMulti<1>({"int","float"}, #name, [](VM* vm, const pkpy::Args& args){                 \
        bool _0 = args[0]->is_type(vm->_tp_int) || args[0]->is_type(vm->_tp_float);                     \
        bool _1 = args[1]->is_type(vm->_tp_int) || args[1]->is_type(vm->_tp_float);                     \
        if(!_0 || !_1){                                                                                 \
            if constexpr(is_eq) return vm->PyBool(args[0].get() op args[1].get());                      \
            vm->typeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        return vm->PyBool(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                      \
    });
    

void __initializeBuiltinFunctions(VM* _vm) {
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

    _vm->bindBuiltinFunc<1>("__sys_stdout_write", [](VM* vm, const pkpy::Args& args) {
        (*vm->_stdout) << vm->PyStr_AS_C(args[0]);
        return vm->None;
    });

    _vm->bindBuiltinFunc<0>("super", [](VM* vm, const pkpy::Args& args) {
        auto it = vm->top_frame()->f_locals().find(m_self);
        if(it == vm->top_frame()->f_locals().end()) vm->typeError("super() can only be called in a class method");
        return vm->new_object(vm->_tp_super, it->second);
    });

    _vm->bindBuiltinFunc<1>("eval", [](VM* vm, const pkpy::Args& args) {
        _Code code = vm->compile(vm->PyStr_AS_C(args[0]), "<eval>", EVAL_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    _vm->bindBuiltinFunc<1>("exec", [](VM* vm, const pkpy::Args& args) {
        _Code code = vm->compile(vm->PyStr_AS_C(args[0]), "<exec>", EXEC_MODE);
        vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
        return vm->None;
    });

    _vm->bindBuiltinFunc<1>("repr", CPP_LAMBDA(vm->asRepr(args[0])));
    _vm->bindBuiltinFunc<1>("hash", CPP_LAMBDA(vm->PyInt(vm->hash(args[0]))));
    _vm->bindBuiltinFunc<1>("len", CPP_LAMBDA(vm->call(args[0], __len__, pkpy::noArg())));

    _vm->bindBuiltinFunc<1>("chr", [](VM* vm, const pkpy::Args& args) {
        i64 i = vm->PyInt_AS_C(args[0]);
        if (i < 0 || i > 128) vm->valueError("chr() arg not in range(128)");
        return vm->PyStr(std::string(1, (char)i));
    });

    _vm->bindBuiltinFunc<1>("ord", [](VM* vm, const pkpy::Args& args) {
        _Str s = vm->PyStr_AS_C(args[0]);
        if (s.size() != 1) vm->typeError("ord() expected an ASCII character");
        return vm->PyInt((i64)(s.c_str()[0]));
    });

    _vm->bindBuiltinFunc<2>("hasattr", [](VM* vm, const pkpy::Args& args) {
        return vm->PyBool(vm->getattr(args[0], vm->PyStr_AS_C(args[1]), false) != nullptr);
    });

    _vm->bindBuiltinFunc<3>("setattr", [](VM* vm, const pkpy::Args& args) {
        PyVar obj = args[0];
        vm->setattr(obj, vm->PyStr_AS_C(args[1]), args[2]);
        return vm->None;
    });

    _vm->bindBuiltinFunc<2>("getattr", [](VM* vm, const pkpy::Args& args) {
        _Str name = vm->PyStr_AS_C(args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bindBuiltinFunc<1>("hex", [](VM* vm, const pkpy::Args& args) {
        std::stringstream ss;
        ss << std::hex << vm->PyInt_AS_C(args[0]);
        return vm->PyStr("0x" + ss.str());
    });

    _vm->bindBuiltinFunc<1>("dir", [](VM* vm, const pkpy::Args& args) {
        std::vector<_Str> names;
        for (auto& [k, _] : args[0]->attribs) names.push_back(k);
        for (auto& [k, _] : args[0]->type->attribs) {
            if (k.find("__") == 0) continue;
            if (std::find(names.begin(), names.end(), k) == names.end()) names.push_back(k);
        }
        PyVarList ret;
        for (const auto& name : names) ret.push_back(vm->PyStr(name));
        std::sort(ret.begin(), ret.end(), [vm](const PyVar& a, const PyVar& b) {
            return vm->PyStr_AS_C(a) < vm->PyStr_AS_C(b);
        });
        return vm->PyList(ret);
    });

    _vm->bindMethod<0>("object", "__repr__", [](VM* vm, const pkpy::Args& args) {
        PyVar _self = args[0];
        std::stringstream ss;
        ss << std::hex << (uintptr_t)_self.get();
        _Str s = "<" + UNION_TP_NAME(_self) + " object at 0x" + ss.str() + ">";
        return vm->PyStr(s);
    });

    _vm->bindMethod<1>("object", "__eq__", CPP_LAMBDA(vm->PyBool(args[0] == args[1])));
    _vm->bindMethod<1>("object", "__ne__", CPP_LAMBDA(vm->PyBool(args[0] != args[1])));

    _vm->bindStaticMethod<1>("type", "__new__", CPP_LAMBDA(args[0]->type));

    _vm->bindStaticMethod<-1>("range", "__new__", [](VM* vm, const pkpy::Args& args) {
        _Range r;
        switch (args.size()) {
            case 1: r.stop = vm->PyInt_AS_C(args[0]); break;
            case 2: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); break;
            case 3: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); r.step = vm->PyInt_AS_C(args[2]); break;
            default: vm->typeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return vm->PyRange(r);
    });

    _vm->bindMethod<0>("range", "__iter__", CPP_LAMBDA(
        vm->PyIter(pkpy::make_shared<BaseIterator, RangeIterator>(vm, args[0]))
    ));

    _vm->bindMethod<0>("NoneType", "__repr__", CPP_LAMBDA(vm->PyStr("None")));
    _vm->bindMethod<0>("NoneType", "__json__", CPP_LAMBDA(vm->PyStr("null")));

    _vm->bindMethodMulti<1>({"int", "float"}, "__truediv__", [](VM* vm, const pkpy::Args& args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->zeroDivisionError();
        return vm->PyFloat(vm->num_to_float(args[0]) / rhs);
    });

    _vm->bindMethodMulti<1>({"int", "float"}, "__pow__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->_tp_int) && args[1]->is_type(vm->_tp_int)){
            return vm->PyInt((i64)round(pow(vm->PyInt_AS_C(args[0]), vm->PyInt_AS_C(args[1]))));
        }else{
            return vm->PyFloat((f64)pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bindStaticMethod<1>("int", "__new__", [](VM* vm, const pkpy::Args& args) {
        if (args[0]->is_type(vm->_tp_int)) return args[0];
        if (args[0]->is_type(vm->_tp_float)) return vm->PyInt((i64)vm->PyFloat_AS_C(args[0]));
        if (args[0]->is_type(vm->_tp_bool)) return vm->PyInt(vm->PyBool_AS_C(args[0]) ? 1 : 0);
        if (args[0]->is_type(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            try{
                size_t parsed = 0;
                i64 val = std::stoll(s, &parsed, 10);
                if(parsed != s.size()) throw std::invalid_argument("");
                return vm->PyInt(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for int(): '" + s + "'");
            }
        }
        vm->typeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod<1>("int", "__floordiv__", [](VM* vm, const pkpy::Args& args) {
        i64 rhs = vm->PyInt_AS_C(args[1]);
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args[0]) / rhs);
    });

    _vm->bindMethod<1>("int", "__mod__", [](VM* vm, const pkpy::Args& args) {
        i64 rhs = vm->PyInt_AS_C(args[1]);
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args[0]) % rhs);
    });

    _vm->bindMethod<0>("int", "__repr__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

    _vm->bindMethod<0>("int", "__json__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyStr(std::to_string((int)vm->PyInt_AS_C(args[0])));
    });

#define __INT_BITWISE_OP(name,op) \
    _vm->bindMethod<1>("int", #name, [](VM* vm, const pkpy::Args& args) {                    \
        return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1]));     \
    });

    __INT_BITWISE_OP(__lshift__, <<)
    __INT_BITWISE_OP(__rshift__, >>)
    __INT_BITWISE_OP(__and__, &)
    __INT_BITWISE_OP(__or__, |)
    __INT_BITWISE_OP(__xor__, ^)

#undef __INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bindStaticMethod<1>("float", "__new__", [](VM* vm, const pkpy::Args& args) {
        if (args[0]->is_type(vm->_tp_int)) return vm->PyFloat((f64)vm->PyInt_AS_C(args[0]));
        if (args[0]->is_type(vm->_tp_float)) return args[0];
        if (args[0]->is_type(vm->_tp_bool)) return vm->PyFloat(vm->PyBool_AS_C(args[0]) ? 1.0 : 0.0);
        if (args[0]->is_type(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            if(s == "inf") return vm->PyFloat(INFINITY);
            if(s == "-inf") return vm->PyFloat(-INFINITY);
            try{
                f64 val = std::stod(s);
                return vm->PyFloat(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->typeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod<0>("float", "__repr__", [](VM* vm, const pkpy::Args& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) return vm->PyStr(std::to_string(val));
        _StrStream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return vm->PyStr(s);
    });

    _vm->bindMethod<0>("float", "__json__", [](VM* vm, const pkpy::Args& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->valueError("cannot jsonify 'nan' or 'inf'");
        return vm->PyStr(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bindStaticMethod<1>("str", "__new__", CPP_LAMBDA(vm->asStr(args[0])));

    _vm->bindMethod<1>("str", "__add__", [](VM* vm, const pkpy::Args& args) {
        const _Str& lhs = vm->PyStr_AS_C(args[0]);
        const _Str& rhs = vm->PyStr_AS_C(args[1]);
        return vm->PyStr(lhs + rhs);
    });

    _vm->bindMethod<0>("str", "__len__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyInt(_self.u8_length());
    });

    _vm->bindMethod<1>("str", "__contains__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _other = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_other) != _Str::npos);
    });

    _vm->bindMethod<0>("str", "__str__", CPP_LAMBDA(args[0]));

    _vm->bindMethod<0>("str", "__iter__", CPP_LAMBDA(
        vm->PyIter(pkpy::make_shared<BaseIterator, StringIterator>(vm, args[0]))
    ));

    _vm->bindMethod<0>("str", "__repr__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(true));
    });

    _vm->bindMethod<0>("str", "__json__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(false));
    });

    _vm->bindMethod<1>("str", "__eq__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->_tp_str) && args[1]->is_type(vm->_tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) == vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethod<1>("str", "__ne__", [](VM* vm, const pkpy::Args& args) {
        if(args[0]->is_type(vm->_tp_str) && args[1]->is_type(vm->_tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) != vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] != args[1]);
    });

    _vm->bindMethod<1>("str", "__getitem__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));

        if(args[1]->is_type(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.u8_length());
            return vm->PyStr(_self.u8_substr(s.start, s.stop));
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.u8_length());
        return vm->PyStr(_self.u8_getitem(_index));
    });

    _vm->bindMethod<1>("str", "__gt__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self > _obj);
    });

    _vm->bindMethod<1>("str", "__lt__", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self < _obj);
    });

    _vm->bindMethod<2>("str", "replace", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _old = vm->PyStr_AS_C(args[1]);
        const _Str& _new = vm->PyStr_AS_C(args[2]);
        _Str _copy = _self;
        // replace all occurences of _old with _new in _copy
        size_t pos = 0;
        while ((pos = _copy.find(_old, pos)) != std::string::npos) {
            _copy.replace(pos, _old.length(), _new);
            pos += _new.length();
        }
        return vm->PyStr(_copy);
    });

    _vm->bindMethod<1>("str", "startswith", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _prefix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_prefix) == 0);
    });

    _vm->bindMethod<1>("str", "endswith", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _suffix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.rfind(_suffix) == _self.length() - _suffix.length());
    });

    _vm->bindMethod<1>("str", "join", [](VM* vm, const pkpy::Args& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        PyVarList* _list;
        if(args[1]->is_type(vm->_tp_list)){
            _list = &vm->PyList_AS_C(args[1]);
        }else if(args[1]->is_type(vm->_tp_tuple)){
            _list = &vm->PyTuple_AS_C(args[1]);
        }else{
            vm->typeError("can only join a list or tuple");
        }
        _StrStream ss;
        for(int i = 0; i < _list->size(); i++){
            if(i > 0) ss << _self;
            ss << vm->PyStr_AS_C(vm->asStr(_list->operator[](i)));
        }
        return vm->PyStr(ss.str());
    });

    /************ PyList ************/
    _vm->bindMethod<0>("list", "__iter__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0])
        );
    });

    _vm->bindMethod<1>("list", "append", [](VM* vm, const pkpy::Args& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        _self.push_back(args[1]);
        return vm->None;
    });

    _vm->bindMethod<2>("list", "insert", [](VM* vm, const pkpy::Args& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        if(_index < 0) _index += _self.size();
        if(_index < 0) _index = 0;
        if(_index > _self.size()) _index = _self.size();
        _self.insert(_self.begin() + _index, args[2]);
        return vm->None;
    });

    _vm->bindMethod<0>("list", "clear", [](VM* vm, const pkpy::Args& args) {
        vm->PyList_AS_C(args[0]).clear();
        return vm->None;
    });

    _vm->bindMethod<0>("list", "copy", [](VM* vm, const pkpy::Args& args) {
        return vm->PyList(vm->PyList_AS_C(args[0]));
    });

    _vm->bindMethod<1>("list", "__add__", [](VM* vm, const pkpy::Args& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        const PyVarList& _obj = vm->PyList_AS_C(args[1]);
        PyVarList _new_list = _self;
        _new_list.insert(_new_list.end(), _obj.begin(), _obj.end());
        return vm->PyList(_new_list);
    });

    _vm->bindMethod<0>("list", "__len__", [](VM* vm, const pkpy::Args& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethodMulti<1>({"list", "tuple"}, "__getitem__", [](VM* vm, const pkpy::Args& args) {
        bool list = args[0]->is_type(vm->_tp_list);
        const PyVarList& _self = list ? vm->PyList_AS_C(args[0]) : vm->PyTuple_AS_C(args[0]);

        if(args[1]->is_type(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.size());
            PyVarList _new_list;
            for(size_t i = s.start; i < s.stop; i++) _new_list.push_back(_self[i]);
            return list ? vm->PyList(_new_list) : vm->PyTuple(_new_list);
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.size());
        return _self[_index];
    });

    _vm->bindMethod<2>("list", "__setitem__", [](VM* vm, const pkpy::Args& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.size());
        _self[_index] = args[2];
        return vm->None;
    });

    _vm->bindMethod<1>("list", "__delitem__", [](VM* vm, const pkpy::Args& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalized_index(_index, _self.size());
        _self.erase(_self.begin() + _index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bindStaticMethod<1>("tuple", "__new__", [](VM* vm, const pkpy::Args& args) {
        PyVarList _list = vm->PyList_AS_C(vm->call(vm->builtins->attribs["list"], args));
        return vm->PyTuple(_list);
    });

    _vm->bindMethod<0>("tuple", "__iter__", [](VM* vm, const pkpy::Args& args) {
        return vm->PyIter(pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0]));
    });

    _vm->bindMethod<0>("tuple", "__len__", [](VM* vm, const pkpy::Args& args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    /************ PyBool ************/
    _vm->bindStaticMethod<1>("bool", "__new__", CPP_LAMBDA(vm->asBool(args[0])));

    _vm->bindMethod<0>("bool", "__repr__", [](VM* vm, const pkpy::Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "True" : "False");
    });

    _vm->bindMethod<0>("bool", "__json__", [](VM* vm, const pkpy::Args& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "true" : "false");
    });

    _vm->bindMethod<1>("bool", "__xor__", [](VM* vm, const pkpy::Args& args) {
        bool _self = vm->PyBool_AS_C(args[0]);
        bool _obj = vm->PyBool_AS_C(args[1]);
        return vm->PyBool(_self ^ _obj);
    });

    _vm->bindMethod<0>("ellipsis", "__repr__", CPP_LAMBDA(vm->PyStr("Ellipsis")));
}

#include "builtins.h"

#ifdef _WIN32
#define __EXPORT __declspec(dllexport)
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used))
#elif __EMSCRIPTEN__
#define __EXPORT EMSCRIPTEN_KEEPALIVE
#define __NO_MAIN
#else
#define __EXPORT
#endif


void __add_module_time(VM* vm){
    PyVar mod = vm->new_module("time");
    vm->bindFunc<0>(mod, "time", [](VM* vm, const pkpy::Args& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return vm->PyFloat(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void __add_module_sys(VM* vm){
    PyVar mod = vm->new_module("sys");
    vm->bindFunc<1>(mod, "getrefcount", [](VM* vm, const pkpy::Args& args) {
        return vm->PyInt(args[0].use_count());
    });

    vm->bindFunc<0>(mod, "getrecursionlimit", [](VM* vm, const pkpy::Args& args) {
        return vm->PyInt(vm->maxRecursionDepth);
    });

    vm->bindFunc<1>(mod, "setrecursionlimit", [](VM* vm, const pkpy::Args& args) {
        vm->maxRecursionDepth = (int)vm->PyInt_AS_C(args[0]);
        return vm->None;
    });

    vm->setattr(mod, "version", vm->PyStr(PK_VERSION));
}

void __add_module_json(VM* vm){
    PyVar mod = vm->new_module("json");
    vm->bindFunc<1>(mod, "loads", [](VM* vm, const pkpy::Args& args) {
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->_locals);
    });

    vm->bindFunc<1>(mod, "dumps", CPP_LAMBDA(vm->call(args[0], __json__)));
}

void __add_module_math(VM* vm){
    PyVar mod = vm->new_module("math");
    vm->setattr(mod, "pi", vm->PyFloat(3.1415926535897932384));
    vm->setattr(mod, "e" , vm->PyFloat(2.7182818284590452354));

    vm->bindFunc<1>(mod, "log", CPP_LAMBDA(vm->PyFloat(log(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "log10", CPP_LAMBDA(vm->PyFloat(log10(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "log2", CPP_LAMBDA(vm->PyFloat(log2(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "sin", CPP_LAMBDA(vm->PyFloat(sin(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "cos", CPP_LAMBDA(vm->PyFloat(cos(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "tan", CPP_LAMBDA(vm->PyFloat(tan(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "isnan", CPP_LAMBDA(vm->PyBool(std::isnan(vm->num_to_float(args[0])))));
    vm->bindFunc<1>(mod, "isinf", CPP_LAMBDA(vm->PyBool(std::isinf(vm->num_to_float(args[0])))));
}

void __add_module_dis(VM* vm){
    PyVar mod = vm->new_module("dis");
    vm->bindFunc<1>(mod, "dis", [](VM* vm, const pkpy::Args& args) {
        _Code code = vm->PyFunction_AS_C(args[0])->code;
        (*vm->_stdout) << vm->disassemble(code);
        return vm->None;
    });
}

#define PY_CLASS(mod, name) inline static PyVar _tp(VM* vm) { return vm->_modules[#mod]->attribs[#name]; }

struct ReMatch {
    PY_CLASS(re, Match)

    i64 start;
    i64 end;
    std::smatch m;
    ReMatch(i64 start, i64 end, std::smatch m) : start(start), end(end), m(m) {}

    static PyVar _bind(VM* vm){
        PyVar _tp_match = vm->new_user_type_object(vm->_modules["re"], "Match", vm->_tp_object);
        vm->bindMethod<0>(_tp_match, "start", CPP_LAMBDA(vm->PyInt(UNION_GET(ReMatch, args[0]).start)));
        vm->bindMethod<0>(_tp_match, "end", CPP_LAMBDA(vm->PyInt(UNION_GET(ReMatch, args[0]).end)));

        vm->bindMethod<0>(_tp_match, "span", [](VM* vm, const pkpy::Args& args) {
            auto& m = UNION_GET(ReMatch, args[0]);
            return vm->PyTuple({ vm->PyInt(m.start), vm->PyInt(m.end) });
        });

        vm->bindMethod<1>(_tp_match, "group", [](VM* vm, const pkpy::Args& args) {
            auto& m = UNION_GET(ReMatch, args[0]);
            int index = (int)vm->PyInt_AS_C(args[1]);
            index = vm->normalized_index(index, m.m.size());
            return vm->PyStr(m.m[index].str());
        });
        return _tp_match;
    }
};

PyVar __regex_search(const _Str& pattern, const _Str& string, bool fromStart, VM* vm){
    std::regex re(pattern);
    std::smatch m;
    if(std::regex_search(string, m, re)){
        if(fromStart && m.position() != 0) return vm->None;
        i64 start = string.__to_u8_index(m.position());
        i64 end = string.__to_u8_index(m.position() + m.length());
        return vm->new_object_c<ReMatch>(start, end, m);
    }
    return vm->None;
};

void __add_module_re(VM* vm){
    PyVar mod = vm->new_module("re");
    ReMatch::_bind(vm);

    vm->bindFunc<2>(mod, "match", [](VM* vm, const pkpy::Args& args) {
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& string = vm->PyStr_AS_C(args[1]);
        return __regex_search(pattern, string, true, vm);
    });

    vm->bindFunc<2>(mod, "search", [](VM* vm, const pkpy::Args& args) {
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& string = vm->PyStr_AS_C(args[1]);
        return __regex_search(pattern, string, false, vm);
    });

    vm->bindFunc<3>(mod, "sub", [](VM* vm, const pkpy::Args& args) {
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& repl = vm->PyStr_AS_C(args[1]);
        const _Str& string = vm->PyStr_AS_C(args[2]);
        std::regex re(pattern);
        return vm->PyStr(std::regex_replace(string, re, repl));
    });

    vm->bindFunc<2>(mod, "split", [](VM* vm, const pkpy::Args& args) {
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& string = vm->PyStr_AS_C(args[1]);
        std::regex re(pattern);
        std::sregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::sregex_token_iterator end;
        PyVarList vec;
        for(; it != end; ++it){
            vec.push_back(vm->PyStr(it->str()));
        }
        return vm->PyList(vec);
    });
}

class _PkExported{
public:
    virtual ~_PkExported() = default;
    virtual void* get() = 0;
};

static std::vector<_PkExported*> _pkLookupTable;

template<typename T>
class PkExported : public _PkExported{
    T* _ptr;
public:
    template<typename... Args>
    PkExported(Args&&... args) {
        _ptr = new T(std::forward<Args>(args)...);
        _pkLookupTable.push_back(this);
    }
    
    ~PkExported() override { delete _ptr; }
    void* get() override { return _ptr; }
    operator T*() { return _ptr; }
};

#define pkpy_allocate(T, ...) *(new PkExported<T>(__VA_ARGS__))


extern "C" {
    __EXPORT
    /// Delete a pointer allocated by `pkpy_xxx_xxx`.
    /// It can be `VM*`, `REPL*`, `char*`, etc.
    /// 
    /// !!!
    /// If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
    /// !!!
    void pkpy_delete(void* p){
        for(int i = 0; i < _pkLookupTable.size(); i++){
            if(_pkLookupTable[i]->get() == p){
                delete _pkLookupTable[i];
                _pkLookupTable.erase(_pkLookupTable.begin() + i);
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
            _Str _repr = vm->PyStr_AS_C(vm->asRepr(it->second));
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
            _Str _repr = vm->PyStr_AS_C(vm->asRepr(ret));
            return strdup(_repr.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Create a REPL, using the given virtual machine as the backend.
    REPL* pkpy_new_repl(VM* vm){
        return pkpy_allocate(REPL, vm);
    }

    __EXPORT
    /// Input a source line to an interactive console.
    int pkpy_repl_input(REPL* r, const char* line){
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
        VM* vm = pkpy_allocate(VM, use_stdio);
        __initializeBuiltinFunctions(vm);
        __add_module_sys(vm);
        __add_module_time(vm);
        __add_module_json(vm);
        __add_module_math(vm);
        __add_module_re(vm);
        __add_module_dis(vm);

        // add builtins | no exception handler | must succeed
        _Code code = vm->compile(__BUILTINS_CODE, "<builtins>", EXEC_MODE);
        vm->_exec(code, vm->builtins, pkpy::make_shared<PyVarDict>());

        pkpy_vm_add_module(vm, "random", __RANDOM_CODE);
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
        _StrStream* s_out = (_StrStream*)(vm->_stdout);
        _StrStream* s_err = (_StrStream*)(vm->_stderr);
        _Str _stdout = s_out->str();
        _Str _stderr = s_err->str();
        _StrStream ss;
        ss << '{' << "\"stdout\": " << _stdout.__escape(false);
        ss << ", " << "\"stderr\": " << _stderr.__escape(false) << '}';
        s_out->str(""); s_err->str("");
        return strdup(ss.str().c_str());
    }
}

#include "_bindings.h"