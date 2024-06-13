#include "pocketpy/pocketpy.hpp"

#include "pocketpy/common/_generated.hpp"

#include "pocketpy/modules/array2d.hpp"
#include "pocketpy/modules/base64.hpp"
#include "pocketpy/modules/csv.hpp"
#include "pocketpy/modules/dataclasses.hpp"
#include "pocketpy/modules/easing.hpp"
#include "pocketpy/modules/io.hpp"
#include "pocketpy/modules/linalg.hpp"
#include "pocketpy/modules/random.hpp"
#include "pocketpy/modules/modules.hpp"

#include <iostream>
#include <algorithm>

namespace pkpy {

#ifdef PK_USE_CJSON
void add_module_cjson(VM* vm);
#endif

template <typename T>
PyVar PyArrayGetItem(VM* vm, PyVar _0, PyVar _1) {
    static_assert(std::is_same_v<T, List> || std::is_same_v<T, Tuple>);
    const T& self = _CAST(T&, _0);
    if(is_int(_1)) {
        i64 index = _1.as<i64>();
        index = vm->normalized_index(index, self.size());
        return self[index];
    }
    if(is_type(_1, vm->tp_slice)) {
        const Slice& s = _CAST(Slice&, _1);
        int start, stop, step;
        vm->parse_int_slice(s, self.size(), start, stop, step);
        List new_list;
        PK_SLICE_LOOP(i, start, stop, step) new_list.push_back(self[i]);

        if constexpr(std::is_same_v<T, List>)
            return VAR(std::move(new_list));
        else
            return VAR(new_list.to_tuple());
    }
    vm->TypeError("indices must be integers or slices");
}

void __init_builtins(VM* _vm) {
#define BIND_NUM_ARITH_OPT(name, op)                                                                                   \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyVar lhs, PyVar rhs) {                                                     \
        if(is_int(rhs)) return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                                \
        if(is_float(rhs)) return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));                                              \
        return vm->NotImplemented;                                                                                     \
    });                                                                                                                \
    _vm->bind##name(VM::tp_float, [](VM* vm, PyVar lhs, PyVar rhs) {                                                   \
        if(is_int(rhs)) return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));                                                \
        if(is_float(rhs)) return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));                                              \
        return vm->NotImplemented;                                                                                     \
    });

    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

#undef BIND_NUM_ARITH_OPT

#define BIND_NUM_LOGICAL_OPT(name, op)                                                                                 \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyVar lhs, PyVar rhs) {                                                     \
        if(is_int(rhs)) return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                                \
        if(is_float(rhs)) return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));                                              \
        return vm->NotImplemented;                                                                                     \
    });                                                                                                                \
    _vm->bind##name(VM::tp_float, [](VM* vm, PyVar lhs, PyVar rhs) {                                                   \
        if(is_int(rhs)) return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));                                                \
        if(is_float(rhs)) return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));                                              \
        return vm->NotImplemented;                                                                                     \
    });

    BIND_NUM_LOGICAL_OPT(__eq__, ==)
    BIND_NUM_LOGICAL_OPT(__lt__, <)
    BIND_NUM_LOGICAL_OPT(__le__, <=)
    BIND_NUM_LOGICAL_OPT(__gt__, >)
    BIND_NUM_LOGICAL_OPT(__ge__, >=)

#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    // builtin functions
    _vm->bind_func(_vm->builtins, "breakpoint", 0, [](VM* vm, ArgsView args) {
#if PK_ENABLE_PROFILER
        vm->_next_breakpoint = NextBreakpoint(vm->callstack.size(), vm->callstack.top().curr_lineno(), false);
#endif
        return vm->None;
    });

    _vm->bind_func(_vm->builtins, "super", -1, [](VM* vm, ArgsView args) {
        PyObject* class_arg = nullptr;
        PyVar self_arg = nullptr;
        if(args.size() == 2) {
            class_arg = args[0].get();
            self_arg = args[1];
        } else if(args.size() == 0) {
            Frame* frame = &vm->callstack.top();
            if(frame->_callable != nullptr) {
                class_arg = frame->_callable->as<Function>()._class;
                if(frame->_locals.size() > 0) self_arg = frame->_locals[0];
            }
            if(class_arg == nullptr || self_arg == nullptr) {
                vm->TypeError("super(): unable to determine the class context, use super(class, self) instead");
            }
        } else {
            vm->TypeError("super() takes 0 or 2 arguments");
        }
        vm->check_type(class_arg, vm->tp_type);
        Type type = class_arg->as<Type>();
        if(!vm->isinstance(self_arg, type)) {
            StrName _0 = _type_name(vm, vm->_tp(self_arg));
            StrName _1 = _type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        return vm->new_object<Super>(vm->tp_super, self_arg, vm->_all_types[type].base);
    });

    _vm->bind_func(_vm->builtins, "staticmethod", 1, [](VM* vm, ArgsView args) {
        PyVar func = args[0];
        vm->check_type(func, vm->tp_function);
        return vm->new_object<StaticMethod>(vm->tp_staticmethod, args[0]);
    });

    _vm->bind_func(_vm->builtins, "classmethod", 1, [](VM* vm, ArgsView args) {
        PyVar func = args[0];
        vm->check_type(func, vm->tp_function);
        return vm->new_object<ClassMethod>(vm->tp_classmethod, args[0]);
    });

    _vm->bind_func(_vm->builtins, "isinstance", 2, [](VM* vm, ArgsView args) {
        if(is_type(args[1], vm->tp_tuple)) {
            Tuple& types = _CAST(Tuple&, args[1]);
            for(PyVar type: types) {
                vm->check_type(type, vm->tp_type);
                if(vm->isinstance(args[0], type->as<Type>())) return vm->True;
            }
            return vm->False;
        }
        vm->check_type(args[1], vm->tp_type);
        Type type = PK_OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_func(_vm->builtins, "issubclass", 2, [](VM* vm, ArgsView args) {
        vm->check_type(args[0], vm->tp_type);
        vm->check_type(args[1], vm->tp_type);
        return VAR(vm->issubclass(PK_OBJ_GET(Type, args[0]), PK_OBJ_GET(Type, args[1])));
    });

    _vm->bind_func(_vm->builtins, "globals", 0, [](VM* vm, ArgsView args) {
        PyObject* mod = vm->callstack.top()._module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind(_vm->builtins, "round(x, ndigits=None)", [](VM* vm, ArgsView args) {
        if(is_int(args[0])) return args[0];
        f64 x = CAST(f64, args[0]);
        f64 offset = x >= 0 ? 0.5 : -0.5;
        if(is_none(args[1])) return VAR((i64)(x + offset));
        int ndigits = CAST(int, args[1]);
        if(ndigits < 0) vm->ValueError("ndigits should be non-negative");
        // ndigits > 0
        return VAR((i64)(x * std::pow(10, ndigits) + offset) / std::pow(10, ndigits));
    });

    _vm->bind_func(_vm->builtins, "abs", 1, [](VM* vm, ArgsView args) {
        if(is_int(args[0])) return VAR(std::abs(_CAST(i64, args[0])));
        if(is_float(args[0])) return VAR(std::abs(_CAST(f64, args[0])));
        vm->TypeError("bad operand type for abs()");
        return vm->None;
    });

    _vm->bind(_vm->builtins, "max(*args, key=None)", [](VM* vm, ArgsView args) {
        return vm->__minmax_reduce(&VM::py_gt, args[0], args[1]);
    });

    _vm->bind(_vm->builtins, "min(*args, key=None)", [](VM* vm, ArgsView args) {
        return vm->__minmax_reduce(&VM::py_lt, args[0], args[1]);
    });

    _vm->bind_func(_vm->builtins, "id", 1, [](VM* vm, ArgsView args) {
        PyVar obj = args[0];
        if(is_tagged(obj)) return vm->None;
        return VAR(reinterpret_cast<i64>(obj.get()));
    });

    _vm->bind_func(_vm->builtins, "callable", 1, [](VM* vm, ArgsView args) {
        return VAR(vm->py_callable(args[0]));
    });

    _vm->bind_func(_vm->builtins, "__import__", 1, [](VM* vm, ArgsView args) -> PyVar {
        const Str& name = CAST(Str&, args[0]);
        return vm->py_import(name);
    });

    _vm->bind_func(_vm->builtins, "divmod", 2, [](VM* vm, ArgsView args) {
        if(is_int(args[0])) {
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = CAST(i64, args[1]);
            if(rhs == 0) vm->ZeroDivisionError();
            auto res = std::div(lhs, rhs);
            return VAR(Tuple(VAR(res.quot), VAR(res.rem)));
        } else {
            return vm->call_method(args[0], __divmod__, args[1]);
        }
    });

    // we use `_0`, `_1` and `_2` here to disable keyword arguments (but with default values)
    _vm->bind(_vm->builtins, "eval(_0, _1=None, _2=None)", [](VM* vm, ArgsView args) {
        return vm->py_eval(CAST(Str&, args[0]), args[1], args[2]);
    });

    _vm->bind(_vm->builtins, "exec(_0, _1=None, _2=None)", [](VM* vm, ArgsView args) {
        vm->py_exec(CAST(Str&, args[0]), args[1], args[2]);
        return vm->None;
    });

    _vm->bind(_vm->builtins, "compile(source: str, filename: str, mode: str) -> str", [](VM* vm, ArgsView args) {
        const Str& source = CAST(Str&, args[0]);
        const Str& filename = CAST(Str&, args[1]);
        const Str& mode = CAST(Str&, args[2]);
        if(mode == "exec") {
            return VAR(vm->precompile(source, filename, EXEC_MODE));
        } else if(mode == "eval") {
            return VAR(vm->precompile(source, filename, EVAL_MODE));
        } else if(mode == "single") {
            return VAR(vm->precompile(source, filename, CELL_MODE));
        } else {
            vm->ValueError("compile() mode must be 'exec', 'eval' or 'single'");
            return vm->None;
        }
    });

    _vm->bind(_vm->builtins, "exit(code=0)", [](VM* vm, ArgsView args) {
        std::exit(CAST(int, args[0]));
        return vm->None;
    });

    _vm->bind_func(_vm->builtins, "repr", 1, [](VM* vm, ArgsView args) {
        return VAR(vm->py_repr(args[0]));
    });

    _vm->bind_func(_vm->builtins, "len", 1, [](VM* vm, ArgsView args) {
        const PyTypeInfo* ti = vm->_tp_info(args[0]);
        if(ti->m__len__) return VAR(ti->m__len__(vm, args[0]));
        return vm->call_method(args[0], __len__);
    });

    _vm->bind_func(_vm->builtins, "hash", 1, [](VM* vm, ArgsView args) {
        i64 value = vm->py_hash(args[0]);
        return VAR(value);
    });

    _vm->bind_func(_vm->builtins, "chr", 1, [](VM* vm, ArgsView args) {
        i64 i = CAST(i64, args[0]);
        if(i < 0 || i >= 128) vm->ValueError("chr() arg not in [0, 128)");
        return VAR(std::string(1, (char)i));
    });

    _vm->bind_func(_vm->builtins, "ord", 1, [](VM* vm, ArgsView args) {
        const Str& s = CAST(Str&, args[0]);
        if(s.length() != 1) vm->TypeError("ord() expected an ASCII character");
        return VAR((i64)(s[0]));
    });

    _vm->bind_func(_vm->builtins, "hasattr", 2, [](VM* vm, ArgsView args) {
        return VAR(vm->getattr(args[0], CAST(Str&, args[1]), false) != nullptr);
    });

    _vm->bind_func(_vm->builtins, "setattr", 3, [](VM* vm, ArgsView args) {
        vm->setattr(args[0], CAST(Str&, args[1]), args[2]);
        return vm->None;
    });

    _vm->bind_func(_vm->builtins, "getattr", -1, [](VM* vm, ArgsView args) {
        if(args.size() != 2 && args.size() != 3) vm->TypeError("getattr() takes 2 or 3 arguments");
        StrName name = CAST(Str&, args[1]);
        PyVar val = vm->getattr(args[0], name, false);
        if(val == nullptr) {
            if(args.size() == 2) vm->AttributeError(args[0], name);
            return args[2];
        }
        return val;
    });

    _vm->bind_func(_vm->builtins, "delattr", 2, [](VM* vm, ArgsView args) {
        vm->delattr(args[0], CAST(Str&, args[1]));
        return vm->None;
    });

    _vm->bind_func(_vm->builtins, "hex", 1, [](VM* vm, ArgsView args) {
        SStream ss;
        i64 val = CAST(i64, args[0]);
        if(val == 0) {
            ss << "0x0";
            return VAR(ss.str());
        }
        if(val < 0) {
            ss << "-";
            val = -val;
        }
        ss << "0x";
        bool non_zero = true;
        for(int i = 56; i >= 0; i -= 8) {
            unsigned char cpnt = (val >> i) & 0xff;
            ss.write_hex(cpnt, non_zero);
            if(cpnt != 0) non_zero = false;
        }
        return VAR(ss.str());
    });

    _vm->bind_func(_vm->builtins, "iter", 1, [](VM* vm, ArgsView args) {
        return vm->py_iter(args[0]);
    });

    _vm->bind_func(_vm->builtins, "next", 1, [](VM* vm, ArgsView args) {
        PyVar retval = vm->py_next(args[0]);
        if(retval == vm->StopIteration) vm->_error(vm->call(vm->StopIteration));
        return retval;
    });

    _vm->bind_func(_vm->builtins, "bin", 1, [](VM* vm, ArgsView args) {
        SStream ss;
        i64 x = CAST(i64, args[0]);
        if(x < 0) {
            ss << "-";
            x = -x;
        }
        ss << "0b";
        std::string bits;
        while(x) {
            bits += (x & 1) ? '1' : '0';
            x >>= 1;
        }
        std::reverse(bits.begin(), bits.end());
        if(bits.empty()) bits = "0";
        ss << bits;
        return VAR(ss.str());
    });

    _vm->bind_func(_vm->builtins, "dir", 1, [](VM* vm, ArgsView args) {
        vector<StrName> names;
        if(!is_tagged(args[0]) && args[0]->is_attr_valid()) {
            auto keys = args[0]->attr().keys();
            names.extend(keys.begin(), keys.end());
        }
        const NameDict& t_attr = vm->_t(args[0])->attr();
        auto keys = t_attr.keys();
        names.extend(keys.begin(), keys.end());
        std::sort(names.begin(), names.end());
        List ret;
        for(int i = 0; i < names.size(); i++) {
            // remove duplicates
            if(i > 0 && names[i] == names[i - 1]) continue;
            ret.push_back(VAR(names[i].sv()));
        }
        return VAR(std::move(ret));
    });

    // tp_object
    _vm->bind__repr__(VM::tp_object, [](VM* vm, PyVar obj) -> Str {
        assert(!is_tagged(obj));
        SStream ss;
        ss << "<" << _type_name(vm, vm->_tp(obj)) << " object at ";
        ss.write_ptr(obj.get());
        ss << ">";
        return ss.str();
    });

    _vm->bind__eq__(VM::tp_object, [](VM* vm, PyVar _0, PyVar _1) {
        return VAR(_0 == _1);
    });

    _vm->__cached_object_new = _vm->bind_func(VM::tp_object, __new__, 1, [](VM* vm, ArgsView args) {
        vm->check_type(args[0], vm->tp_type);
        Type t = PK_OBJ_GET(Type, args[0]);
        return vm->new_object<DummyInstance>(t);
    });

    // tp_type
    _vm->bind_func(VM::tp_type, __new__, 2, PK_LAMBDA(vm->_t(args[1])));

    // tp_range
    _vm->bind_func(VM::tp_range, __new__, -1, [](VM* vm, ArgsView args) {
        args._begin += 1;  // skip cls
        Range r;
        switch(args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2:
                r.start = CAST(i64, args[0]);
                r.stop = CAST(i64, args[1]);
                break;
            case 3:
                r.start = CAST(i64, args[0]);
                r.stop = CAST(i64, args[1]);
                r.step = CAST(i64, args[2]);
                break;
            default: vm->TypeError("expected 1-3 arguments, got " + std::to_string(args.size()));
        }
        if(r.step == 0) vm->ValueError("range() arg 3 must not be zero");
        return VAR(r);
    });

    _vm->bind__iter__(VM::tp_range, [](VM* vm, PyVar _0) {
        const Range& r = PK_OBJ_GET(Range, _0);
        if(r.step > 0) {
            return vm->new_user_object<RangeIter>(r);
        } else {
            return vm->new_user_object<RangeIterR>(r);
        }
    });

    // tp_nonetype
    _vm->bind__repr__(_vm->_tp(_vm->None), [](VM* vm, PyVar _0) -> Str {
        return "None";
    });

    // tp_float / tp_float
    _vm->bind__truediv__(VM::tp_float, [](VM* vm, PyVar _0, PyVar _1) {
        f64 value = CAST_F(_1);
        return VAR(_CAST(f64, _0) / value);
    });
    _vm->bind__truediv__(VM::tp_int, [](VM* vm, PyVar _0, PyVar _1) {
        f64 value = CAST_F(_1);
        return VAR(_CAST(i64, _0) / value);
    });

    auto py_number_pow = [](VM* vm, PyVar _0, PyVar _1) {
        if(is_int(_0) && is_int(_1)) {
            i64 lhs = _CAST(i64, _0);
            i64 rhs = _CAST(i64, _1);
            if(rhs < 0) {
                if(lhs == 0) vm->ZeroDivisionError("0.0 cannot be raised to a negative power");
                return VAR((f64)std::pow(lhs, rhs));
            }
            i64 ret = 1;
            while(rhs) {
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            return VAR(ret);
        } else {
            return VAR((f64)std::pow(CAST_F(_0), CAST_F(_1)));
        }
    };

    _vm->bind__pow__(VM::tp_int, py_number_pow);
    _vm->bind__pow__(VM::tp_float, py_number_pow);

    _vm->bind_func(VM::tp_int, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return VAR(0);
        // 1 arg
        if(args.size() == 1 + 1) {
            switch(vm->_tp(args[1])) {
                case VM::tp_float: return VAR((i64)_CAST(f64, args[1]));
                case VM::tp_int: return args[1];
                case VM::tp_bool: return VAR(args[1] == vm->True ? 1 : 0);
                case VM::tp_str: break;
                default: vm->TypeError("invalid arguments for int()");
            }
        }
        // 2+ args -> error
        if(args.size() > 1 + 2) vm->TypeError("int() takes at most 2 arguments");
        // 1 or 2 args with str
        int base = 10;
        if(args.size() == 1 + 2) base = CAST(i64, args[2]);
        const Str& s = CAST(Str&, args[1]);
        std::string_view sv = s.sv();
        bool negative = false;
        if(!sv.empty() && (sv[0] == '+' || sv[0] == '-')) {
            negative = sv[0] == '-';
            sv.remove_prefix(1);
        }
        i64 val;
        if(parse_uint(sv, &val, base) != IntParsingResult::Success) {
            vm->ValueError(_S("invalid literal for int() with base ", base, ": ", s.escape()));
        }
        if(negative) val = -val;
        return VAR(val);
    });

    _vm->bind__floordiv__(VM::tp_int, [](VM* vm, PyVar _0, PyVar _1) {
        i64 rhs = CAST(i64, _1);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, _0) / rhs);
    });

    _vm->bind__mod__(VM::tp_int, [](VM* vm, PyVar _0, PyVar _1) {
        i64 rhs = CAST(i64, _1);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, _0) % rhs);
    });

    _vm->bind_func(VM::tp_int, "bit_length", 1, [](VM* vm, ArgsView args) {
        i64 x = _CAST(i64, args[0]);
        if(x < 0) x = -x;
        int bits = 0;
        while(x) {
            x >>= 1;
            bits++;
        }
        return VAR(bits);
    });

    _vm->bind__repr__(VM::tp_int, [](VM* vm, PyVar obj) -> Str {
        return std::to_string(_CAST(i64, obj));
    });
    _vm->bind__neg__(VM::tp_int, [](VM* vm, PyVar obj) {
        return VAR(-_CAST(i64, obj));
    });
    _vm->bind__hash__(VM::tp_int, [](VM* vm, PyVar obj) {
        return _CAST(i64, obj);
    });
    _vm->bind__invert__(VM::tp_int, [](VM* vm, PyVar obj) {
        return VAR(~_CAST(i64, obj));
    });

#define INT_BITWISE_OP(name, op)                                                                                       \
    _vm->bind##name(VM::tp_int, [](VM* vm, PyVar lhs, PyVar rhs) {                                                     \
        return VAR(_CAST(i64, lhs) op CAST(i64, rhs));                                                                 \
    });

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    _vm->bind_func(VM::tp_float, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return VAR(0.0);
        if(args.size() > 1 + 1) vm->TypeError("float() takes at most 1 argument");
        // 1 arg
        switch(vm->_tp(args[1])) {
            case VM::tp_int: return VAR((f64)CAST(i64, args[1]));
            case VM::tp_float: return args[1];
            case VM::tp_bool: return VAR(args[1] == vm->True ? 1.0 : 0.0);
            case VM::tp_str: break;
            default: vm->TypeError("invalid arguments for float()");
        }
        // str to float
        const Str& s = PK_OBJ_GET(Str, args[1]);
        if(s == "inf") return VAR(INFINITY);
        if(s == "-inf") return VAR(-INFINITY);

        double float_out;
        char* p_end;
        try {
            float_out = std::strtod(s.c_str(), &p_end);
            if(p_end != s.end()) throw 1;
        } catch(...) { vm->ValueError("invalid literal for float(): " + s.escape()); }
        return VAR(float_out);
    });

    _vm->bind__hash__(VM::tp_float, [](VM* vm, PyVar _0) {
        f64 val = _CAST(f64, _0);
        return (i64)std::hash<f64>()(val);
    });

    _vm->bind__neg__(VM::tp_float, [](VM* vm, PyVar _0) {
        return VAR(-_CAST(f64, _0));
    });

    _vm->bind__repr__(VM::tp_float, [](VM* vm, PyVar _0) -> Str {
        f64 val = _CAST(f64, _0);
        SStream ss;
        ss << val;
        return ss.str();
    });

    // tp_str
    _vm->bind_func(VM::tp_str, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1) return VAR(Str());
        if(args.size() > 2) vm->TypeError("str() takes at most 1 argument");
        return VAR(vm->py_str(args[1]));
    });

    _vm->bind__hash__(VM::tp_str, [](VM* vm, PyVar _0) {
        return (i64)_CAST(Str&, _0).hash();
    });

    _vm->bind__add__(VM::tp_str, [](VM* vm, PyVar _0, PyVar _1) {
        return VAR(_CAST(Str&, _0) + CAST(Str&, _1));
    });
    _vm->bind__len__(VM::tp_str, [](VM* vm, PyVar _0) {
        return (i64)_CAST(Str&, _0).u8_length();
    });
    _vm->bind__mul__(VM::tp_str, [](VM* vm, PyVar _0, PyVar _1) {
        const Str& self = _CAST(Str&, _0);
        i64 n = CAST(i64, _1);
        SStream ss;
        for(i64 i = 0; i < n; i++)
            ss << self.sv();
        return VAR(ss.str());
    });
    _vm->bind_func(VM::tp_str, "__rmul__", 2, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        i64 n = CAST(i64, args[1]);
        SStream ss;
        for(i64 i = 0; i < n; i++)
            ss << self.sv();
        return VAR(ss.str());
    });
    _vm->bind__contains__(VM::tp_str, [](VM* vm, PyVar _0, PyVar _1) {
        const Str& self = _CAST(Str&, _0);
        return VAR(self.index(CAST(Str&, _1)) != -1);
    });

    _vm->bind_func(VM::tp_str, __str__, 1, [](VM* vm, ArgsView args) {
        return args[0];
    });
    _vm->bind__iter__(VM::tp_str, [](VM* vm, PyVar _0) {
        return vm->new_user_object<StringIter>(_0);
    });
    _vm->bind__repr__(VM::tp_str, [](VM* vm, PyVar _0) -> Str {
        const Str& self = _CAST(Str&, _0);
        return self.escape();
    });

#define BIND_CMP_STR(name, op)                                                                                         \
    _vm->bind##name(VM::tp_str, [](VM* vm, PyVar lhs, PyVar rhs) {                                                     \
        if(!is_type(rhs, vm->tp_str)) return vm->NotImplemented;                                                       \
        return VAR(_CAST(Str&, lhs) op _CAST(Str&, rhs));                                                              \
    });

    BIND_CMP_STR(__eq__, ==)
    BIND_CMP_STR(__lt__, <)
    BIND_CMP_STR(__le__, <=)
    BIND_CMP_STR(__gt__, >)
    BIND_CMP_STR(__ge__, >=)
#undef BIND_CMP_STR

    _vm->bind__getitem__(VM::tp_str, [](VM* vm, PyVar _0, PyVar _1) {
        const Str& self = PK_OBJ_GET(Str, _0);
        if(is_type(_1, vm->tp_slice)) {
            const Slice& s = _CAST(Slice&, _1);
            int start, stop, step;
            vm->parse_int_slice(s, self.u8_length(), start, stop, step);
            return VAR(self.u8_slice(start, stop, step));
        }
        i64 i = CAST(i64, _1);
        i = vm->normalized_index(i, self.u8_length());
        return VAR(self.u8_getitem(i));
    });

    _vm->bind(_vm->_t(VM::tp_str), "replace(self, old, new)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& old = CAST(Str&, args[1]);
        if(old.empty()) vm->ValueError("empty substring");
        const Str& new_ = CAST(Str&, args[2]);
        return VAR(self.replace(old, new_));
    });

    _vm->bind(_vm->_t(VM::tp_str), "split(self, sep=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sep = CAST(Str&, args[1]);
        if(sep.empty()) vm->ValueError("empty separator");
        vector<std::string_view> parts;
        if(sep.size == 1) {
            parts = self.split(sep[0]);
        } else {
            parts = self.split(sep);
        }
        List ret(parts.size());
        for(int i = 0; i < parts.size(); i++)
            ret[i] = VAR(Str(parts[i]));
        return VAR(std::move(ret));
    });

    _vm->bind(_vm->_t(VM::tp_str), "splitlines(self)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        vector<std::string_view> parts = self.split('\n');
        List ret(parts.size());
        for(int i = 0; i < parts.size(); i++)
            ret[i] = VAR(Str(parts[i]));
        return VAR(std::move(ret));
    });

    _vm->bind(_vm->_t(VM::tp_str), "count(self, s: str)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& s = CAST(Str&, args[1]);
        return VAR(self.count(s));
    });

    _vm->bind(_vm->_t(VM::tp_str), "index(self, value, __start=0)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& value = CAST(Str&, args[1]);
        int start = CAST(int, args[2]);
        if(start < 0) vm->ValueError("argument 'start' can't be negative");
        int index = self.index(value, start);
        if(index < 0) vm->ValueError("substring not found");
        return VAR(index);
    });

    _vm->bind(_vm->_t(VM::tp_str), "find(self, value, __start=0)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& value = CAST(Str&, args[1]);
        int start = CAST(int, args[2]);
        if(start < 0) vm->ValueError("argument 'start' can't be negative");
        return VAR(self.index(value, start));
    });

    _vm->bind_func(VM::tp_str, "startswith", 2, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& prefix = CAST(Str&, args[1]);
        return VAR(self.index(prefix) == 0);
    });

    _vm->bind_func(VM::tp_str, "endswith", 2, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& suffix = CAST(Str&, args[1]);
        int offset = self.length() - suffix.length();
        if(offset < 0) return vm->False;
        bool ok = memcmp(self.c_str() + offset, suffix.c_str(), suffix.length()) == 0;
        return VAR(ok);
    });

    _vm->bind_func(VM::tp_str, "encode", 1, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        Bytes retval(self.length());
        std::memcpy(retval.data(), self.c_str(), self.length());
        return VAR(std::move(retval));
    });

    _vm->bind_func(VM::tp_str, "join", 2, [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        const Str& self = _CAST(Str&, args[0]);
        SStream ss;
        PyVar it = vm->py_iter(args[1]);  // strong ref
        const PyTypeInfo* info = vm->_tp_info(args[1]);
        PyVar obj = vm->_py_next(info, it);
        while(obj != vm->StopIteration) {
            if(!ss.empty()) ss << self;
            ss << CAST(Str&, obj);
            obj = vm->_py_next(info, it);
        }
        return VAR(ss.str());
    });

    _vm->bind_func(VM::tp_str, "lower", 1, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.lower());
    });

    _vm->bind_func(VM::tp_str, "upper", 1, [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.upper());
    });

    _vm->bind(_vm->_t(VM::tp_str), "strip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(is_none(args[1])) {
            return VAR(self.strip());
        } else {
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(true, true, chars));
        }
    });

    _vm->bind(_vm->_t(VM::tp_str), "lstrip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(is_none(args[1])) {
            return VAR(self.lstrip());
        } else {
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(true, false, chars));
        }
    });

    _vm->bind(_vm->_t(VM::tp_str), "rstrip(self, chars=None)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        if(is_none(args[1])) {
            return VAR(self.rstrip());
        } else {
            const Str& chars = CAST(Str&, args[1]);
            return VAR(self.strip(false, true, chars));
        }
    });

    // zfill
    _vm->bind(_vm->_t(VM::tp_str), "zfill(self, width)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        SStream ss;
        for(int i = 0; i < delta; i++)
            ss << '0';
        ss << self;
        return VAR(ss.str());
    });

    // ljust
    _vm->bind(_vm->_t(VM::tp_str), "ljust(self, width, fillchar=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        const Str& fillchar = CAST(Str&, args[2]);
        if(fillchar.u8_length() != 1) vm->TypeError("The fill character must be exactly one character long");
        SStream ss;
        ss << self;
        for(int i = 0; i < delta; i++)
            ss << fillchar;
        return VAR(ss.str());
    });

    // rjust
    _vm->bind(_vm->_t(VM::tp_str), "rjust(self, width, fillchar=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        int width = CAST(int, args[1]);
        int delta = width - self.u8_length();
        if(delta <= 0) return args[0];
        const Str& fillchar = CAST(Str&, args[2]);
        if(fillchar.u8_length() != 1) vm->TypeError("The fill character must be exactly one character long");
        SStream ss;
        for(int i = 0; i < delta; i++)
            ss << fillchar;
        ss << self;
        return VAR(ss.str());
    });

    // tp_list / tp_tuple
    _vm->bind(_vm->_t(VM::tp_list), "sort(self, key=None, reverse=False)", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyVar key = args[1];
        if(is_none(key)) {
            std::stable_sort(self.begin(), self.end(), [vm](PyVar a, PyVar b) {
                return vm->py_lt(a, b);
            });
        } else {
            std::stable_sort(self.begin(), self.end(), [vm, key](PyVar a, PyVar b) {
                return vm->py_lt(vm->call(key, a), vm->call(key, b));
            });
        }
        bool reverse = CAST(bool, args[2]);
        if(reverse) { std::reverse(self.begin(), self.end()); }
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_list, [](VM* vm, PyVar _0) -> Str {
        if(vm->_repr_recursion_set.contains(_0)) return "[...]";
        List& iterable = _CAST(List&, _0);
        SStream ss;
        ss << '[';
        vm->_repr_recursion_set.push_back(_0);
        for(int i = 0; i < iterable.size(); i++) {
            ss << vm->py_repr(iterable[i]);
            if(i != iterable.size() - 1) ss << ", ";
        }
        vm->_repr_recursion_set.pop_back();
        ss << ']';
        return ss.str();
    });

    _vm->bind__repr__(VM::tp_tuple, [](VM* vm, PyVar _0) -> Str {
        Tuple& iterable = _CAST(Tuple&, _0);
        SStream ss;
        ss << '(';
        if(iterable.size() == 1) {
            ss << vm->py_repr(iterable[0]);
            ss << ',';
        } else {
            for(int i = 0; i < iterable.size(); i++) {
                ss << vm->py_repr(iterable[i]);
                if(i != iterable.size() - 1) ss << ", ";
            }
        }
        ss << ')';
        return ss.str();
    });

    _vm->bind_func(VM::tp_list, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return VAR(List());
        if(args.size() == 1 + 1) return VAR(vm->py_list(args[1]));
        vm->TypeError("list() takes 0 or 1 arguments");
        return vm->None;
    });

    _vm->bind__contains__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1) {
        List& self = _CAST(List&, _0);
        for(PyVar i: self)
            if(vm->py_eq(i, _1)) return vm->True;
        return vm->False;
    });

    _vm->bind_func(VM::tp_list, "count", 2, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int count = 0;
        for(PyVar i: self)
            if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1) {
        List& a = _CAST(List&, _0);
        if(!is_type(_1, vm->tp_list)) return vm->NotImplemented;
        List& b = _CAST(List&, _1);
        if(a.size() != b.size()) return vm->False;
        for(int i = 0; i < a.size(); i++) {
            if(!vm->py_eq(a[i], b[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind(_vm->_t(VM::tp_list), "index(self, value, __start=0)", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyVar obj = args[1];
        int start = CAST(int, args[2]);
        for(int i = start; i < self.size(); i++) {
            if(vm->py_eq(self[i], obj)) return VAR(i);
        }
        vm->ValueError(vm->py_repr(obj) + " is not in list");
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "remove", 2, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyVar obj = args[1];
        for(PyVar* it = self.begin(); it != self.end(); it++) {
            if(vm->py_eq(*it, obj)) {
                self.erase(it);
                return vm->None;
            }
        }
        vm->ValueError(vm->py_repr(obj) + " is not in list");
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "pop", -1, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        if(args.size() == 1 + 0) {
            if(self.empty()) vm->IndexError("pop from empty list");
            PyVar retval = self.back();
            self.pop_back();
            return retval;
        }
        if(args.size() == 1 + 1) {
            i64 index = CAST(i64, args[1]);
            index = vm->normalized_index(index, self.size());
            PyVar ret = self[index];
            self.erase(self.begin() + index);
            return ret;
        }
        vm->TypeError("pop() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "append", 2, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "extend", 2, [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        List& self = _CAST(List&, args[0]);
        PyVar it = vm->py_iter(args[1]);  // strong ref
        const PyTypeInfo* info = vm->_tp_info(args[1]);
        PyVar obj = vm->_py_next(info, it);
        while(obj != vm->StopIteration) {
            self.push_back(obj);
            obj = vm->_py_next(info, it);
        }
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "reverse", 1, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind__mul__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1) {
        const List& self = _CAST(List&, _0);
        if(!is_int(_1)) return vm->NotImplemented;
        int n = _CAST(int, _1);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++)
            result.extend(self.begin(), self.end());
        return VAR(std::move(result));
    });
    _vm->bind_func(VM::tp_list, "__rmul__", 2, [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        if(!is_int(args[1])) return vm->NotImplemented;
        int n = _CAST(int, args[1]);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++)
            result.extend(self.begin(), self.end());
        return VAR(std::move(result));
    });

    _vm->bind_func(VM::tp_list, "insert", 3, [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int index = CAST(int, args[1]);
        if(index < 0) index += self.size();
        if(index < 0) index = 0;
        if(index > self.size()) index = self.size();
        self.insert(self.begin() + index, args[2]);
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "clear", 1, [](VM* vm, ArgsView args) {
        _CAST(List&, args[0]).clear();
        return vm->None;
    });

    _vm->bind_func(VM::tp_list, "copy", 1, [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        return VAR(List(explicit_copy_t(), self));
    });

#define BIND_RICH_CMP(name, op, _t, _T)                                                                                \
    _vm->bind__##name##__(_vm->_t, [](VM* vm, PyVar lhs, PyVar rhs) {                                                  \
        if(!is_type(rhs, vm->_t)) return vm->NotImplemented;                                                           \
        auto& a = _CAST(_T&, lhs);                                                                                     \
        auto& b = _CAST(_T&, rhs);                                                                                     \
        for(int i = 0; i < a.size() && i < b.size(); i++) {                                                            \
            if(vm->py_eq(a[i], b[i])) continue;                                                                        \
            return VAR(vm->py_##name(a[i], b[i]));                                                                     \
        }                                                                                                              \
        return VAR(a.size() op b.size());                                                                              \
    });

    BIND_RICH_CMP(lt, <, tp_list, List)
    BIND_RICH_CMP(le, <=, tp_list, List)
    BIND_RICH_CMP(gt, >, tp_list, List)
    BIND_RICH_CMP(ge, >=, tp_list, List)

    BIND_RICH_CMP(lt, <, tp_tuple, Tuple)
    BIND_RICH_CMP(le, <=, tp_tuple, Tuple)
    BIND_RICH_CMP(gt, >, tp_tuple, Tuple)
    BIND_RICH_CMP(ge, >=, tp_tuple, Tuple)

#undef BIND_RICH_CMP

    _vm->bind__add__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1) {
        const List& self = _CAST(List&, _0);
        const List& other = CAST(List&, _1);
        List new_list(explicit_copy_t(), self);
        new_list.extend(other.begin(), other.end());
        return VAR(std::move(new_list));
    });

    _vm->bind__len__(VM::tp_list, [](VM* vm, PyVar _0) {
        return (i64)_CAST(List&, _0).size();
    });
    _vm->bind__iter__(VM::tp_list, [](VM* vm, PyVar _0) {
        List& self = _CAST(List&, _0);
        return vm->new_user_object<ArrayIter>(_0.get(), self.begin(), self.end());
    });

    _vm->bind__getitem__(VM::tp_list, PyArrayGetItem<List>);
    _vm->bind__setitem__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1, PyVar _2) {
        List& self = _CAST(List&, _0);
        i64 i = CAST(i64, _1);
        i = vm->normalized_index(i, self.size());
        self[i] = _2;
    });
    _vm->bind__delitem__(VM::tp_list, [](VM* vm, PyVar _0, PyVar _1) {
        List& self = _CAST(List&, _0);
        i64 i = CAST(i64, _1);
        i = vm->normalized_index(i, self.size());
        self.erase(self.begin() + i);
    });

    _vm->bind_func(VM::tp_tuple, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return VAR(Tuple(0));
        if(args.size() == 1 + 1) {
            List list = vm->py_list(args[1]);
            return VAR(list.to_tuple());
        }
        vm->TypeError("tuple() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__contains__(VM::tp_tuple, [](VM* vm, PyVar obj, PyVar item) {
        Tuple& self = _CAST(Tuple&, obj);
        for(PyVar i: self)
            if(vm->py_eq(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_func(VM::tp_tuple, "count", 2, [](VM* vm, ArgsView args) {
        Tuple& self = _CAST(Tuple&, args[0]);
        int count = 0;
        for(PyVar i: self)
            if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(VM::tp_tuple, [](VM* vm, PyVar _0, PyVar _1) {
        const Tuple& self = _CAST(Tuple&, _0);
        if(!is_type(_1, vm->tp_tuple)) return vm->NotImplemented;
        const Tuple& other = _CAST(Tuple&, _1);
        if(self.size() != other.size()) return vm->False;
        for(int i = 0; i < self.size(); i++) {
            if(!vm->py_eq(self[i], other[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__hash__(VM::tp_tuple, [](VM* vm, PyVar _0) {
        i64 x = 1000003;
        for(PyVar item: _CAST(Tuple&, _0)) {
            i64 y = vm->py_hash(item);
            // recommended by Github Copilot
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
        }
        return x;
    });

    _vm->bind__iter__(VM::tp_tuple, [](VM* vm, PyVar _0) {
        Tuple& self = _CAST(Tuple&, _0);
        return vm->new_user_object<ArrayIter>(_0.get(), self.begin(), self.end());
    });

    _vm->bind__getitem__(VM::tp_tuple, PyArrayGetItem<Tuple>);
    _vm->bind__len__(VM::tp_tuple, [](VM* vm, PyVar obj) {
        return (i64)_CAST(Tuple&, obj).size();
    });

    // tp_bool
    _vm->bind_func(VM::tp_bool, __new__, 2, PK_LAMBDA(VAR(vm->py_bool(args[1]))));
    _vm->bind__hash__(VM::tp_bool, [](VM* vm, PyVar _0) {
        return (i64)_CAST(bool, _0);
    });
    _vm->bind__repr__(VM::tp_bool, [](VM* vm, PyVar _0) -> Str {
        bool val = _CAST(bool, _0);
        return val ? "True" : "False";
    });

    _vm->bind__and__(VM::tp_bool, [](VM* vm, PyVar _0, PyVar _1) {
        return VAR(_CAST(bool, _0) && CAST(bool, _1));
    });
    _vm->bind__or__(VM::tp_bool, [](VM* vm, PyVar _0, PyVar _1) {
        return VAR(_CAST(bool, _0) || CAST(bool, _1));
    });
    _vm->bind__xor__(VM::tp_bool, [](VM* vm, PyVar _0, PyVar _1) {
        return VAR(_CAST(bool, _0) != CAST(bool, _1));
    });
    _vm->bind__eq__(VM::tp_bool, [](VM* vm, PyVar _0, PyVar _1) {
        if(is_type(_1, vm->tp_bool)) return VAR(_0 == _1);
        if(is_int(_1)) return VAR(_CAST(bool, _0) == (bool)CAST(i64, _1));
        return vm->NotImplemented;
    });

    // tp_ellipsis / tp_NotImplementedType
    _vm->bind__repr__(_vm->_tp(_vm->Ellipsis), [](VM* vm, PyVar _0) -> Str {
        return "...";
    });
    _vm->bind__repr__(_vm->_tp(_vm->NotImplemented), [](VM* vm, PyVar _0) -> Str {
        return "NotImplemented";
    });

    // tp_bytes
    _vm->bind_func(VM::tp_bytes, __new__, 2, [](VM* vm, ArgsView args) {
        List& list = CAST(List&, args[1]);
        Bytes retval(list.size());
        for(int i = 0; i < list.size(); i++) {
            i64 b = CAST(i64, list[i]);
            if(b < 0 || b > 255) vm->ValueError("byte must be in range[0, 256)");
            retval[i] = (char)b;
        }
        return VAR(std::move(retval));
    });

    _vm->bind__getitem__(VM::tp_bytes, [](VM* vm, PyVar _0, PyVar _1) {
        const Bytes& self = PK_OBJ_GET(Bytes, _0);
        if(is_type(_1, vm->tp_slice)) {
            const Slice& s = _CAST(Slice&, _1);
            int start, stop, step;
            vm->parse_int_slice(s, self.size(), start, stop, step);
            int guess_max_size = abs(stop - start) / abs(step) + 1;
            if(guess_max_size > self.size()) guess_max_size = self.size();
            unsigned char* buffer = (unsigned char*)std::malloc(guess_max_size);
            int j = 0;  // actual size
            PK_SLICE_LOOP(i, start, stop, step) buffer[j++] = self[i];
            return VAR(Bytes(buffer, j));
        }
        i64 i = CAST(i64, _1);
        i = vm->normalized_index(i, self.size());
        return VAR(self[i]);
    });

    _vm->bind__add__(VM::tp_bytes, [](VM* vm, PyVar _0, PyVar _1) {
        const Bytes& a = _CAST(Bytes&, _0);
        const Bytes& b = CAST(Bytes&, _1);
        Bytes retval(a.size() + b.size());
        std::memcpy(retval.data(), a.data(), a.size());
        std::memcpy(retval.data() + a.size(), b.data(), b.size());
        return VAR(std::move(retval));
    });

    _vm->bind__hash__(VM::tp_bytes, [](VM* vm, PyVar _0) {
        const Bytes& self = _CAST(Bytes&, _0);
        std::string_view view((char*)self.data(), self.size());
        return (i64)std::hash<std::string_view>()(view);
    });

    _vm->bind__repr__(VM::tp_bytes, [](VM* vm, PyVar _0) -> Str {
        const Bytes& self = _CAST(Bytes&, _0);
        SStream ss;
        ss << "b'";
        for(int i = 0; i < self.size(); i++) {
            ss << "\\x";
            ss.write_hex((unsigned char)self[i]);
        }
        ss << "'";
        return ss.str();
    });
    _vm->bind__len__(VM::tp_bytes, [](VM* vm, PyVar _0) {
        return (i64)_CAST(Bytes&, _0).size();
    });

    _vm->bind_func(VM::tp_bytes, "decode", 1, [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        return VAR(Str(std::string_view((char*)self.data(), self.size())));
    });

    _vm->bind__eq__(VM::tp_bytes, [](VM* vm, PyVar _0, PyVar _1) {
        if(!is_type(_1, vm->tp_bytes)) return vm->NotImplemented;
        const Bytes& lhs = _CAST(Bytes&, _0);
        const Bytes& rhs = _CAST(Bytes&, _1);
        if(lhs.size() != rhs.size()) return vm->False;
        return VAR(memcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
    });

    // tp_slice
    _vm->bind_func(VM::tp_slice, __new__, 4, [](VM* vm, ArgsView args) {
        return VAR(Slice(args[1], args[2], args[3]));
    });

    _vm->bind__eq__(VM::tp_slice, [](VM* vm, PyVar _0, PyVar _1) {
        const Slice& self = _CAST(Slice&, _0);
        if(!is_type(_1, vm->tp_slice)) return vm->NotImplemented;
        const Slice& other = _CAST(Slice&, _1);
        if(vm->py_ne(self.start, other.start)) return vm->False;
        if(vm->py_ne(self.stop, other.stop)) return vm->False;
        if(vm->py_ne(self.step, other.step)) return vm->False;
        return vm->True;
    });

    _vm->bind__repr__(VM::tp_slice, [](VM* vm, PyVar _0) -> Str {
        const Slice& self = _CAST(Slice&, _0);
        SStream ss;
        ss << "slice(";
        ss << vm->py_repr(self.start) << ", ";
        ss << vm->py_repr(self.stop) << ", ";
        ss << vm->py_repr(self.step) << ")";
        return ss.str();
    });

    // tp_mappingproxy
    _vm->bind_func(VM::tp_mappingproxy, "keys", 1, [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List keys;
        for(StrName name: self.attr().keys())
            keys.push_back(VAR(name.sv()));
        return VAR(std::move(keys));
    });

    _vm->bind_func(VM::tp_mappingproxy, "values", 1, [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List values;
        for(auto [k, v]: self.attr().items())
            values.push_back(v);
        return VAR(std::move(values));
    });

    _vm->bind_func(VM::tp_mappingproxy, "items", 1, [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        List items;
        for(auto [k, v]: self.attr().items()) {
            PyVar t = VAR(Tuple(VAR(k.sv()), v));
            items.push_back(std::move(t));
        }
        return VAR(std::move(items));
    });

    _vm->bind__len__(VM::tp_mappingproxy, [](VM* vm, PyVar _0) {
        return (i64)_CAST(MappingProxy&, _0).attr().size();
    });

    _vm->bind__eq__(VM::tp_mappingproxy, [](VM* vm, PyVar _0, PyVar _1) {
        const MappingProxy& a = _CAST(MappingProxy&, _0);
        if(!is_type(_1, VM::tp_mappingproxy)) return vm->NotImplemented;
        const MappingProxy& b = _CAST(MappingProxy&, _1);
        return VAR(a.obj == b.obj);
    });

    _vm->bind__getitem__(VM::tp_mappingproxy, [](VM* vm, PyVar _0, PyVar _1) {
        MappingProxy& self = _CAST(MappingProxy&, _0);
        StrName key = CAST(Str&, _1);
        PyVar ret = self.attr().try_get_likely_found(key);
        if(ret == nullptr) vm->KeyError(_1);
        return ret;
    });

    _vm->bind(_vm->_t(VM::tp_mappingproxy), "get(self, key, default=None)", [](VM* vm, ArgsView args) {
        MappingProxy& self = _CAST(MappingProxy&, args[0]);
        StrName key = CAST(Str&, args[1]);
        PyVar ret = self.attr().try_get(key);
        if(ret == nullptr) return args[2];
        return ret;
    });

    _vm->bind__repr__(VM::tp_mappingproxy, [](VM* vm, PyVar _0) -> Str {
        if(vm->_repr_recursion_set.contains(_0)) return "{...}";
        MappingProxy& self = _CAST(MappingProxy&, _0);
        SStream ss;
        ss << "mappingproxy({";
        bool first = true;
        vm->_repr_recursion_set.push_back(_0);
        for(auto [k, v]: self.attr().items()) {
            if(!first) ss << ", ";
            first = false;
            ss << k.escape() << ": ";
            ss << vm->py_repr(v);
        }
        vm->_repr_recursion_set.pop_back();
        ss << "})";
        return ss.str();
    });

    _vm->bind__contains__(VM::tp_mappingproxy, [](VM* vm, PyVar _0, PyVar _1) {
        MappingProxy& self = _CAST(MappingProxy&, _0);
        return VAR(self.attr().contains(CAST(Str&, _1)));
    });

    // tp_dict
    _vm->bind_func(VM::tp_dict, __new__, -1, [](VM* vm, ArgsView args) {
        Type cls_t = PK_OBJ_GET(Type, args[0]);
        return vm->new_object<Dict>(cls_t);
    });

    _vm->bind_func(VM::tp_dict, __init__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 0) return vm->None;
        if(args.size() == 1 + 1) {
            auto _lock = vm->heap.gc_scope_lock();
            Dict& self = PK_OBJ_GET(Dict, args[0]);
            if(is_type(args[1], vm->tp_dict)) {
                Dict& other = CAST(Dict&, args[1]);
                self.update(vm, other);
                return vm->None;
            }
            if(is_type(args[1], vm->tp_list)) {
                List& list = PK_OBJ_GET(List, args[1]);
                for(PyVar item: list) {
                    Tuple& t = CAST(Tuple&, item);
                    if(t.size() != 2) {
                        vm->ValueError("dict() takes a list of tuples (key, value)");
                        return vm->None;
                    }
                    self.set(vm, t[0], t[1]);
                }
                return vm->None;
            }
            vm->TypeError("dict() takes a dictionary or a list of tuples");
        }
        vm->TypeError("dict() takes at most 1 argument");
    });

    _vm->bind__len__(VM::tp_dict, [](VM* vm, PyVar _0) {
        return (i64)PK_OBJ_GET(Dict, _0).size();
    });

    _vm->bind__getitem__(VM::tp_dict, [](VM* vm, PyVar _0, PyVar _1) {
        Dict& self = PK_OBJ_GET(Dict, _0);
        PyVar ret = self.try_get(vm, _1);
        if(ret == nullptr) {
            // try __missing__
            PyVar self;
            PyVar f_missing = vm->get_unbound_method(_0, __missing__, &self, false);
            if(f_missing != nullptr) { return vm->call_method(self, f_missing, _1); }
            vm->KeyError(_1);
        }
        return ret;
    });

    _vm->bind__setitem__(VM::tp_dict, [](VM* vm, PyVar _0, PyVar _1, PyVar _2) {
        Dict& self = _CAST(Dict&, _0);
        self.set(vm, _1, _2);
    });

    _vm->bind__delitem__(VM::tp_dict, [](VM* vm, PyVar _0, PyVar _1) {
        Dict& self = _CAST(Dict&, _0);
        bool ok = self.del(vm, _1);
        if(!ok) vm->KeyError(_1);
    });

    _vm->bind_func(VM::tp_dict, "pop", -1, [](VM* vm, ArgsView args) {
        if(args.size() != 2 && args.size() != 3) {
            vm->TypeError("pop() expected 1 or 2 arguments");
            return vm->None;
        }
        Dict& self = _CAST(Dict&, args[0]);
        PyVar value = self.try_get(vm, args[1]);
        if(value == nullptr) {
            if(args.size() == 2) vm->KeyError(args[1]);
            if(args.size() == 3) { return args[2]; }
        }
        self.del(vm, args[1]);
        return value;
    });

    _vm->bind__contains__(VM::tp_dict, [](VM* vm, PyVar _0, PyVar _1) {
        Dict& self = _CAST(Dict&, _0);
        return VAR(self.contains(vm, _1));
    });

    _vm->bind__iter__(VM::tp_dict, [](VM* vm, PyVar _0) {
        const Dict& self = _CAST(Dict&, _0);
        return vm->py_iter(VAR(self.keys()));
    });

    _vm->bind_func(VM::tp_dict, "get", -1, [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        if(args.size() == 1 + 1) {
            PyVar ret = self.try_get(vm, args[1]);
            if(ret != nullptr) return ret;
            return vm->None;
        } else if(args.size() == 1 + 2) {
            PyVar ret = self.try_get(vm, args[1]);
            if(ret != nullptr) return ret;
            return args[2];
        }
        vm->TypeError("get() takes at most 2 arguments");
        return vm->None;
    });

    _vm->bind_func(VM::tp_dict, "keys", 1, [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.keys());
    });

    _vm->bind_func(VM::tp_dict, "values", 1, [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.values());
    });

    _vm->bind_func(VM::tp_dict, "items", 1, [](VM* vm, ArgsView args) {
        return vm->new_user_object<DictItemsIter>(args[0]);
    });

    _vm->bind_func(VM::tp_dict, "update", 2, [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        const Dict& other = CAST(Dict&, args[1]);
        self.update(vm, other);
        return vm->None;
    });

    _vm->bind_func(VM::tp_dict, "copy", 1, [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self);
    });

    _vm->bind_func(VM::tp_dict, "clear", 1, [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        self.clear();
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_dict, [](VM* vm, PyVar _0) -> Str {
        if(vm->_repr_recursion_set.contains(_0)) return "{...}";
        Dict& self = _CAST(Dict&, _0);
        SStream ss;
        ss << "{";
        bool first = true;
        vm->_repr_recursion_set.push_back(_0);
        self.apply([&](PyVar k, PyVar v) {
            if(!first) ss << ", ";
            first = false;
            ss << vm->py_repr(k) << ": " << vm->py_repr(v);
        });
        vm->_repr_recursion_set.pop_back();
        ss << "}";
        return ss.str();
    });

    _vm->bind__eq__(VM::tp_dict, [](VM* vm, PyVar _0, PyVar _1) {
        Dict& self = _CAST(Dict&, _0);
        if(!vm->isinstance(_1, vm->tp_dict)) return vm->NotImplemented;
        Dict& other = _CAST(Dict&, _1);
        if(self.size() != other.size()) return vm->False;
        pkpy_DictIter it = self.iter();
        PyVar key, val;
        while(pkpy_DictIter__next(&it, reinterpret_cast<::pkpy_Var*>(&key), reinterpret_cast<::pkpy_Var*>(&val))) {
            if(!vm->py_eq(val, other.try_get(vm, key))) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__repr__(VM::tp_module, [](VM* vm, PyVar _0) -> Str {
        const Str& path = CAST(Str&, _0->attr(__path__));
        return _S("<module ", path.escape(), ">");
    });

    // tp_property
    _vm->bind_func(VM::tp_property, __new__, -1, [](VM* vm, ArgsView args) {
        if(args.size() == 1 + 1) {
            return VAR(Property(args[1], vm->None));
        } else if(args.size() == 1 + 2) {
            return VAR(Property(args[1], args[2]));
        }
        vm->TypeError("property() takes at most 2 arguments");
        return vm->None;
    });

    _vm->bind_property(_vm->_t(VM::tp_function), "__doc__", [](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        if(!func.decl->docstring) return vm->None;
        return VAR(func.decl->docstring);
    });

    _vm->bind_property(_vm->_t(VM::tp_native_func), "__doc__", [](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl == nullptr) return vm->None;
        if(!func.decl->docstring) return vm->None;
        return VAR(func.decl->docstring);
    });

    // tp_exception
    _vm->bind_func(VM::tp_exception, __new__, -1, [](VM* vm, ArgsView args) -> PyVar {
        Type cls = PK_OBJ_GET(Type, args[0]);
        StrName cls_name = _type_name(vm, cls);
        PyObject* e_obj = vm->heap.gcnew<Exception>(cls, cls_name);
        e_obj->_attr = new NameDict();
        e_obj->as<Exception>()._self = e_obj;
        return e_obj;
    });

    _vm->bind(_vm->_t(VM::tp_exception), "__init__(self, msg=...)", [](VM* vm, ArgsView args) {
        Exception& self = _CAST(Exception&, args[0]);
        if(args[1] == vm->Ellipsis) {
            self.msg = "";
        } else {
            self.msg = CAST(Str, args[1]);
        }
        return vm->None;
    });

    _vm->bind__repr__(VM::tp_exception, [](VM* vm, PyVar _0) -> Str {
        Exception& self = _CAST(Exception&, _0);
        return _S(_type_name(vm, _0.type), '(', self.msg.escape(), ')');
    });

    _vm->bind__str__(VM::tp_exception, [](VM* vm, PyVar _0) -> Str {
        Exception& self = _CAST(Exception&, _0);
        return self.msg;
    });

    _vm->register_user_class<RangeIter>(_vm->builtins, "_range_iter");
    _vm->register_user_class<RangeIterR>(_vm->builtins, "_range_iter_r");
    _vm->register_user_class<ArrayIter>(_vm->builtins, "_array_iter");
    _vm->register_user_class<StringIter>(_vm->builtins, "_string_iter");
    _vm->register_user_class<Generator>(_vm->builtins, "generator");
    _vm->register_user_class<DictItemsIter>(_vm->builtins, "_dict_items_iter");
}

void VM::__post_init_builtin_types() {
    __init_builtins(this);

    bind_func(tp_module, __new__, -1, PK_ACTION(vm->NotImplementedError()));

    _all_types[tp_module].m__getattr__ = [](VM* vm, PyVar obj, StrName name) -> PyVar {
        const Str& path = CAST(Str&, obj->attr(__path__));
        PyObject* retval = vm->py_import(_S(path, ".", name.sv()), false);
        if(retval) return retval;
        return nullptr;
    };

    bind_func(tp_property, "setter", 2, [](VM* vm, ArgsView args) {
        Property& self = _CAST(Property&, args[0]);
        // The setter's name is not necessary to be the same as the property's name
        // However, for cpython compatibility, we recommend to use the same name
        self.setter = args[1];
        return args[0];
    });

    // type
    bind__getitem__(tp_type, [](VM* vm, PyVar self, PyVar _) {
        return self;  // for generics
    });

    bind__repr__(tp_type, [](VM* vm, PyVar self) -> Str {
        SStream ss;
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, self)];
        ss << "<class '" << info.name << "'>";
        return ss.str();
    });

    bind_property(_t(tp_object), "__class__", PK_LAMBDA(vm->_t(args[0])));
    bind_property(_t(tp_type), "__base__", [](VM* vm, ArgsView args) {
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return info.base ? vm->_all_types[info.base].obj : vm->None;
    });
    bind_property(_t(tp_type), "__name__", [](VM* vm, ArgsView args) {
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return VAR(info.name.sv());
    });
    bind_property(_t(tp_type), "__module__", [](VM* vm, ArgsView args) -> PyVar {
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        if(info.mod == nullptr) return vm->None;
        return info.mod;
    });
    bind_property(_t(tp_bound_method), "__self__", [](VM* vm, ArgsView args) {
        return CAST(BoundMethod&, args[0]).self;
    });
    bind_property(_t(tp_bound_method), "__func__", [](VM* vm, ArgsView args) {
        return CAST(BoundMethod&, args[0]).func;
    });

    bind__eq__(tp_bound_method, [](VM* vm, PyVar lhs, PyVar rhs) {
        if(!is_type(rhs, vm->tp_bound_method)) return vm->NotImplemented;
        const BoundMethod& _0 = PK_OBJ_GET(BoundMethod, lhs);
        const BoundMethod& _1 = PK_OBJ_GET(BoundMethod, rhs);
        return VAR(_0.self == _1.self && _0.func == _1.func);
    });

    bind_property(_t(tp_slice), "start", [](VM* vm, ArgsView args) {
        return CAST(Slice&, args[0]).start;
    });
    bind_property(_t(tp_slice), "stop", [](VM* vm, ArgsView args) {
        return CAST(Slice&, args[0]).stop;
    });
    bind_property(_t(tp_slice), "step", [](VM* vm, ArgsView args) {
        return CAST(Slice&, args[0]).step;
    });

    bind_property(_t(tp_object), "__dict__", [](VM* vm, ArgsView args) {
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()) return vm->None;
        return VAR(MappingProxy(args[0].get()));
    });

    bind(builtins, "print(*args, sep=' ', end='\\n')", [](VM* vm, ArgsView args) {
        const Tuple& _0 = CAST(Tuple&, args[0]);
        const Str& _1 = CAST(Str&, args[1]);
        const Str& _2 = CAST(Str&, args[2]);
        SStream ss;
        for(int i = 0; i < _0.size(); i++) {
            ss << vm->py_str(_0[i]);
            if(i != _0.size() - 1) ss << _1;
        }
        ss << _2;
        vm->stdout_write(ss.str());
        return vm->None;
    });

    add_module___builtins(vm);
    add_module_sys(this);
    add_module_traceback(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_dis(this);
    add_module_c(this);
    add_module_gc(this);
    add_module_random(this);
    add_module_base64(this);

    _lazy_modules.insert("this", kPythonLibs_this);
    _lazy_modules.insert("functools", kPythonLibs_functools);
    _lazy_modules.insert("heapq", kPythonLibs_heapq);
    _lazy_modules.insert("bisect", kPythonLibs_bisect);
    _lazy_modules.insert("pickle", kPythonLibs_pickle);
    _lazy_modules.insert("_long", kPythonLibs__long);
    _lazy_modules.insert("colorsys", kPythonLibs_colorsys);
    _lazy_modules.insert("typing", kPythonLibs_typing);
    _lazy_modules.insert("datetime", kPythonLibs_datetime);
    _lazy_modules.insert("cmath", kPythonLibs_cmath);
    _lazy_modules.insert("itertools", kPythonLibs_itertools);
    _lazy_modules.insert("operator", kPythonLibs_operator);
    _lazy_modules.insert("collections", kPythonLibs_collections);

    try {
        // initialize dummy func_decl for exec/eval
        CodeObject_ dynamic_co = compile("def _(): pass", "<dynamic>", EXEC_MODE);
        __dynamic_func_decl = dynamic_co->func_decls[0];
        // initialize builtins
        CodeObject_ code = compile(kPythonLibs_builtins, "<builtins>", EXEC_MODE);
        this->_exec(code, this->builtins);
        code = compile(kPythonLibs__set, "<set>", EXEC_MODE);
        this->_exec(code, this->builtins);
    } catch(TopLevelException e) {
        std::cerr << e.summary() << std::endl;
        std::cerr << "failed to load builtins module!!" << std::endl;
        exit(1);
    }

    if(enable_os) {
        add_module_io(this);
        add_module_os(this);
        _import_handler = &_default_import_handler;
    }

    add_module_csv(this);
    add_module_dataclasses(this);
    add_module_linalg(this);
    add_module_easing(this);
    add_module_array2d(this);
    add_module_line_profiler(this);
    add_module_enum(this);

#ifdef PK_USE_CJSON
    add_module_cjson(this);
#endif
}

CodeObject_ VM::compile(std::string_view source, const Str& filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    CodeObject_ code;
    Error* err = compiler.compile(&code);
    if(err) __compile_error(err);
    return code;
}

void VM::__compile_error(Error* err){
    assert(err != nullptr);
    if(err->type == std::string_view("NeedMoreLines")){
        throw NeedMoreLines((bool)err->userdata);
    }
    __last_exception = vm->call(
        vm->builtins->attr(err->type),
        VAR((const char*)err->msg)
    ).get();
    Exception& e = __last_exception->as<Exception>();
    e.st_push(err->src, err->lineno, err->cursor, "");
    _error(__last_exception);
}

Str VM::precompile(std::string_view source, const Str& filename, CompileMode mode) {
    Compiler compiler(this, source, filename, mode, false);
    Str out;
    Error* err = compiler.lexer.precompile(&out);
    if(err) __compile_error(err);
    return out;
}

}  // namespace pkpy
