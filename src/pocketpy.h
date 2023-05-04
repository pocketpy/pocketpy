#pragma once

#include "ceval.h"
#include "compiler.h"
#include "obj.h"
#include "repl.h"
#include "iter.h"
#include "base64.h"
#include "cffi.h"
#include "requests.h"
#include "io.h"
#include "_generated.h"

namespace pkpy {

inline CodeObject_ VM::compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(Exception& e){
#if DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e);
        return nullptr;
    }
}

#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bind_method<1>("int", #name, [](VM* vm, ArgsView args){                                        \
        if(is_int(args[1])){                                                                            \
            return VAR(_CAST(i64, args[0]) op _CAST(i64, args[1]));                                     \
        }else{                                                                                          \
            return VAR(vm->num_to_float(args[0]) op vm->num_to_float(args[1]));                         \
        }                                                                                               \
    });                                                                                                 \
    _vm->bind_method<1>("float", #name, [](VM* vm, ArgsView args){                                      \
        return VAR(_CAST(f64, args[0]) op vm->num_to_float(args[1]));                                   \
    });



#define BIND_NUM_LOGICAL_OPT(name, op, is_eq)                                                           \
    _vm->bind_method<1>("int", #name, [](VM* vm, ArgsView args){                                        \
        if(is_int(args[1]))   return VAR(_CAST(i64, args[0]) op _CAST(i64, args[1]));                   \
        if(is_float(args[1])) return VAR(vm->num_to_float(args[0]) op _CAST(f64, args[1]));             \
        if constexpr(is_eq)   return VAR(args[0] op args[1]);                                             \
        vm->TypeError("unsupported operand type(s) for " #op );                                         \
        return vm->None;                                                                                \
    });                                                                                                 \
    _vm->bind_method<1>("float", #name, [](VM* vm, ArgsView args){                                      \
        if(is_float(args[1])) return VAR(_CAST(f64, args[0]) op _CAST(f64, args[1]));                   \
        if(is_int(args[1]))   return VAR(_CAST(f64, args[0]) op _CAST(i64, args[1]));                   \
        if constexpr(is_eq)   return VAR(args[0] op args[1]);                                             \
        vm->TypeError("unsupported operand type(s) for " #op );                                         \
        return vm->None;                                                                                \
    });
    

inline void init_builtins(VM* _vm) {
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

    _vm->bind_builtin_func<2>("super", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type type = OBJ_GET(Type, args[0]);
        if(!vm->isinstance(args[1], type)){
            Str _0 = obj_type_name(vm, OBJ_GET(Type, vm->_t(args[1])));
            Str _1 = obj_type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        Type base = vm->_all_types[type].base;
        return vm->heap.gcnew(vm->tp_super, Super(args[1], base));
    });

    _vm->bind_builtin_func<2>("isinstance", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type type = OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_builtin_func<0>("globals", [](VM* vm, ArgsView args) {
        PyObject* mod = vm->top_frame()->_module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind_builtin_func<1>("id", [](VM* vm, ArgsView args) {
        PyObject* obj = args[0];
        if(is_tagged(obj)) return VAR((i64)0);
        return VAR(BITS(obj));
    });

    _vm->bind_builtin_func<2>("divmod", [](VM* vm, ArgsView args) {
        i64 lhs = CAST(i64, args[0]);
        i64 rhs = CAST(i64, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(Tuple({VAR(lhs/rhs), VAR(lhs%rhs)}));
    });

    _vm->bind_builtin_func<1>("eval", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<eval>", EVAL_MODE, true);
        FrameId frame = vm->top_frame();
        return vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
    });

    _vm->bind_builtin_func<1>("exec", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<exec>", EXEC_MODE, true);
        FrameId frame = vm->top_frame();
        vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, ArgsView args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit(CAST(int, args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", CPP_LAMBDA(vm->asRepr(args[0])));
    _vm->bind_builtin_func<1>("len", [](VM* vm, ArgsView args){
        return vm->call_method(args[0], __len__);
    });

    _vm->bind_builtin_func<1>("hash", [](VM* vm, ArgsView args){
        i64 value = vm->hash(args[0]);
        if(((value << 2) >> 2) != value) value >>= 2;
        return VAR(value);
    });

    _vm->bind_builtin_func<1>("chr", [](VM* vm, ArgsView args) {
        i64 i = CAST(i64, args[0]);
        if (i < 0 || i > 128) vm->ValueError("chr() arg not in range(128)");
        return VAR(std::string(1, (char)i));
    });

    _vm->bind_builtin_func<1>("ord", [](VM* vm, ArgsView args) {
        const Str& s = CAST(Str&, args[0]);
        if (s.length()!=1) vm->TypeError("ord() expected an ASCII character");
        return VAR((i64)(s[0]));
    });

    _vm->bind_builtin_func<2>("hasattr", [](VM* vm, ArgsView args) {
        return VAR(vm->getattr(args[0], CAST(Str&, args[1]), false) != nullptr);
    });

    _vm->bind_builtin_func<3>("setattr", [](VM* vm, ArgsView args) {
        vm->setattr(args[0], CAST(Str&, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_builtin_func<2>("getattr", [](VM* vm, ArgsView args) {
        const Str& name = CAST(Str&, args[1]);
        return vm->getattr(args[0], name);
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, ArgsView args) {
        std::stringstream ss;
        ss << std::hex << CAST(i64, args[0]);
        return VAR("0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("iter", [](VM* vm, ArgsView args) {
        return vm->asIter(args[0]);
    });

    _vm->bind_builtin_func<1>("next", [](VM* vm, ArgsView args) {
        return vm->PyIterNext(args[0]);
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, ArgsView args) {
        std::set<StrName> names;
        if(args[0]->is_attr_valid()){
            std::vector<StrName> keys = args[0]->attr().keys();
            names.insert(keys.begin(), keys.end());
        }
        const NameDict& t_attr = vm->_t(args[0])->attr();
        std::vector<StrName> keys = t_attr.keys();
        names.insert(keys.begin(), keys.end());
        List ret;
        for (StrName name : names) ret.push_back(VAR(name.sv()));
        return VAR(std::move(ret));
    });

    _vm->bind_method<0>("object", "__repr__", [](VM* vm, ArgsView args) {
        PyObject* self = args[0];
        if(is_tagged(self)) self = nullptr;
        std::stringstream ss;
        ss << "<" << OBJ_NAME(vm->_t(self)) << " object at " << std::hex << self << ">";
        return VAR(ss.str());
    });

    _vm->bind_method<1>("object", "__eq__", CPP_LAMBDA(VAR(args[0] == args[1])));
    _vm->bind_method<1>("object", "__ne__", CPP_LAMBDA(VAR(args[0] != args[1])));

    _vm->bind_static_method<1>("type", "__new__", CPP_LAMBDA(vm->_t(args[0])));
    _vm->bind_static_method<-1>("range", "__new__", [](VM* vm, ArgsView args) {
        Range r;
        switch (args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, but got " + std::to_string(args.size()));
        }
        return VAR(r);
    });

    _vm->bind_method<0>("range", "__iter__", CPP_LAMBDA(
        vm->PyIter(RangeIter(vm, args[0]))
    ));

    _vm->bind_method<0>("NoneType", "__repr__", CPP_LAMBDA(VAR("None")));
    _vm->bind_method<0>("NoneType", "__json__", CPP_LAMBDA(VAR("null")));

    _vm->bind_method<1>("int", "__truediv__", [](VM* vm, ArgsView args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, args[0]) / rhs);
    });

    _vm->bind_method<1>("float", "__truediv__", [](VM* vm, ArgsView args) {
        f64 rhs = vm->num_to_float(args[1]);
        if (rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(f64, args[0]) / rhs);
    });

    auto py_number_pow = [](VM* vm, ArgsView args) {
        if(is_both_int(args[0], args[1])){
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = _CAST(i64, args[1]);
            bool flag = false;
            if(rhs < 0) {flag = true; rhs = -rhs;}
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            if(flag) return VAR((f64)(1.0 / ret));
            return VAR(ret);
        }else{
            return VAR((f64)std::pow(vm->num_to_float(args[0]), vm->num_to_float(args[1])));
        }
    };

    _vm->bind_method<1>("int", "__pow__", py_number_pow);
    _vm->bind_method<1>("float", "__pow__", py_number_pow);

    /************ PyInt ************/
    _vm->bind_static_method<1>("int", "__new__", [](VM* vm, ArgsView args) {
        if (is_type(args[0], vm->tp_float)) return VAR((i64)CAST(f64, args[0]));
        if (is_type(args[0], vm->tp_int)) return args[0];
        if (is_type(args[0], vm->tp_bool)) return VAR(_CAST(bool, args[0]) ? 1 : 0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = CAST(Str&, args[0]);
            try{
                size_t parsed = 0;
                i64 val = Number::stoi(s.str(), &parsed, 10);
                if(parsed != s.length()) throw std::invalid_argument("<?>");
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for int(): " + s.escape());
            }
        }
        vm->TypeError("int() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<1>("int", "__floordiv__", [](VM* vm, ArgsView args) {
        i64 rhs = CAST(i64, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(CAST(i64, args[0]) / rhs);
    });

    _vm->bind_method<1>("int", "__mod__", [](VM* vm, ArgsView args) {
        i64 rhs = CAST(i64, args[1]);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(CAST(i64, args[0]) % rhs);
    });

    _vm->bind_method<0>("int", "__repr__", CPP_LAMBDA(VAR(std::to_string(CAST(i64, args[0])))));
    _vm->bind_method<0>("int", "__json__", CPP_LAMBDA(VAR(std::to_string(CAST(i64, args[0])))));

#define INT_BITWISE_OP(name,op) \
    _vm->bind_method<1>("int", #name, CPP_LAMBDA(VAR(CAST(i64, args[0]) op CAST(i64, args[1]))));

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    /************ PyFloat ************/
    _vm->bind_static_method<1>("float", "__new__", [](VM* vm, ArgsView args) {
        if (is_type(args[0], vm->tp_int)) return VAR((f64)CAST(i64, args[0]));
        if (is_type(args[0], vm->tp_float)) return args[0];
        if (is_type(args[0], vm->tp_bool)) return VAR(_CAST(bool, args[0]) ? 1.0 : 0.0);
        if (is_type(args[0], vm->tp_str)) {
            const Str& s = CAST(Str&, args[0]);
            if(s == "inf") return VAR(INFINITY);
            if(s == "-inf") return VAR(-INFINITY);
            try{
                f64 val = Number::stof(s.str());
                return VAR(val);
            }catch(std::invalid_argument&){
                vm->ValueError("invalid literal for float(): '" + s + "'");
            }
        }
        vm->TypeError("float() argument must be a int, float, bool or str");
        return vm->None;
    });

    _vm->bind_method<0>("float", "__repr__", [](VM* vm, ArgsView args) {
        f64 val = CAST(f64, args[0]);
        if(std::isinf(val) || std::isnan(val)) return VAR(std::to_string(val));
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<f64>::max_digits10-1-2) << val;
        std::string s = ss.str();
        if(std::all_of(s.begin()+1, s.end(), isdigit)) s += ".0";
        return VAR(s);
    });

    _vm->bind_method<0>("float", "__json__", [](VM* vm, ArgsView args) {
        f64 val = CAST(f64, args[0]);
        if(std::isinf(val) || std::isnan(val)) vm->ValueError("cannot jsonify 'nan' or 'inf'");
        return VAR(std::to_string(val));
    });

    /************ PyString ************/
    _vm->bind_static_method<1>("str", "__new__", CPP_LAMBDA(vm->asStr(args[0])));

    _vm->bind_method<1>("str", "__add__", [](VM* vm, ArgsView args) {
        const Str& lhs = _CAST(Str&, args[0]);
        const Str& rhs = CAST(Str&, args[1]);
        return VAR(lhs + rhs);
    });

    _vm->bind_method<0>("str", "__len__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.u8_length());
    });

    _vm->bind_method<1>("str", "__contains__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& other = CAST(Str&, args[1]);
        return VAR(self.index(other) != -1);
    });

    _vm->bind_method<0>("str", "__str__", CPP_LAMBDA(args[0]));
    _vm->bind_method<0>("str", "__iter__", CPP_LAMBDA(vm->PyIter(StringIter(vm, args[0]))));

    _vm->bind_method<0>("str", "__repr__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.escape());
    });

    _vm->bind_method<0>("str", "__json__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.escape(false));
    });

    _vm->bind_method<1>("str", "__eq__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(!is_type(args[1], vm->tp_str)) return VAR(false);
        return VAR(self == CAST(Str&, args[1]));
    });

    _vm->bind_method<1>("str", "__ne__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(!is_type(args[1], vm->tp_str)) return VAR(true);
        return VAR(self != CAST(Str&, args[1]));
    });

    _vm->bind_method<1>("str", "__getitem__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            const Slice& s = _CAST(Slice&, args[1]);
            int start, stop, step;
            vm->parse_int_slice(s, self.u8_length(), start, stop, step);
            return VAR(self.u8_slice(start, stop, step));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.u8_length());
        return VAR(self.u8_getitem(index));
    });

    _vm->bind_method<1>("str", "__gt__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& obj = CAST(Str&, args[1]);
        return VAR(self > obj);
    });

    _vm->bind_method<1>("str", "__lt__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& obj = CAST(Str&, args[1]);
        return VAR(self < obj);
    });

    _vm->bind_method<-1>("str", "replace", [](VM* vm, ArgsView args) {
        if(args.size() != 1+2 && args.size() != 1+3) vm->TypeError("replace() takes 2 or 3 arguments");
        const Str& self = _CAST(Str&, args[0]);
        const Str& old = CAST(Str&, args[1]);
        const Str& new_ = CAST(Str&, args[2]);
        int count = args.size()==1+3 ? CAST(int, args[3]) : -1;
        return VAR(self.replace(old, new_, count));
    });

    _vm->bind_method<1>("str", "index", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        int index = self.index(sub);
        if(index == -1) vm->ValueError("substring not found");
        return VAR(index);
    });

    _vm->bind_method<1>("str", "startswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& prefix = CAST(Str&, args[1]);
        return VAR(self.index(prefix) == 0);
    });

    _vm->bind_method<1>("str", "endswith", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& suffix = CAST(Str&, args[1]);
        int offset = self.length() - suffix.length();
        if(offset < 0) return vm->False;
        bool ok = memcmp(self.data+offset, suffix.data, suffix.length()) == 0;
        return VAR(ok);
    });

    _vm->bind_method<0>("str", "encode", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        std::vector<char> buffer(self.length());
        memcpy(buffer.data(), self.data, self.length());
        return VAR(Bytes(std::move(buffer)));
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        FastStrStream ss;
        PyObject* obj = vm->asList(args[1]);
        const List& list = CAST(List&, obj);
        for (int i = 0; i < list.size(); ++i) {
            if (i > 0) ss << self;
            ss << CAST(Str&, list[i]);
        }
        return VAR(ss.str());
    });

    /************ PyList ************/
    _vm->bind_method<1>("list", "append", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<1>("list", "extend", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = vm->asList(args[1]);
        const List& list = CAST(List&, obj);
        self.extend(list);
        return vm->None;
    });

    _vm->bind_method<0>("list", "reverse", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind_method<1>("list", "__mul__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        int n = CAST(int, args[1]);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });

    _vm->bind_method<2>("list", "insert", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(index, args[2]);
        return vm->None;
    });

    _vm->bind_method<0>("list", "clear", [](VM* vm, ArgsView args) {
        _CAST(List&, args[0]).clear();
        return vm->None;
    });

    _vm->bind_method<0>("list", "copy", CPP_LAMBDA(VAR(_CAST(List, args[0]))));

    _vm->bind_method<1>("list", "__add__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        const List& other = CAST(List&, args[1]);
        List new_list(self);    // copy construct
        new_list.extend(other);
        return VAR(std::move(new_list));
    });

    _vm->bind_method<0>("list", "__len__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        return VAR(self.size());
    });

    _vm->bind_method<0>("list", "__iter__", [](VM* vm, ArgsView args) {
        return vm->PyIter(ArrayIter<List>(vm, args[0]));
    });

    _vm->bind_method<1>("list", "__getitem__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            const Slice& s = _CAST(Slice&, args[1]);
            int start, stop, step;
            vm->parse_int_slice(s, self.size(), start, stop, step);
            List new_list;
            for(int i=start; step>0?i<stop:i>stop; i+=step) new_list.push_back(self[i]);
            return VAR(std::move(new_list));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<2>("list", "__setitem__", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        self[index] = args[2];
        return vm->None;
    });

    _vm->bind_method<1>("list", "__delitem__", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        self.erase(index);
        return vm->None;
    });

    /************ PyTuple ************/
    _vm->bind_static_method<1>("tuple", "__new__", [](VM* vm, ArgsView args) {
        List list = CAST(List, vm->asList(args[0]));
        return VAR(Tuple(std::move(list)));
    });

    _vm->bind_method<0>("tuple", "__iter__", [](VM* vm, ArgsView args) {
        return vm->PyIter(ArrayIter<Tuple>(vm, args[0]));
    });

    _vm->bind_method<1>("tuple", "__getitem__", [](VM* vm, ArgsView args) {
        const Tuple& self = _CAST(Tuple&, args[0]);

        if(is_type(args[1], vm->tp_slice)){
            const Slice& s = _CAST(Slice&, args[1]);
            int start, stop, step;
            vm->parse_int_slice(s, self.size(), start, stop, step);
            List new_list;
            for(int i=start; step>0?i<stop:i>stop; i+=step) new_list.push_back(self[i]);
            return VAR(Tuple(std::move(new_list)));
        }

        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        return self[index];
    });

    _vm->bind_method<0>("tuple", "__len__", [](VM* vm, ArgsView args) {
        const Tuple& self = _CAST(Tuple&, args[0]);
        return VAR(self.size());
    });

    /************ bool ************/
    _vm->bind_static_method<1>("bool", "__new__", CPP_LAMBDA(VAR(vm->asBool(args[0]))));

    _vm->bind_method<0>("bool", "__repr__", [](VM* vm, ArgsView args) {
        bool val = _CAST(bool, args[0]);
        return VAR(val ? "True" : "False");
    });

    _vm->bind_method<0>("bool", "__json__", [](VM* vm, ArgsView args) {
        bool val = _CAST(bool, args[0]);
        return VAR(val ? "true" : "false");
    });

    _vm->bind_method<1>("bool", "__xor__", [](VM* vm, ArgsView args) {
        bool self = _CAST(bool, args[0]);
        bool other = CAST(bool, args[1]);
        return VAR(self ^ other);
    });

    _vm->bind_method<0>("ellipsis", "__repr__", CPP_LAMBDA(VAR("Ellipsis")));

    /************ bytes ************/
    _vm->bind_static_method<1>("bytes", "__new__", [](VM* vm, ArgsView args){
        List& list = CAST(List&, args[0]);
        std::vector<char> buffer(list.size());
        for(int i=0; i<list.size(); i++){
            i64 b = CAST(i64, list[i]);
            if(b<0 || b>255) vm->ValueError("byte must be in range[0, 256)");
            buffer[i] = (char)b;
        }
        return VAR(Bytes(std::move(buffer)));
    });

    _vm->bind_method<1>("bytes", "__getitem__", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        int index = CAST(int, args[1]);
        index = vm->normalized_index(index, self.size());
        return VAR(self[index]);
    });

    _vm->bind_method<0>("bytes", "__repr__", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        std::stringstream ss;
        ss << "b'";
        for(int i=0; i<self.size(); i++){
            ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << self[i];
        }
        ss << "'";
        return VAR(ss.str());
    });

    _vm->bind_method<0>("bytes", "__len__", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        return VAR(self.size());
    });

    _vm->bind_method<0>("bytes", "decode", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        // TODO: check encoding is utf-8
        return VAR(Str(self.str()));
    });

    _vm->bind_method<1>("bytes", "__eq__", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        if(!is_type(args[1], vm->tp_bytes)) return VAR(false);
        const Bytes& other = CAST(Bytes&, args[1]);
        return VAR(self == other);
    });

    _vm->bind_method<1>("bytes", "__ne__", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        if(!is_type(args[1], vm->tp_bytes)) return VAR(true);
        const Bytes& other = CAST(Bytes&, args[1]);
        return VAR(self != other);
    });

    /************ slice ************/
    _vm->bind_static_method<3>("slice", "__new__", [](VM* vm, ArgsView args) {
        return VAR(Slice(args[0], args[1], args[2]));
    });

    _vm->bind_method<0>("slice", "__repr__", [](VM* vm, ArgsView args) {
        const Slice& self = _CAST(Slice&, args[0]);
        std::stringstream ss;
        ss << "slice(";
        ss << CAST(Str, vm->asRepr(self.start)) << ", ";
        ss << CAST(Str, vm->asRepr(self.stop)) << ", ";
        ss << CAST(Str, vm->asRepr(self.step)) << ")";
        return VAR(ss.str());
    });

    /************ MappingProxy ************/
    _vm->bind_method<0>("mappingproxy", "keys", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List keys;
        for(StrName name : self.attr().keys()) keys.push_back(VAR(name.sv()));
        return VAR(std::move(keys));
    });

    _vm->bind_method<0>("mappingproxy", "values", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List values;
        for(auto& item : self.attr().items()) values.push_back(item.second);
        return VAR(std::move(values));
    });

    _vm->bind_method<0>("mappingproxy", "items", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List items;
        for(auto& item : self.attr().items()){
            PyObject* t = VAR(Tuple({VAR(item.first.sv()), item.second}));
            items.push_back(std::move(t));
        }
        return VAR(std::move(items));
    });

    _vm->bind_method<0>("mappingproxy", "__len__", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        return VAR(self.attr().size());
    });

    _vm->bind_method<1>("mappingproxy", "__getitem__", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        StrName key = CAST(Str&, args[1]);
        PyObject* ret = self.attr().try_get(key);
        if(ret == nullptr) vm->AttributeError(key.sv());
        return ret;
    });

    _vm->bind_method<0>("mappingproxy", "__repr__", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        std::stringstream ss;
        ss << "mappingproxy({";
        bool first = true;
        for(auto& item : self.attr().items()){
            if(!first) ss << ", ";
            first = false;
            ss << item.first.escape() << ": " << CAST(Str, vm->asRepr(item.second));
        }
        ss << "})";
        return VAR(ss.str());
    });

    _vm->bind_method<1>("mappingproxy", "__contains__", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        StrName key = CAST(Str&, args[1]);
        return VAR(self.attr().contains(key));
    });
}

#ifdef _WIN32
#define __EXPORT __declspec(dllexport) inline
#elif __APPLE__
#define __EXPORT __attribute__((visibility("default"))) __attribute__((used)) inline
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define __EXPORT EMSCRIPTEN_KEEPALIVE inline
#else
#define __EXPORT inline
#endif

inline void add_module_time(VM* vm){
    PyObject* mod = vm->new_module("time");
    vm->bind_func<0>(mod, "time", [](VM* vm, ArgsView args) {
        auto now = std::chrono::high_resolution_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() / 1000000.0);
    });
}

inline void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object, {});
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        vm->_stdout(vm, CAST(Str&, args[0]));
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        vm->_stderr(vm, CAST(Str&, args[0]));
        return vm->None;
    });
}

inline void add_module_json(VM* vm){
    PyObject* mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        const Str& expr = CAST(Str&, args[0]);
        CodeObject_ code = vm->compile(expr, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", CPP_LAMBDA(vm->call_method(args[0], __json__)));
}

inline void add_module_math(VM* vm){
    PyObject* mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));

    vm->bind_func<1>(mod, "log", CPP_LAMBDA(VAR(std::log(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log10", CPP_LAMBDA(VAR(std::log10(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "log2", CPP_LAMBDA(VAR(std::log2(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sin", CPP_LAMBDA(VAR(std::sin(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "cos", CPP_LAMBDA(VAR(std::cos(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "tan", CPP_LAMBDA(VAR(std::tan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isnan", CPP_LAMBDA(VAR(std::isnan(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "isinf", CPP_LAMBDA(VAR(std::isinf(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "fabs", CPP_LAMBDA(VAR(std::fabs(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "floor", CPP_LAMBDA(VAR((i64)std::floor(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "ceil", CPP_LAMBDA(VAR((i64)std::ceil(vm->num_to_float(args[0])))));
    vm->bind_func<1>(mod, "sqrt", CPP_LAMBDA(VAR(std::sqrt(vm->num_to_float(args[0])))));
    vm->bind_func<2>(mod, "gcd", [](VM* vm, ArgsView args) {
        i64 a = CAST(i64, args[0]);
        i64 b = CAST(i64, args[1]);
        if(a < 0) a = -a;
        if(b < 0) b = -b;
        while(b != 0){
            i64 t = b;
            b = a % b;
            a = t;
        }
        return VAR(a);
    });
}

inline void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");
    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        PyObject* f = args[0];
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, args[0]).func;
        CodeObject_ code = CAST(Function&, f).decl->code;
        vm->_stdout(vm, vm->disassemble(code));
        return vm->None;
    });
}

struct ReMatch {
    PY_CLASS(ReMatch, re, Match)

    i64 start;
    i64 end;
    std::cmatch m;
    ReMatch(i64 start, i64 end, std::cmatch m) : start(start), end(end), m(m) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_method<-1>(type, "__init__", CPP_NOT_IMPLEMENTED());
        vm->bind_method<0>(type, "start", CPP_LAMBDA(VAR(_CAST(ReMatch&, args[0]).start)));
        vm->bind_method<0>(type, "end", CPP_LAMBDA(VAR(_CAST(ReMatch&, args[0]).end)));

        vm->bind_method<0>(type, "span", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            return VAR(Tuple({VAR(self.start), VAR(self.end)}));
        });

        vm->bind_method<1>(type, "group", [](VM* vm, ArgsView args) {
            auto& self = _CAST(ReMatch&, args[0]);
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.m.size());
            return VAR(self.m[index].str());
        });
    }
};

inline PyObject* _regex_search(const Str& pattern, const Str& string, bool from_start, VM* vm){
    std::regex re(pattern.begin(), pattern.end());
    std::cmatch m;
    if(std::regex_search(string.begin(), string.end(), m, re)){
        if(from_start && m.position() != 0) return vm->None;
        i64 start = string._byte_index_to_unicode(m.position());
        i64 end = string._byte_index_to_unicode(m.position() + m.length());
        return VAR_T(ReMatch, start, end, m);
    }
    return vm->None;
};

inline void add_module_re(VM* vm){
    PyObject* mod = vm->new_module("re");
    ReMatch::register_class(vm, mod);

    vm->bind_func<2>(mod, "match", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, true, vm);
    });

    vm->bind_func<2>(mod, "search", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        return _regex_search(pattern, string, false, vm);
    });

    vm->bind_func<3>(mod, "sub", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& repl = CAST(Str&, args[1]);
        const Str& string = CAST(Str&, args[2]);
        std::regex re(pattern.begin(), pattern.end());
        return VAR(std::regex_replace(string.str(), re, repl.str()));
    });

    vm->bind_func<2>(mod, "split", [](VM* vm, ArgsView args) {
        const Str& pattern = CAST(Str&, args[0]);
        const Str& string = CAST(Str&, args[1]);
        std::regex re(pattern.begin(), pattern.end());
        std::cregex_token_iterator it(string.begin(), string.end(), re, -1);
        std::cregex_token_iterator end;
        List vec;
        for(; it != end; ++it){
            vec.push_back(VAR(it->str()));
        }
        return VAR(vec);
    });
}

struct Random{
    PY_CLASS(Random, random, Random)
    std::mt19937 gen;

    Random(){
        gen.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<0>(type, "__new__", CPP_LAMBDA(VAR_T(Random)));

        vm->bind_method<1>(type, "seed", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            self.gen.seed(CAST(i64, args[1]));
            return vm->None;
        });

        vm->bind_method<2>(type, "randint", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            i64 a = CAST(i64, args[1]);
            i64 b = CAST(i64, args[2]);
            std::uniform_int_distribution<i64> dis(a, b);
            return VAR(dis(self.gen));
        });

        vm->bind_method<0>(type, "random", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            std::uniform_real_distribution<f64> dis(0.0, 1.0);
            return VAR(dis(self.gen));
        });

        vm->bind_method<2>(type, "uniform", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            f64 a = CAST(f64, args[1]);
            f64 b = CAST(f64, args[2]);
            std::uniform_real_distribution<f64> dis(a, b);
            return VAR(dis(self.gen));
        });
    }
};

inline void add_module_random(VM* vm){
    PyObject* mod = vm->new_module("random");
    Random::register_class(vm, mod);
    CodeObject_ code = vm->compile(kPythonLibs["random"], "random.py", EXEC_MODE);
    vm->_exec(code, mod);
}

inline void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", CPP_LAMBDA(VAR(vm->heap.collect())));
}

inline void VM::post_init(){
    init_builtins(this);
#if !DEBUG_NO_BUILTIN_MODULES
    add_module_sys(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_re(this);
    add_module_dis(this);
    add_module_c(this);
    add_module_gc(this);
    add_module_random(this);
    add_module_base64(this);

    for(const char* name: {"this", "functools", "collections", "heapq", "bisect"}){
        _lazy_modules[name] = kPythonLibs[name];
    }

    CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
    this->_exec(code, this->builtins);
    code = compile(kPythonLibs["_dict"], "<dict>", EXEC_MODE);
    this->_exec(code, this->builtins);
    code = compile(kPythonLibs["_set"], "<set>", EXEC_MODE);
    this->_exec(code, this->builtins);

    // property is defined in builtins.py so we need to add it after builtins is loaded
    _t(tp_object)->attr().set(__class__, property(CPP_LAMBDA(vm->_t(args[0]))));
    _t(tp_type)->attr().set(__base__, property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[OBJ_GET(Type, args[0])];
        return info.base.index == -1 ? vm->None : vm->_all_types[info.base].obj;
    }));
    _t(tp_type)->attr().set(__name__, property([](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[OBJ_GET(Type, args[0])];
        return VAR(info.name);
    }));

    _t(tp_bound_method)->attr().set("__self__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).self;
    }));
    _t(tp_bound_method)->attr().set("__func__", property([](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).func;
    }));

    vm->bind_method<1>(_t(tp_bound_method), "__eq__", [](VM* vm, ArgsView args){
        if(!is_non_tagged_type(args[1], vm->tp_bound_method)) return vm->False;
        bool ok = _CAST(BoundMethod&, args[0]) == _CAST(BoundMethod&, args[1]);
        return VAR(ok);
    });

    vm->bind_method<1>(_t(tp_bound_method), "__ne__", [](VM* vm, ArgsView args){
        if(!is_non_tagged_type(args[1], vm->tp_bound_method)) return vm->True;
        bool ok = _CAST(BoundMethod&, args[0]) != _CAST(BoundMethod&, args[1]);
        return VAR(ok);
    });

    _t(tp_slice)->attr().set("start", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).start;
    }));
    _t(tp_slice)->attr().set("stop", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).stop;
    }));
    _t(tp_slice)->attr().set("step", property([](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).step;
    }));
    _t(tp_object)->attr().set("__dict__", property([](VM* vm, ArgsView args){
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()){
            vm->AttributeError("__dict__");
        }
        return VAR(MappingProxy(args[0]));
    }));

    if(enable_os){
        add_module_io(this);
        add_module_os(this);
        add_module_requests(this);
    }
#endif
}

}   // namespace pkpy

/*************************GLOBAL NAMESPACE*************************/
static std::map<void*, void(*)(void*)> _pk_deleter_map;

extern "C" {
    __EXPORT
    void pkpy_delete(void* p){
        auto it = _pk_deleter_map.find(p);
        if(it != _pk_deleter_map.end()){
            it->second(p);
        }else{
            free(p);
        }
    }

    __EXPORT
    void pkpy_vm_exec(pkpy::VM* vm, const char* source){
        vm->exec(source, "main.py", pkpy::EXEC_MODE);
    }

    __EXPORT
    char* pkpy_vm_get_global(pkpy::VM* vm, const char* name){
        pkpy::PyObject* val = vm->_main->attr().try_get(name);
        if(val == nullptr) return nullptr;
        try{
            pkpy::Str repr = pkpy::CAST(pkpy::Str&, vm->asRepr(val));
            return repr.c_str_dup();
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    char* pkpy_vm_eval(pkpy::VM* vm, const char* source){
        pkpy::PyObject* ret = vm->exec(source, "<eval>", pkpy::EVAL_MODE);
        if(ret == nullptr) return nullptr;
        try{
            pkpy::Str repr = pkpy::CAST(pkpy::Str&, vm->asRepr(ret));
            return repr.c_str_dup();
        }catch(...){
            return nullptr;
        }
    }

    __EXPORT
    pkpy::REPL* pkpy_new_repl(pkpy::VM* vm){
        pkpy::REPL* p = new pkpy::REPL(vm);
        _pk_deleter_map[p] = [](void* p){ delete (pkpy::REPL*)p; };
        return p;
    }

    __EXPORT
    bool pkpy_repl_input(pkpy::REPL* r, const char* line){
        return r->input(line);
    }

    __EXPORT
    void pkpy_vm_add_module(pkpy::VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    __EXPORT
    pkpy::VM* pkpy_new_vm(bool enable_os=true){
        pkpy::VM* p = new pkpy::VM(enable_os);
        _pk_deleter_map[p] = [](void* p){ delete (pkpy::VM*)p; };
        return p;
    }
}