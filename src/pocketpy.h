#pragma once

#include "vm.h"
#include "compiler.h"
#include "repl.h"

_Code VM::compile(_Str source, _Str filename, CompileMode mode) {
    Compiler compiler(this, source.c_str(), filename, mode);
    try{
        return compiler.__fillCode();
    }catch(_Error& e){
        throw e;
    }catch(std::exception& e){
        throw CompileError("UnexpectedError", e.what(), compiler.getLineSnapshot());
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, const pkpy::ArgList& args){                 \
        if(!vm->is_int_or_float(args[0], args[1]))                                                         \
            vm->typeError("unsupported operand type(s) for " #op );                                     \
        if(args._index(0)->is_type(vm->_tp_int) && args._index(1)->is_type(vm->_tp_int)){                 \
            return vm->PyInt(vm->PyInt_AS_C(args._index(0)) op vm->PyInt_AS_C(args._index(1)));         \
        }else{                                                                                          \
            return vm->PyFloat(vm->num_to_float(args._index(0)) op vm->num_to_float(args._index(1)));       \
        }                                                                                               \
    });

#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->bindMethodMulti({"int","float"}, #name, [](VM* vm, const pkpy::ArgList& args){                 \
        if(!vm->is_int_or_float(args[0], args[1])){                                                        \
            if constexpr(is_eq) return vm->PyBool(args[0] == args[1]);                                  \
            vm->typeError("unsupported operand type(s) for " #op );                                     \
        }                                                                                               \
        return vm->PyBool(vm->num_to_float(args._index(0)) op vm->num_to_float(args._index(1)));            \
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

    _vm->bindBuiltinFunc("__sys_stdout_write", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        (*vm->_stdout) << vm->PyStr_AS_C(args[0]);
        return vm->None;
    });

    _vm->bindBuiltinFunc("super", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 0);
        auto it = vm->top_frame()->f_locals.find(m_self);
        if(it == vm->top_frame()->f_locals.end()) vm->typeError("super() can only be called in a class method");
        return vm->new_object(vm->_tp_super, it->second);
    });

    _vm->bindBuiltinFunc("eval", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = vm->compile(expr, "<eval>", EVAL_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->f_locals_copy());
    });

    _vm->bindBuiltinFunc("repr", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->asRepr(args[0]);
    });

    _vm->bindBuiltinFunc("hash", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyInt(vm->hash(args[0]));
    });

    _vm->bindBuiltinFunc("len", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->call(args[0], __len__, pkpy::noArg());
    });

    _vm->bindBuiltinFunc("chr", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        i64 i = vm->PyInt_AS_C(args[0]);
        if (i < 0 || i > 128) vm->valueError("chr() arg not in range(128)");
        return vm->PyStr(std::string(1, (char)i));
    });

    _vm->bindBuiltinFunc("ord", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        _Str s = vm->PyStr_AS_C(args[0]);
        if (s.size() != 1) vm->typeError("ord() expected an ASCII character");
        return vm->PyInt((i64)s[0]);
    });

    _vm->bindBuiltinFunc("globals", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 0);
        const auto& d = vm->top_frame()->f_globals();
        PyVar obj = vm->call(vm->builtins->attribs["dict"]);
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, pkpy::twoArgs(vm->PyStr(k), v));
        }
        return obj;
    });

    _vm->bindBuiltinFunc("locals", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 0);
        const auto& d = vm->top_frame()->f_locals;
        PyVar obj = vm->call(vm->builtins->attribs["dict"]);
        for (const auto& [k, v] : d) {
            vm->call(obj, __setitem__, pkpy::twoArgs(vm->PyStr(k), v));
        }
        return obj;
    });

    _vm->bindBuiltinFunc("hex", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        std::stringstream ss;
        ss << std::hex << vm->PyInt_AS_C(args[0]);
        return vm->PyStr("0x" + ss.str());
    });

    _vm->bindBuiltinFunc("dir", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        std::vector<_Str> names;
        for (auto& [k, _] : args[0]->attribs) names.push_back(k);
        for (auto& [k, _] : args[0]->_type->attribs) {
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

    _vm->bindMethod("object", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        PyVar _self = args[0];
        std::stringstream ss;
        ss << std::hex << (uintptr_t)_self.get();
        _Str s = "<" + UNION_TP_NAME(_self) + " object at 0x" + ss.str() + ">";
        return vm->PyStr(s);
    });

    _vm->bindMethod("type", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return args[0]->_type;
    });

    _vm->bindMethod("type", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethod("range", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        _Range r;
        switch (args.size()) {
            case 1: r.stop = vm->PyInt_AS_C(args[0]); break;
            case 2: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); break;
            case 3: r.start = vm->PyInt_AS_C(args[0]); r.stop = vm->PyInt_AS_C(args[1]); r.step = vm->PyInt_AS_C(args[2]); break;
            default: vm->typeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return vm->PyRange(r);
    });

    _vm->bindMethod("range", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_type(args[0], vm->_tp_range);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, RangeIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("NoneType", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("None");
    });

    _vm->bindMethod("NoneType", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("null");
    });

    _vm->bindMethod("NoneType", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethodMulti({"int", "float"}, "__truediv__", [](VM* vm, const pkpy::ArgList& args) {
        if(!vm->is_int_or_float(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "/" );
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->zeroDivisionError();
        return vm->PyFloat(vm->num_to_float(args[0]) / rhs);
    });

    _vm->bindMethodMulti({"int", "float"}, "__pow__", [](VM* vm, const pkpy::ArgList& args) {
        if(!vm->is_int_or_float(args[0], args[1]))
            vm->typeError("unsupported operand type(s) for " "**" );
        if(args[0]->is_type(vm->_tp_int) && args[1]->is_type(vm->_tp_int)){
            return vm->PyInt((i64)round(pow(vm->PyInt_AS_C(args[0]), vm->PyInt_AS_C(args[1]))));
        }else{
            return vm->PyFloat((f64)pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    });

    /************ PyInt ************/
    _vm->bindMethod("int", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        if(args.size() == 0) return vm->PyInt(0);
        vm->check_args_size(args, 1);
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

    _vm->bindMethod("int", "__floordiv__", [](VM* vm, const pkpy::ArgList& args) {
        if(!args[0]->is_type(vm->_tp_int) || !args[1]->is_type(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "//" );
        i64 rhs = vm->PyInt_AS_C(args._index(1));
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) / rhs);
    });

    _vm->bindMethod("int", "__mod__", [](VM* vm, const pkpy::ArgList& args) {
        if(!args[0]->is_type(vm->_tp_int) || !args[1]->is_type(vm->_tp_int))
            vm->typeError("unsupported operand type(s) for " "%" );
        i64 rhs = vm->PyInt_AS_C(args._index(1));
        if(rhs == 0) vm->zeroDivisionError();
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) % rhs);
    });

    _vm->bindMethod("int", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr(std::to_string(vm->PyInt_AS_C(args[0])));
    });

    _vm->bindMethod("int", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr(std::to_string((int)vm->PyInt_AS_C(args[0])));
    });

#define __INT_BITWISE_OP(name,op) \
    _vm->bindMethod("int", #name, [](VM* vm, const pkpy::ArgList& args) {                       \
        vm->check_args_size(args, 2, true);                                                     \
        return vm->PyInt(vm->PyInt_AS_C(args._index(0)) op vm->PyInt_AS_C(args._index(1)));     \
    });

    __INT_BITWISE_OP(__lshift__, <<)
    __INT_BITWISE_OP(__rshift__, >>)
    __INT_BITWISE_OP(__and__, &)
    __INT_BITWISE_OP(__or__, |)
    __INT_BITWISE_OP(__xor__, ^)

#undef __INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bindMethod("float", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        if(args.size() == 0) return vm->PyFloat(0.0);
        vm->check_args_size(args, 1);
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

    _vm->bindMethod("float", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) return vm->PyStr(std::to_string(val));
        _StrStream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return vm->PyStr(s);
    });

    _vm->bindMethod("float", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        f64 val = vm->PyFloat_AS_C(args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->valueError("cannot jsonify 'nan' or 'inf'");
        return vm->PyStr(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bindMethod("str", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->asStr(args._index(0));
    });

    _vm->bindMethod("str", "__add__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& lhs = vm->PyStr_AS_C(args[0]);
        const _Str& rhs = vm->PyStr_AS_C(args[1]);
        return vm->PyStr(lhs + rhs);
    });

    _vm->bindMethod("str", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyInt(_self.u8_length());
    });

    _vm->bindMethod("str", "__contains__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _other = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_other) != _Str::npos);
    });

    _vm->bindMethod("str", "__str__", [](VM* vm, const pkpy::ArgList& args) {
        return args[0]; // str is immutable
    });

    _vm->bindMethod("str", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyIter(pkpy::make_shared<BaseIterator, StringIterator>(vm, args[0]));
    });

    _vm->bindMethod("str", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(true));
    });

    _vm->bindMethod("str", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        return vm->PyStr(_self.__escape(false));
    });

    _vm->bindMethod("str", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        if(args[0]->is_type(vm->_tp_str) && args[1]->is_type(vm->_tp_str))
            return vm->PyBool(vm->PyStr_AS_C(args[0]) == vm->PyStr_AS_C(args[1]));
        return vm->PyBool(args[0] == args[1]);      // fallback
    });

    _vm->bindMethod("str", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));

        if(args[1]->is_type(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.u8_length());
            return vm->PyStr(_self.u8_substr(s.start, s.stop));
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.u8_length());
        return vm->PyStr(_self.u8_getitem(_index));
    });

    _vm->bindMethod("str", "__gt__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self > _obj);
    });

    _vm->bindMethod("str", "__lt__", [](VM* vm, const pkpy::ArgList& args) {
        const _Str& _self (vm->PyStr_AS_C(args[0]));
        const _Str& _obj (vm->PyStr_AS_C(args[1]));
        return vm->PyBool(_self < _obj);
    });

    _vm->bindMethod("str", "replace", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 3, true);
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

    _vm->bindMethod("str", "startswith", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _prefix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.find(_prefix) == 0);
    });

    _vm->bindMethod("str", "endswith", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
        const _Str& _self = vm->PyStr_AS_C(args[0]);
        const _Str& _suffix = vm->PyStr_AS_C(args[1]);
        return vm->PyBool(_self.rfind(_suffix) == _self.length() - _suffix.length());
    });

    _vm->bindMethod("str", "join", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
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
    _vm->bindMethod("list", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_type(args[0], vm->_tp_list);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("list", "append", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        _self.push_back(args._index(1));
        return vm->None;
    });

    _vm->bindMethod("list", "insert", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 3, true);
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        if(_index < 0) _index += _self.size();
        if(_index < 0) _index = 0;
        if(_index > _self.size()) _index = _self.size();
        _self.insert(_self.begin() + _index, args[2]);
        return vm->None;
    });

    _vm->bindMethod("list", "clear", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1, true);
        vm->PyList_AS_C(args[0]).clear();
        return vm->None;
    });

    _vm->bindMethod("list", "copy", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1, true);
        return vm->PyList(vm->PyList_AS_C(args[0]));
    });

    _vm->bindMethod("list", "__add__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        const PyVarList& _obj = vm->PyList_AS_C(args[1]);
        PyVarList _new_list = _self;
        _new_list.insert(_new_list.end(), _obj.begin(), _obj.end());
        return vm->PyList(_new_list);
    });

    _vm->bindMethod("list", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("list", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyList_AS_C(args[0]);

        if(args[1]->is_type(vm->_tp_slice)){
            _Slice s = vm->PySlice_AS_C(args[1]);
            s.normalize(_self.size());
            PyVarList _new_list;
            for(size_t i = s.start; i < s.stop; i++)
                _new_list.push_back(_self[i]);
            return vm->PyList(_new_list);
        }

        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    _vm->bindMethod("list", "__setitem__", [](VM* vm, const pkpy::ArgList& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self[_index] = args[2];
        return vm->None;
    });

    _vm->bindMethod("list", "__delitem__", [](VM* vm, const pkpy::ArgList& args) {
        PyVarList& _self = vm->PyList_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        _self.erase(_self.begin() + _index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bindMethod("tuple", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        PyVarList _list = vm->PyList_AS_C(vm->call(vm->builtins->attribs["list"], args));
        return vm->PyTuple(_list);
    });

    _vm->bindMethod("tuple", "__iter__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_type(args[0], vm->_tp_tuple);
        return vm->PyIter(
            pkpy::make_shared<BaseIterator, VectorIterator>(vm, args[0])
        );
    });

    _vm->bindMethod("tuple", "__len__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        return vm->PyInt(_self.size());
    });

    _vm->bindMethod("tuple", "__getitem__", [](VM* vm, const pkpy::ArgList& args) {
        const PyVarList& _self = vm->PyTuple_AS_C(args[0]);
        int _index = (int)vm->PyInt_AS_C(args[1]);
        _index = vm->normalizedIndex(_index, _self.size());
        return _self[_index];
    });

    /************ PyBool ************/
    _vm->bindMethod("bool", "__new__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->asBool(args[0]);
    });

    _vm->bindMethod("bool", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "True" : "False");
    });

    _vm->bindMethod("bool", "__json__", [](VM* vm, const pkpy::ArgList& args) {
        bool val = vm->PyBool_AS_C(args[0]);
        return vm->PyStr(val ? "true" : "false");
    });

    _vm->bindMethod("bool", "__eq__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyBool(args[0] == args[1]);
    });

    _vm->bindMethod("bool", "__xor__", [](VM* vm, const pkpy::ArgList& args) {
        bool _self = vm->PyBool_AS_C(args[0]);
        bool _obj = vm->PyBool_AS_C(args[1]);
        return vm->PyBool(_self ^ _obj);
    });

    _vm->bindMethod("ellipsis", "__repr__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->PyStr("Ellipsis");
    });

    _vm->bindMethod("_native_function", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        const _CppFunc& _self = vm->PyNativeFunction_AS_C(args[0]);
        return _self(vm, args.subList(1));
    });

    _vm->bindMethod("function", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        return vm->call(args[0], args.subList(1));
    });

    _vm->bindMethod("_bounded_method", "__call__", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_type(args[0], vm->_tp_bounded_method);
        const _BoundedMethod& _self = vm->PyBoundedMethod_AS_C(args[0]);
        pkpy::ArgList newArgs(args.size());
        newArgs[0] = _self.obj;
        for(int i = 1; i < args.size(); i++) newArgs[i] = args[i];
        return vm->call(_self.method, newArgs);
    });
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


void __addModuleTime(VM* vm){
    PyVar mod = vm->newModule("time");
    vm->bindFunc(mod, "time", [](VM* vm, const pkpy::ArgList& args) {
        auto now = std::chrono::high_resolution_clock::now();
        return vm->PyFloat(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

void __addModuleSys(VM* vm){
    PyVar mod = vm->newModule("sys");
    vm->bindFunc(mod, "getrefcount", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyInt(args[0].use_count());
    });

    vm->bindFunc(mod, "getrecursionlimit", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 0);
        return vm->PyInt(vm->maxRecursionDepth);
    });

    vm->bindFunc(mod, "setrecursionlimit", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        vm->maxRecursionDepth = (int)vm->PyInt_AS_C(args[0]);
        return vm->None;
    });

    vm->setattr(mod, "version", vm->PyStr(PK_VERSION));
}

void __addModuleJson(VM* vm){
    PyVar mod = vm->newModule("json");
    vm->bindFunc(mod, "loads", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        const _Str& expr = vm->PyStr_AS_C(args[0]);
        _Code code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module, vm->top_frame()->f_locals_copy());
    });

    vm->bindFunc(mod, "dumps", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->asJson(args[0]);
    });
}

void __addModuleMath(VM* vm){
    PyVar mod = vm->newModule("math");
    vm->setattr(mod, "pi", vm->PyFloat(3.1415926535897932384));
    vm->setattr(mod, "e" , vm->PyFloat(2.7182818284590452354));

    vm->bindFunc(mod, "log", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(log(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "log10", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(log10(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "log2", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(log2(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "sin", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(sin(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "cos", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(cos(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "tan", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyFloat(tan(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "isclose", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2);
        f64 a = vm->num_to_float(args[0]);
        f64 b = vm->num_to_float(args[1]);
        return vm->PyBool(fabs(a - b) < 1e-9);
    });

    vm->bindFunc(mod, "isnan", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyBool(std::isnan(vm->num_to_float(args[0])));
    });

    vm->bindFunc(mod, "isinf", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1);
        return vm->PyBool(std::isinf(vm->num_to_float(args[0])));
    });
}


struct ReMatch {
    i64 start;
    i64 end;
    std::smatch m;
    ReMatch(i64 start, i64 end, std::smatch m) : start(start), end(end), m(m) {}
};


PyVar __regex_search(const _Str& pattern, const _Str& string, bool fromStart, VM* vm){
    std::regex re(pattern);
    std::smatch m;
    if(std::regex_search(string, m, re)){
        if(fromStart && m.position() != 0) return vm->None;
        i64 start = string.__to_u8_index(m.position());
        i64 end = string.__to_u8_index(m.position() + m.length());
        return vm->new_object(vm->_userTypes["re.Match"], ReMatch(start, end, m));
    }
    return vm->None;
};

void __addModuleRe(VM* vm){
    PyVar mod = vm->newModule("re");
    PyVar _tp_match = vm->new_user_type_object(mod, "Match", vm->_tp_object);
    vm->bindMethod("re.Match", "start", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1, true);
        return vm->PyInt(UNION_GET(ReMatch, args[0]).start);
    });

    vm->bindMethod("re.Match", "end", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1, true);
        return vm->PyInt(UNION_GET(ReMatch, args[0]).end);
    });

    vm->bindMethod("re.Match", "span", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 1, true);
        auto& m = UNION_GET(ReMatch, args[0]);
        PyVarList vec = { vm->PyInt(m.start), vm->PyInt(m.end) };
        return vm->PyTuple(vec);
    });

    vm->bindMethod("re.Match", "group", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2, true);
        auto& m = UNION_GET(ReMatch, args[0]);
        int index = (int)vm->PyInt_AS_C(args[1]);
        vm->normalizedIndex(index, m.m.size());
        return vm->PyStr(m.m[index].str());
    });

    vm->bindFunc(mod, "match", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2);
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& string = vm->PyStr_AS_C(args[1]);
        return __regex_search(pattern, string, true, vm);
    });

    vm->bindFunc(mod, "search", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2);
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& string = vm->PyStr_AS_C(args[1]);
        return __regex_search(pattern, string, false, vm);
    });

    vm->bindFunc(mod, "sub", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 3);
        const _Str& pattern = vm->PyStr_AS_C(args[0]);
        const _Str& repl = vm->PyStr_AS_C(args[1]);
        const _Str& string = vm->PyStr_AS_C(args[2]);
        std::regex re(pattern);
        return vm->PyStr(std::regex_replace(string, re, repl));
    });

    vm->bindFunc(mod, "split", [](VM* vm, const pkpy::ArgList& args) {
        vm->check_args_size(args, 2);
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
    /// For char*, you can also use trivial `delete` in your language.
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
    /// Return a json representing the result.
    /// If the variable is not found, return `nullptr`.
    char* pkpy_vm_get_global(VM* vm, const char* name){
        auto it = vm->_main->attribs.find(name);
        if(it == vm->_main->attribs.end()) return nullptr;
        try{
            _Str _json = vm->PyStr_AS_C(vm->asJson(it->second));
            return strdup(_json.c_str());
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    /// Evaluate an expression.
    /// 
    /// Return a json representing the result.
    /// If there is any error, return `nullptr`.
    char* pkpy_vm_eval(VM* vm, const char* source){
        PyVarOrNull ret = vm->exec(source, "<eval>", EVAL_MODE);
        if(ret == nullptr) return nullptr;
        try{
            _Str _json = vm->PyStr_AS_C(vm->asJson(ret));
            return strdup(_json.c_str());
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
        vm->addLazyModule(name, source);
    }

    void __vm_init(VM* vm){
        __initializeBuiltinFunctions(vm);
        __addModuleSys(vm);
        __addModuleTime(vm);
        __addModuleJson(vm);
        __addModuleMath(vm);
        __addModuleRe(vm);

        // add builtins | no exception handler | must succeed
        _Code code = vm->compile(__BUILTINS_CODE, "<builtins>", EXEC_MODE);
        vm->_exec(code, vm->builtins, {});

        pkpy_vm_add_module(vm, "random", __RANDOM_CODE);
        pkpy_vm_add_module(vm, "os", __OS_CODE);
    }

    __EXPORT
    /// Create a virtual machine.
    VM* pkpy_new_vm(bool use_stdio){
        VM* vm = pkpy_allocate(VM, use_stdio);
        __vm_init(vm);
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
        ss << ", ";
        ss << "\"stderr\": " << _stderr.__escape(false) << '}';
        s_out->str("");
        s_err->str("");
        return strdup(ss.str().c_str());
    }
}