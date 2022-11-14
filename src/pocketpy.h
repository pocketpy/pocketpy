#pragma once

#include "vm.h"
#include "compiler.h"
#include "repl.h"

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, PyVarList args){               \
        if(!vm->isIntOrFloat(args[0], args[1]))                                                         \
            vm->typeError("unsupported operand type(s) for " #op );                        \
        if(args[0]->isType(vm->_tp_int) && args[1]->isType(vm->_tp_int)){                               \
            return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1]));                       \
        }else{                                                                                          \
            return vm->PyFloat(vm->numToFloat(args[0]) op vm->numToFloat(args[1]));                     \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, fallback)                                                        \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, PyVarList args){               \
        if(!vm->isIntOrFloat(args[0], args[1])){                                                        \
            if constexpr(fallback) return vm->PyBool(args[0] op args[1]);                               \
            vm->typeError("unsupported operand type(s) for " #op );                        \
        }                                                                                               \
        return vm->PyBool(vm->numToFloat(args[0]) op vm->numToFloat(args[1]));                          \
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

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bindBuiltinFunc("print", [](VM* vm, PyVarList args) {
        _StrStream ss;
        for (auto& arg : args) ss << vm->PyStr_AS_C(vm->asStr(arg)) << " ";
        vm->_stdout(vm, ss.str().c_str());
        vm->_stdout(vm, "\n");
        return vm->None;
    });

    _vm->bindBuiltinFunc("input", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 0);
        ThreadedVM* tvm = dynamic_cast<ThreadedVM*>(vm);
        if(tvm == nullptr) vm->typeError("input() can only be called in threaded mode");
        tvm->suspend();
        return vm->PyStr(tvm->readStdin());
    });

    _vm->bindBuiltinFunc("eval", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = compile(vm, expr.c_str(), "<eval>", EVAL_MODE);
        if(code == nullptr) return vm->None;
        return vm->_exec(code, vm->topFrame()->_module, vm->topFrame()->f_locals);
    });

    _vm->bindBuiltinFunc("isinstance", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 2);
        return vm->PyBool(vm->isInstance(args[0], args[1]));
    });

    _vm->bindBuiltinFunc("repr", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        return vm->asRepr(args[0]);
    });

    _vm->bindBuiltinFunc("hash", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        return vm->PyInt(vm->hash(args[0]));
    });

    _vm->bindBuiltinFunc("chr", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        _Int i = vm->PyInt_AS_C(args[0]);
        if (i < 0 || i > 128) vm->valueError("chr() arg not in range(128)");
        return vm->PyStr(_Str(1, (char)i));
    });

    _vm->bindBuiltinFunc("ord", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        _Str s = vm->PyStr_AS_C(args[0]);
        if (s.size() != 1) vm->typeError("ord() expected an ASCII character");
        return vm->PyInt((_Int)s[0]);
    });

    _vm->bindBuiltinFunc("globals", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 0);
        const auto& d = vm->topFrame()->f_globals();
        PyVar obj = vm->call(vm->builtins->attribs["dict"], {});
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, {vm->PyStr(k), v});
        }
        return obj;
    });

    _vm->bindBuiltinFunc("locals", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 0);
        const auto& d = vm->topFrame()->f_locals;
        PyVar obj = vm->call(vm->builtins->attribs["dict"], {});
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, {vm->PyStr(k), v});
        }
        return obj;
    });

    _vm->bindBuiltinFunc("dir", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        PyVarList ret;
        for (auto& [k, _] : args[0]->attribs) ret.push_back(vm->PyStr(k));
        return vm->PyList(ret);
    });

    _vm->bindMethod("object", "__repr__", [](VM* vm, PyVarList args) {
        PyVar _self = args[0];
        _Str s = "<" + _self->getTypeName() + " object at " + std::to_string((uintptr_t)_self.get()) + ">";
        return vm->PyStr(s);
    });

    _vm->bindMethod("type", "__new__", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        return args[0]->attribs[__class__];
    });

    _vm->bindMethod("range", "__new__", [](VM* vm, PyVarList args) {
        _Range r;
        switch (args.size()) {
            case 1: r.stop = vm->PyInt_AS_C(args[0]); break;
            case 2: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); break;
            case 3: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); r.step = vm->PyInt_AS_C(args[2]); break;
            default: vm->typeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return vm->PyRange(r);
    });

    _vm->bindMethod("range", "__iter__", [](VM* vm, PyVarList args) {
        vm->__checkType(args[0], vm->_tp_range);
        auto iter = std::make_shared<RangeIterator>(vm, args[0]);
        return vm->PyIter(iter);
    });

    _vm->bindMethod("NoneType", "__repr__", [](VM* vm, PyVarList args) {
        return vm->PyStr("None");
    });

    _vm->bindMethod("NoneType", "__json__", [](VM* vm, PyVarList args) {
        return vm->PyStr("null");
    });

    _vm->bindMethodMulti({"int", "float"}, "__truediv__", [](VM* vm, PyVarList args) {
        if(!vm->isIntOrFloat(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "/" );
        _Float rhs = vm->numToFloat(args[1]);
        if (rhs == 0) vm->zeroDivisionError();
        return vm->PyFloat(vm->numToFloat(args[0]) / rhs);
    });

    _vm->bindMethodMulti({"int", "float"}, "__pow__", [](VM* vm, PyVarList args) {
        if(!vm->isIntOrFloat(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "**" );
        if(args[0]->isType(vm->_tp_int) && args[1]->isType(vm->_tp_int)){
            return vm->PyInt((_Int)round(pow(vm->PyInt_AS_C(args[0]), vm->PyInt_AS_C(args[1]))));
        }else{
            return vm->PyFloat((_Float)pow(vm->numToFloat(args[0]), vm->numToFloat(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bindMethod("int", "__new__", [](VM* vm, PyVarList args) {
        if(args.size() == 0) return vm->PyInt(0);
        vm->__checkArgSize(args, 1);
        if (args[0]->isType(vm->_tp_int)) return args[0];
        if (args[0]->isType(vm->_tp_float)) return vm->PyInt((_Int)vm->PyFloat_AS_C(args[0]));
        if (args[0]->isType(vm->_tp_bool)) return vm->PyInt(vm->PyBool_AS_C(args[0]) ? 1 : 0);
        if (args[0]->isType(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            try{
                _Int val = std::stoll(s.str());
                return vm->PyInt(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for int(): '" + s + "'");
            }
        }
        vm->typeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod("int", "__floordiv__", [](VM* vm, PyVarList args) {
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "//" );
        _Int rhs = vm->PyInt_AS_C(args[1]);
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args[0]) / rhs);
    });

    _vm->bindMethod("int", "__mod__", [](VM* vm, PyVarList args) {
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "%" );
        return vm->PyInt(vm->PyInt_AS_C(args[0]) % vm->PyInt_AS_C(args[1]));
    });

    _vm->bindMethod("int", "__repr__", [](VM* vm, PyVarList args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

    _vm->bindMethod("int", "__json__", [](VM* vm, PyVarList args) {
        return vm->PyStr(std::to_string((int)vm->PyInt_AS_C(args[0])));
    });

#define __INT_BITWISE_OP(name,op) \
    _vm->bindMethod("int", #name, [](VM* vm, PyVarList args) { \
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int)) \
            vm->typeError("unsupported operand type(s) for " #op ); \
        return vm->PyInt(vm->PyInt_AS_C(args[0]) op vm->PyInt_AS_C(args[1])); \
    });

    __INT_BITWISE_OP(__lshift__, <<)
    __INT_BITWISE_OP(__rshift__, >>)
    __INT_BITWISE_OP(__and__, &)
    __INT_BITWISE_OP(__or__, |)
    __INT_BITWISE_OP(__xor__, ^)

#undef __INT_BITWISE_OP

    _vm->bindMethod("int", "__xor__", [](VM* vm, PyVarList args) {
        if(!args[0]->isType(vm->_tp_int) || !args[1]->isType(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "^" );
        return vm->PyInt(vm->PyInt_AS_C(args[0]) ^ vm->PyInt_AS_C(args[1]));
    });

    /************ PyFloat ************/
    _vm->bindMethod("float", "__new__", [](VM* vm, PyVarList args) {
        if(args.size() == 0) return vm->PyFloat(0.0);
        vm->__checkArgSize(args, 1);
        if (args[0]->isType(vm->_tp_int)) return vm->PyFloat((_Float)vm->PyInt_AS_C(args[0]));
        if (args[0]->isType(vm->_tp_float)) return args[0];
        if (args[0]->isType(vm->_tp_bool)) return vm->PyFloat(vm->PyBool_AS_C(args[0]) ? 1.0 : 0.0);
        if (args[0]->isType(vm->_tp_str)) {
            const _Str& s = vm->PyStr_AS_C(args[0]);
            if(s == "inf") return vm->PyFloat(_FLOAT_INF_POS);
            if(s == "-inf") return vm->PyFloat(_FLOAT_INF_NEG);
            try{
                _Float val = std::stod(s.str());
                return vm->PyFloat(val);
            }catch(std::invalid_argument&){
                vm->valueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->typeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bindMethod("float", "__repr__", [](VM* vm, PyVarList args) {
        _Float val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) return vm->PyStr(std::to_string(val));
        _StrStream ss;
        ss << std::setprecision(std::numeric_limits<_Float>::max_digits10-1) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return vm->PyStr(s);
    });

    _vm->bindMethod("float", "__json__", [](VM* vm, PyVarList args) {
        return vm->PyStr(std::to_string((float)vm->PyFloat_AS_C(args[0])));
    });

    /************ PyString ************/
    _vm->bindMethod("str", "__new__", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        return vm->asStr(args[0]);
    });

    _vm->bindMethod("str", "__add__", [](VM* vm, PyVarList args) {
        if(!args[0]->isType(vm->_tp_str) || !args[1]->isType(vm->_tp_str))
            vm->typeError("unsupported operand type(s) for " "+" );
        const _Str& lhs = vm->PyStr_AS_C(args[0]);
        const _Str& rhs = vm->PyStr_AS_C(args[1]);
        return vm->PyStr(lhs + rhs);
    });

    _vm->bindMethod("str", "__len__", [](VM* vm, PyVarList args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyInt(_self.u8_length());
    });

    _vm->bindMethod("str", "__contains__", [](VM* vm, PyVarList args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _other = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().find(_other.str()) != _Str::npos);
    });

    _vm->bindMethod("str", "__str__", [](VM* vm, PyVarList args) {
        return args[0]; // str is immutable
    });

    _vm->bindMethod("str", "__iter__", [](VM* vm, PyVarList args) {
        auto it = std::make_shared<StringIterator>(vm, args[0]);
        return vm->PyIter(it);
    });

    _vm->bindMethod("str", "__repr__", [](VM* vm, PyVarList args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr("'" + _self.__escape(true).str() + "'");
    });

    _vm->bindMethod("str", "__json__", [](VM* vm, PyVarList args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr("\"" + _self.__escape(false) + "\"");
    });

    _vm->bindMethod("str", "__eq__", [](VM* vm, PyVarList args) {
        if(args[0]->isType(vm->_tp_str) && args[1]->isType(vm->_tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) == vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] == args[1]);      // fallback
    });

    _vm->bindMethod("str", "__getitem__", [](VM* vm, PyVarList args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));

        if(args[1]->isType(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.u8_length());
            return vm->PyStr(_self.u8_substr(s.start, s.stop));
        }

        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.u8_length());
        return vm->PyStr(_self.u8_getitem(_index));
    });

    _vm->bindMethod("str", "__gt__", [](VM* vm, PyVarList args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self > _obj);
    });

    _vm->bindMethod("str", "__lt__", [](VM* vm, PyVarList args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self < _obj);
    });

    _vm->bindMethod("str", "upper", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1, true);
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        _StrStream ss;
        for(auto c : _self.str()) ss << (char)toupper(c);
        return vm->PyStr(ss);
    });

    _vm->bindMethod("str", "lower", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1, true);
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        _StrStream ss;
        for(auto c : _self.str()) ss << (char)tolower(c);
        return vm->PyStr(ss);
    });

    _vm->bindMethod("str", "replace", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 3, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _old = vm->PyStr_AS_C(args[1]);
        const _Str& _new = vm->PyStr_AS_C(args[2]);
        std::string _copy = _self.str();
        // replace all occurences of _old with _new in _copy
        size_t pos = 0;
        while ((pos = _copy.find(_old.str(), pos)) != std::string::npos) {
            _copy.replace(pos, _old.str().length(), _new.str());
            pos += _new.str().length();
        }
        return vm->PyStr(_copy);
    });

    _vm->bindMethod("str", "startswith", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _prefix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().find(_prefix.str()) == 0);
    });

    _vm->bindMethod("str", "endswith", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _suffix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.str().rfind(_suffix.str()) == _self.str().length() - _suffix.str().length());
    });

    _vm->bindMethod("str", "join", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const PyVarList& _list = vm->PyList_AS_C(args[1]);
        _StrStream ss;
        for(int i = 0; i < _list.size(); i++){
            if(i > 0) ss << _self;
            ss << vm->PyStr_AS_C(vm->asStr(_list[i]));
        }
        return vm->PyStr(ss);
    });

    /************ PyList ************/
    _vm->bindMethod("list", "__iter__", [](VM* vm, PyVarList args) {
        vm->__checkType(args[0], vm->_tp_list);
        auto iter = std::make_shared<VectorIterator>(vm, args[0]);
        return vm->PyIter(iter);
    });

    _vm->bindMethod("list", "append", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 2, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        _self.push_back(args[1]);
        return vm->None;
    });

    _vm->bindMethod("list", "insert", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 3, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self.insert(_self.begin() + _index, args[2]);
        return vm->None;
    });

    _vm->bindMethod("list", "clear", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1, true);
        vm->PyList_AS_C(args[0]).clear();
        return vm->None;
    });

    _vm->bindMethod("list", "copy", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1, true);
        return vm->PyList(vm->PyList_AS_C(args[0]));
    });

    _vm->bindMethod("list", "pop", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        if(_self.empty()) vm->indexError("pop from empty list");
        PyVar ret = _self.back();
        _self.pop_back();
        return ret;
    });      

    _vm->bindMethod("list", "__add__", [](VM* vm, PyVarList args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        const PyVarList& _obj = vm->PyList_AS_C(args[1]);
        PyVarList _new_list = _self;
        _new_list.insert(_new_list.end(), _obj.begin(), _obj.end());
        return vm->PyList(_new_list);
    });

    _vm->bindMethod("list", "__len__", [](VM* vm, PyVarList args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("list", "__getitem__", [](VM* vm, PyVarList args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);

        if(args[1]->isType(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.size());
            PyVarList _new_list;
            for(int i = s.start; i < s.stop; i++)
                _new_list.push_back(_self[i]);
            return vm->PyList(_new_list);
        }

        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    _vm->bindMethod("list", "__setitem__", [](VM* vm, PyVarList args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self[_index] = args[2];
        return vm->None;
    });

    _vm->bindMethod("list", "__delitem__", [](VM* vm, PyVarList args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self.erase(_self.begin() + _index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bindMethod("tuple", "__new__", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        PyVarList _list = vm->PyList_AS_C(vm->call(vm->builtins->attribs["list"], args));
        return vm->PyTuple(_list);
    });

    _vm->bindMethod("tuple", "__iter__", [](VM* vm, PyVarList args) {
        vm->__checkType(args[0], vm->_tp_tuple);
        auto iter = std::make_shared<VectorIterator>(vm, args[0]);
        return vm->PyIter(iter);
    });

    _vm->bindMethod("tuple", "__len__", [](VM* vm, PyVarList args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("tuple", "__getitem__", [](VM* vm, PyVarList args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        int _index = vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    /************ PyBool ************/
    _vm->bindMethod("bool", "__repr__", [](VM* vm, PyVarList args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "True" : "False");
    });

    _vm->bindMethod("bool", "__json__", [](VM* vm, PyVarList args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "true" : "false");
    });

    _vm->bindMethod("bool", "__eq__", [](VM* vm, PyVarList args) {
        return vm->PyBool(args[0] == args[1]);
    });
}

#include "builtins.h"

#ifdef _WIN32
#define __EXPORT __declspec(dllexport)
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used))
#else
#define __EXPORT
#endif


void __addModuleTime(VM* vm){
    PyVar mod = vm->newModule("time");
    vm->bindFunc(mod, "time", [](VM* vm, PyVarList args) {
        auto now = std::chrono::high_resolution_clock::now();
        return vm->PyFloat(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void __addModuleSys(VM* vm){
    PyVar mod = vm->newModule("sys");
    vm->bindFunc(mod, "getrefcount", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        return vm->PyInt(args[0].use_count());
    });

    vm->bindFunc(mod, "getrecursionlimit", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 0);
        return vm->PyInt(vm->maxRecursionDepth);
    });

    vm->bindFunc(mod, "setrecursionlimit", [](VM* vm, PyVarList args) {
        vm->__checkArgSize(args, 1);
        vm->maxRecursionDepth = vm->PyInt_AS_C(args[0]);
        return vm->None;
    });

    vm->setAttr(mod, "version", vm->PyStr(PK_VERSION));
}


extern "C" {
    __EXPORT
    struct PyObjectDump: public PkExportedResource{
        const char* type;   // "int", "str", "float" ...
        const char* json;   // json representation

        PyObjectDump(const char* _type, const char* _json){
            type = strdup(_type);
            json = strdup(_json);
        }

        ~PyObjectDump(){
            delete[] type;
            delete[] json;
        }
    };

    __EXPORT
    void pkpy_delete(PkExportedResource* p){
        delete p;
    }

    __EXPORT
    bool pkpy_exec(VM* vm, const char* source){
        _Code code = compile(vm, source, "main.py");
        if(code == nullptr) return false;
        return vm->exec(code) != nullptr;
    }

    __EXPORT
    PyObjectDump* pkpy_eval(VM* vm, const char* source){
        _Code code = compile(vm, source, "<eval>", EVAL_MODE);
        if(code == nullptr) return nullptr;
        PyVar ret = vm->exec(code);
        if(ret == nullptr) return nullptr;
        return new PyObjectDump(
            ret->getTypeName().c_str(),
            vm->PyStr_AS_C(vm->asJson(ret)).c_str()
        );
    }

    __EXPORT
    REPL* pkpy_new_repl(VM* vm, bool use_prompt){
        return new REPL(vm, use_prompt);
    }

    __EXPORT
    bool pkpy_repl_input(REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    bool pkpy_add_module(VM* vm, const char* name, const char* source){
        _Code code = compile(vm, source, name + _Str(".py"));
        if(code == nullptr) return false;
        PyVar _m = vm->newModule(name);
        return vm->exec(code, _m) != nullptr;
    }

    void __vm_init(VM* vm, PrintFn _stdout, PrintFn _stderr){
        __initializeBuiltinFunctions(vm);
        vm->_stdout = _stdout;
        vm->_stderr = _stderr;

        _Code code = compile(vm, __BUILTINS_CODE, "<builtins>");
        if(code == nullptr) exit(1);
        vm->_exec(code, vm->builtins);

        __addModuleSys(vm);
        __addModuleTime(vm);
        pkpy_add_module(vm, "random", __RANDOM_CODE);
    }

    __EXPORT
    VM* pkpy_new_vm(PrintFn _stdout, PrintFn _stderr){
        VM* vm = new VM();
        __vm_init(vm, _stdout, _stderr);
        return vm;
    }

    __EXPORT
    ThreadedVM* pkpy_new_tvm(PrintFn _stdout, PrintFn _stderr){
        ThreadedVM* vm = new ThreadedVM();
        __vm_init(vm, _stdout, _stderr);
        return vm;
    }

    __EXPORT
    int pkpy_tvm_get_state(ThreadedVM* vm){
        return vm->getState();
    }

    __EXPORT
    void pkpy_tvm_start_exec(ThreadedVM* vm, const char* source){
        _Code code = compile(vm, source, "main.py");
        if(code == nullptr) return;
        return vm->startExec(code);
    }

    __EXPORT
    void pkpy_tvm_write_stdin(ThreadedVM* vm, const char* line){
        vm->_stdin = _Str(line);
    }

    __EXPORT
    void pkpy_tvm_resume(ThreadedVM* vm){
        vm->resume();
    }
}