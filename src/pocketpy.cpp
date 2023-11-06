#include "pocketpy/pocketpy.h"

#ifdef PK_USE_BOX2D
#include "box2dw.hpp"
#endif

#ifdef PK_USE_CJSON
#include "cJSONw.hpp"
#endif

#if defined (_WIN32) && PK_SUPPORT_DYLIB == 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace pkpy{

using dylib_entry_t = const char* (*)(void*, const char*);

#if PK_ENABLE_OS

#if PK_SUPPORT_DYLIB == 1
// win32
static dylib_entry_t load_dylib(const char* path){
    std::error_code ec;
    auto p = std::filesystem::absolute(path, ec);
    if(ec) return nullptr;
    HMODULE handle = LoadLibraryA(p.string().c_str());
    if(!handle){
        DWORD errorCode = GetLastError();
        // Convert the error code to text
        LPSTR errorMessage = nullptr;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            errorCode,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPSTR)&errorMessage,
            0,
            nullptr
        );
        printf("%lu: %s\n", errorCode, errorMessage);
        LocalFree(errorMessage);
        return nullptr;
    }
    return (dylib_entry_t)GetProcAddress(handle, "pkpy_module__init__");
}
#elif PK_SUPPORT_DYLIB == 2
// linux/darwin
static dylib_entry_t load_dylib(const char* path){
    std::error_code ec;
    auto p = std::filesystem::absolute(path, ec);
    if(ec) return nullptr;
    void* handle = dlopen(p.c_str(), RTLD_LAZY);
    if(!handle) return nullptr;
    return (dylib_entry_t)dlsym(handle, "pkpy_module__init__");
}

#elif PK_SUPPORT_DYLIB == 3
// android
static dylib_entry_t load_dylib(const char* path){
    void* handle = dlopen(path, RTLD_LAZY);
    if(!handle) return nullptr;
    return (dylib_entry_t)dlsym(handle, "pkpy_module__init__");
}

#else
static dylib_entry_t load_dylib(const char* path){
    return nullptr;
}
#endif

#else
static dylib_entry_t load_dylib(const char* path){
    return nullptr;
}
#endif

void init_builtins(VM* _vm) {
#define BIND_NUM_ARITH_OPT(name, op)                                                                    \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {                             \
        if(is_int(rhs)) return VAR(_CAST(i64, lhs) op _CAST(i64, rhs));                                 \
        if(is_float(rhs)) return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));                               \
        return vm->NotImplemented;                                                                      \
    });                                                                                                 \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {                           \
        if(is_float(rhs)) return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));                               \
        if(is_int(rhs)) return VAR(_CAST(f64, lhs) op _CAST(i64, rhs));                                 \
        return vm->NotImplemented;                                                                      \
    });

    BIND_NUM_ARITH_OPT(__add__, +)
    BIND_NUM_ARITH_OPT(__sub__, -)
    BIND_NUM_ARITH_OPT(__mul__, *)

#undef BIND_NUM_ARITH_OPT

#define BIND_NUM_LOGICAL_OPT(name, op)   \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {     \
        i64 val;                                                                \
        if(try_cast_int(rhs, &val)) return VAR(_CAST(i64, lhs) op val);         \
        if(is_float(rhs))   return VAR(_CAST(i64, lhs) op _CAST(f64, rhs));     \
        return vm->NotImplemented;                                              \
    });                                                                         \
    _vm->bind##name(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {   \
        i64 val;                                                                \
        if(try_cast_int(rhs, &val)) return VAR(_CAST(f64, lhs) op val);         \
        if(is_float(rhs))   return VAR(_CAST(f64, lhs) op _CAST(f64, rhs));     \
        return vm->NotImplemented;                                              \
    });

    BIND_NUM_LOGICAL_OPT(__eq__, ==)
    BIND_NUM_LOGICAL_OPT(__lt__, <)
    BIND_NUM_LOGICAL_OPT(__le__, <=)
    BIND_NUM_LOGICAL_OPT(__gt__, >)
    BIND_NUM_LOGICAL_OPT(__ge__, >=)
    
#undef BIND_NUM_ARITH_OPT
#undef BIND_NUM_LOGICAL_OPT

    _vm->bind_builtin_func<-1>("super", [](VM* vm, ArgsView args) {
        PyObject* class_arg = nullptr;
        PyObject* self_arg = nullptr;
        if(args.size() == 2){
            class_arg = args[0];
            self_arg = args[1];
        }else if(args.size() == 0){
            FrameId frame = vm->top_frame();
            if(frame->_callable != nullptr){
                class_arg = PK_OBJ_GET(Function, frame->_callable)._class;
                if(frame->_locals.size() > 0) self_arg = frame->_locals[0];
            }
            if(class_arg == nullptr || self_arg == nullptr){
                vm->TypeError("super(): unable to determine the class context, use super(class, self) instead");
            }
        }else{
            vm->TypeError("super() takes 0 or 2 arguments");
        }
        vm->check_non_tagged_type(class_arg, vm->tp_type);
        Type type = PK_OBJ_GET(Type, class_arg);
        if(!vm->isinstance(self_arg, type)){
            Str _0 = obj_type_name(vm, vm->_tp(self_arg));
            Str _1 = obj_type_name(vm, type);
            vm->TypeError("super(): " + _0.escape() + " is not an instance of " + _1.escape());
        }
        return vm->heap.gcnew<Super>(vm->tp_super, self_arg, vm->_all_types[type].base);
    });

    _vm->bind_builtin_func<2>("isinstance", [](VM* vm, ArgsView args) {
        if(is_non_tagged_type(args[1], vm->tp_tuple)){
            Tuple& types = _CAST(Tuple&, args[1]);
            for(PyObject* type : types){
                vm->check_non_tagged_type(type, vm->tp_type);
                if(vm->isinstance(args[0], PK_OBJ_GET(Type, type))) return vm->True;
            }
            return vm->False;
        }
        vm->check_non_tagged_type(args[1], vm->tp_type);
        Type type = PK_OBJ_GET(Type, args[1]);
        return VAR(vm->isinstance(args[0], type));
    });

    _vm->bind_builtin_func<0>("globals", [](VM* vm, ArgsView args) {
        PyObject* mod = vm->top_frame()->_module;
        return VAR(MappingProxy(mod));
    });

    _vm->bind(_vm->builtins, "round(x, ndigits=0)", [](VM* vm, ArgsView args) {
        f64 x = CAST(f64, args[0]);
        int ndigits = CAST(int, args[1]);
        if(ndigits == 0){
            return x >= 0 ? VAR((i64)(x + 0.5)) : VAR((i64)(x - 0.5));
        }
        if(ndigits < 0) vm->ValueError("ndigits should be non-negative");
        if(x >= 0){
            return VAR((i64)(x * std::pow(10, ndigits) + 0.5) / std::pow(10, ndigits));
        }else{
            return VAR((i64)(x * std::pow(10, ndigits) - 0.5) / std::pow(10, ndigits));
        }
    });

    _vm->bind_builtin_func<1>("abs", [](VM* vm, ArgsView args) {
        if(is_int(args[0])) return VAR(std::abs(_CAST(i64, args[0])));
        if(is_float(args[0])) return VAR(std::abs(_CAST(f64, args[0])));
        vm->TypeError("bad operand type for abs()");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("id", [](VM* vm, ArgsView args) {
        PyObject* obj = args[0];
        if(is_tagged(obj)) return vm->None;
        return VAR(PK_BITS(obj));
    });

    _vm->bind_builtin_func<1>("callable", [](VM* vm, ArgsView args) {
        PyObject* cls = vm->_t(args[0]);
        Type t = PK_OBJ_GET(Type, cls);
        if(t == vm->tp_function) return vm->True;
        if(t == vm->tp_native_func) return vm->True;
        if(t == vm->tp_bound_method) return vm->True;
        if(t == vm->tp_type) return vm->True;
        bool ok = vm->find_name_in_mro(cls, __call__) != nullptr;
        return VAR(ok);
    });

    _vm->bind_builtin_func<1>("__import__", [](VM* vm, ArgsView args) {
        const Str& name = CAST(Str&, args[0]);
        auto dot = name.sv().find_last_of(".");
        if(dot != std::string_view::npos){
            auto ext = name.sv().substr(dot);
            if(ext == ".so" || ext == ".dll" || ext == ".dylib"){
                dylib_entry_t entry = load_dylib(name.c_str());
                if(!entry){
                    vm->ImportError("cannot load dynamic library: " + name.escape());
                }
                vm->_c.s_view.push(ArgsView(vm->s_data.end(), vm->s_data.end()));
                const char* name = entry(vm, PK_VERSION);
                vm->_c.s_view.pop();
                if(name == nullptr){
                    vm->ImportError("module initialization failed: " + Str(name).escape());
                }
                return vm->_modules[name];
            }
        }
        return vm->py_import(name);
    });

    _vm->bind_builtin_func<2>("divmod", [](VM* vm, ArgsView args) {
        if(is_int(args[0])){
            i64 lhs = _CAST(i64, args[0]);
            i64 rhs = CAST(i64, args[1]);
            if(rhs == 0) vm->ZeroDivisionError();
            auto res = std::div(lhs, rhs);
            return VAR(Tuple({VAR(res.quot), VAR(res.rem)}));
        }else{
            return vm->call_method(args[0], __divmod__, args[1]);
        }
    });

    _vm->bind(_vm->builtins, "eval(__source, __globals=None)", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<eval>", EVAL_MODE, true);
        PyObject* globals = args[1];
        if(globals == vm->None){
            FrameId frame = vm->top_frame();
            return vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
        }
        vm->check_non_tagged_type(globals, vm->tp_mappingproxy);
        PyObject* obj = PK_OBJ_GET(MappingProxy, globals).obj;
        return vm->_exec(code, obj);
    });

    _vm->bind(_vm->builtins, "exec(__source, __globals=None)", [](VM* vm, ArgsView args) {
        CodeObject_ code = vm->compile(CAST(Str&, args[0]), "<exec>", EXEC_MODE, true);
        PyObject* globals = args[1];
        if(globals == vm->None){
            FrameId frame = vm->top_frame();
            vm->_exec(code.get(), frame->_module, frame->_callable, frame->_locals);
            return vm->None;
        }
        vm->check_non_tagged_type(globals, vm->tp_mappingproxy);
        PyObject* obj = PK_OBJ_GET(MappingProxy, globals).obj;
        vm->_exec(code, obj);
        return vm->None;
    });

    _vm->bind_builtin_func<-1>("exit", [](VM* vm, ArgsView args) {
        if(args.size() == 0) std::exit(0);
        else if(args.size() == 1) std::exit(CAST(int, args[0]));
        else vm->TypeError("exit() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_builtin_func<1>("repr", PK_LAMBDA(vm->py_repr(args[0])));

    _vm->bind_builtin_func<1>("len", [](VM* vm, ArgsView args){
        const PyTypeInfo* ti = vm->_inst_type_info(args[0]);
        if(ti->m__len__) return VAR(ti->m__len__(vm, args[0]));
        return vm->call_method(args[0], __len__);
    });

    _vm->bind_builtin_func<1>("hash", [](VM* vm, ArgsView args){
        i64 value = vm->py_hash(args[0]);
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

    _vm->bind_builtin_func<2>("delattr", [](VM* vm, ArgsView args) {
        vm->delattr(args[0], CAST(Str&, args[1]));
        return vm->None;
    });

    _vm->bind_builtin_func<1>("hex", [](VM* vm, ArgsView args) {
        std::stringstream ss; // hex
        ss << std::hex << CAST(i64, args[0]);
        return VAR("0x" + ss.str());
    });

    _vm->bind_builtin_func<1>("iter", [](VM* vm, ArgsView args) {
        return vm->py_iter(args[0]);
    });

    _vm->bind_builtin_func<1>("next", [](VM* vm, ArgsView args) {
        return vm->py_next(args[0]);
    });

    _vm->bind_builtin_func<1>("bin", [](VM* vm, ArgsView args) {
        SStream ss;
        i64 x = CAST(i64, args[0]);
        if(x < 0){ ss << "-"; x = -x; }
        ss << "0b";
        std::string bits;
        while(x){
            bits += (x & 1) ? '1' : '0';
            x >>= 1;
        }
        std::reverse(bits.begin(), bits.end());
        if(bits.empty()) bits = "0";
        ss << bits;
        return VAR(ss.str());
    });

    _vm->bind_builtin_func<1>("dir", [](VM* vm, ArgsView args) {
        std::set<StrName> names;
        if(!is_tagged(args[0]) && args[0]->is_attr_valid()){
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

    _vm->bind__repr__(_vm->tp_object, [](VM* vm, PyObject* obj) {
        if(is_tagged(obj)) FATAL_ERROR();
        std::stringstream ss; // hex
        ss << "<" << OBJ_NAME(vm->_t(obj)) << " object at 0x";
        ss << std::hex << reinterpret_cast<intptr_t>(obj) << ">";
        return VAR(ss.str());
    });

    _vm->bind__eq__(_vm->tp_object, [](VM* vm, PyObject* lhs, PyObject* rhs) { return VAR(lhs == rhs); });

    _vm->cached_object__new__ = _vm->bind_constructor<1>("object", [](VM* vm, ArgsView args) {
        vm->check_non_tagged_type(args[0], vm->tp_type);
        Type t = PK_OBJ_GET(Type, args[0]);
        return vm->heap.gcnew<DummyInstance>(t);
    });

    _vm->bind_method<0>("object", "_enable_instance_dict", [](VM* vm, ArgsView args){
        PyObject* self = args[0];
        if(is_tagged(self)){
            vm->TypeError("object: tagged object cannot enable instance dict");
        }
        if(self->is_attr_valid()){
            vm->TypeError("object: instance dict is already enabled");
        }
        self->_enable_instance_dict();
        return vm->None;
    });

    _vm->bind_constructor<2>("type", PK_LAMBDA(vm->_t(args[1])));

    _vm->bind_constructor<-1>("range", [](VM* vm, ArgsView args) {
        args._begin += 1;   // skip cls
        Range r;
        switch (args.size()) {
            case 1: r.stop = CAST(i64, args[0]); break;
            case 2: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); break;
            case 3: r.start = CAST(i64, args[0]); r.stop = CAST(i64, args[1]); r.step = CAST(i64, args[2]); break;
            default: vm->TypeError("expected 1-3 arguments, got " + std::to_string(args.size()));
        }
        return VAR(r);
    });

    _vm->bind__iter__(_vm->tp_range, [](VM* vm, PyObject* obj) { return VAR_T(RangeIter, PK_OBJ_GET(Range, obj)); });
    _vm->bind__repr__(_vm->_type("NoneType"), [](VM* vm, PyObject* obj) { return VAR("None"); });

    _vm->bind__truediv__(_vm->tp_float, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        f64 value = CAST_F(rhs);
        return VAR(_CAST(f64, lhs) / value);
    });

    _vm->bind__truediv__(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        f64 value = CAST_F(rhs);
        return VAR(_CAST(i64, lhs) / value);
    });

    auto py_number_pow = [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        i64 lhs, rhs;
        if(try_cast_int(lhs_, &lhs) && try_cast_int(rhs_, &rhs)){
            if(rhs < 0) {
                if(lhs == 0) vm->ZeroDivisionError("0.0 cannot be raised to a negative power");
                return VAR((f64)std::pow(lhs, rhs));
            }
            i64 ret = 1;
            while(rhs){
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            return VAR(ret);
        }else{
            return VAR((f64)std::pow(CAST_F(lhs_), CAST_F(rhs_)));
        }
    };

    _vm->bind__pow__(_vm->tp_int, py_number_pow);
    _vm->bind__pow__(_vm->tp_float, py_number_pow);

    /************ int ************/
    _vm->bind_constructor<-1>("int", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(0);
        // 1 arg
        if(args.size() == 1+1){
            if (is_type(args[1], vm->tp_float)) return VAR((i64)CAST(f64, args[1]));
            if (is_type(args[1], vm->tp_int)) return args[1];
            if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1 : 0);
        }
        if(args.size() > 1+2) vm->TypeError("int() takes at most 2 arguments");
        // 2 args
        if (is_type(args[1], vm->tp_str)) {
            int base = 10;
            if(args.size() == 1+2) base = CAST(i64, args[2]);
            const Str& s = CAST(Str&, args[1]);
            i64 val;
            if(!parse_int(s.sv(), &val, base)){
                vm->ValueError("invalid literal for int(): " + s.escape());
            }
            return VAR(val);
        }
        vm->TypeError("invalid arguments for int()");
        return vm->None;
    });

    _vm->bind_method<0>("int", "bit_length", [](VM* vm, ArgsView args) {
        i64 x = _CAST(i64, args[0]);
        if(x < 0) x = -x;
        int bits = 0;
        while(x){ x >>= 1; bits++; }
        return VAR(bits);
    });

    _vm->bind__floordiv__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        i64 rhs = CAST(i64, rhs_);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, lhs_) / rhs);
    });

    _vm->bind__mod__(_vm->tp_int, [](VM* vm, PyObject* lhs_, PyObject* rhs_) {
        i64 rhs = CAST(i64, rhs_);
        if(rhs == 0) vm->ZeroDivisionError();
        return VAR(_CAST(i64, lhs_) % rhs);
    });

    _vm->bind__repr__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(std::to_string(_CAST(i64, obj))); });

    _vm->bind__neg__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(-_CAST(i64, obj)); });

    _vm->bind__hash__(_vm->tp_int, [](VM* vm, PyObject* obj) { return _CAST(i64, obj); });

    _vm->bind__invert__(_vm->tp_int, [](VM* vm, PyObject* obj) { return VAR(~_CAST(i64, obj)); });

#define INT_BITWISE_OP(name, op) \
    _vm->bind##name(_vm->tp_int, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        return VAR(_CAST(i64, lhs) op CAST(i64, rhs)); \
    });

    INT_BITWISE_OP(__lshift__, <<)
    INT_BITWISE_OP(__rshift__, >>)
    INT_BITWISE_OP(__and__, &)
    INT_BITWISE_OP(__or__, |)
    INT_BITWISE_OP(__xor__, ^)

#undef INT_BITWISE_OP

    /************ float ************/
    _vm->bind_constructor<-1>("float", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(0.0);
        if(args.size() > 1+1) vm->TypeError("float() takes at most 1 argument");
        // 1 arg
        if (is_type(args[1], vm->tp_int)) return VAR((f64)CAST(i64, args[1]));
        if (is_type(args[1], vm->tp_float)) return args[1];
        if (is_type(args[1], vm->tp_bool)) return VAR(_CAST(bool, args[1]) ? 1.0 : 0.0);
        if (is_type(args[1], vm->tp_str)) {
            const Str& s = CAST(Str&, args[1]);
            if(s == "inf") return VAR(INFINITY);
            if(s == "-inf") return VAR(-INFINITY);

            double float_out;
            char* p_end;
            try{
                float_out = std::strtod(s.data, &p_end);
                PK_ASSERT(p_end == s.end());
            }catch(...){
                vm->ValueError("invalid literal for float(): " + s.escape());
            }
            return VAR(float_out);
        }
        vm->TypeError("invalid arguments for float()");
        return vm->None;
    });

    _vm->bind__hash__(_vm->tp_float, [](VM* vm, PyObject* obj) {
        f64 val = _CAST(f64, obj);
        return (i64)std::hash<f64>()(val);
    });

    _vm->bind__neg__(_vm->tp_float, [](VM* vm, PyObject* obj) { return VAR(-_CAST(f64, obj)); });

    _vm->bind__repr__(_vm->tp_float, [](VM* vm, PyObject* obj) {
        f64 val = _CAST(f64, obj);
        SStream ss;
        ss << val;
        return VAR(ss.str());
    });

    /************ str ************/
    _vm->bind_constructor<2>("str", PK_LAMBDA(vm->py_str(args[1])));

    _vm->bind__hash__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Str&, obj).hash();
    });

    _vm->bind__add__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(Str&, lhs) + CAST(Str&, rhs));
    });
    _vm->bind__len__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Str&, obj).u8_length();
    });
    _vm->bind__mul__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Str& self = _CAST(Str&, lhs);
        i64 n = CAST(i64, rhs);
        SStream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });

    _vm->bind_method<1>("str", "__rmul__", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        i64 n = CAST(i64, args[1]);
        SStream ss;
        for(i64 i = 0; i < n; i++) ss << self.sv();
        return VAR(ss.str());
    });

    _vm->bind__contains__(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Str& self = _CAST(Str&, lhs);
        return VAR(self.index(CAST(Str&, rhs)) != -1);
    });
    _vm->bind__str__(_vm->tp_str, [](VM* vm, PyObject* obj) { return obj; });
    _vm->bind__iter__(_vm->tp_str, [](VM* vm, PyObject* obj) { return VAR_T(StringIter, obj); });
    _vm->bind__repr__(_vm->tp_str, [](VM* vm, PyObject* obj) {
        const Str& self = _CAST(Str&, obj);
        return VAR(self.escape(true));
    });

#define BIND_CMP_STR(name, op) \
    _vm->bind##name(_vm->tp_str, [](VM* vm, PyObject* lhs, PyObject* rhs) { \
        if(!is_non_tagged_type(rhs, vm->tp_str)) return vm->NotImplemented; \
        return VAR(_CAST(Str&, lhs) op _CAST(Str&, rhs));                   \
    });

    BIND_CMP_STR(__eq__, ==)
    BIND_CMP_STR(__lt__, <)
    BIND_CMP_STR(__le__, <=)
    BIND_CMP_STR(__gt__, >)
    BIND_CMP_STR(__ge__, >=)
#undef BIND_CMP_STR

    _vm->bind__getitem__(_vm->tp_str, [](VM* vm, PyObject* obj, PyObject* index) {
        const Str& self = _CAST(Str&, obj);
        if(is_non_tagged_type(index, vm->tp_slice)){
            const Slice& s = _CAST(Slice&, index);
            int start, stop, step;
            vm->parse_int_slice(s, self.u8_length(), start, stop, step);
            return VAR(self.u8_slice(start, stop, step));
        }
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.u8_length());
        return VAR(self.u8_getitem(i));
    });

    _vm->bind(_vm->_t(_vm->tp_str), "replace(self, old, new, count=-1)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& old = CAST(Str&, args[1]);
        if(old.empty()) vm->ValueError("empty substring");
        const Str& new_ = CAST(Str&, args[2]);
        int count = CAST(int, args[3]);
        return VAR(self.replace(old, new_, count));
    });

    _vm->bind(_vm->_t(_vm->tp_str), "split(self, sep=' ')", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sep = CAST(Str&, args[1]);
        if(sep.empty()) vm->ValueError("empty separator");
        std::vector<std::string_view> parts;
        if(sep.size == 1){
            parts = self.split(sep[0]);
        }else{
            parts = self.split(sep);
        }
        List ret(parts.size());
        for(int i=0; i<parts.size(); i++) ret[i] = VAR(Str(parts[i]));
        return VAR(std::move(ret));
    });

    _vm->bind(_vm->_t(_vm->tp_str), "count(self, s: str)", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& s = CAST(Str&, args[1]);
        return VAR(self.count(s));
    });

    _vm->bind_method<1>("str", "index", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        int index = self.index(sub);
        if(index == -1) vm->ValueError("substring not found");
        return VAR(index);
    });

    _vm->bind_method<1>("str", "find", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        const Str& sub = CAST(Str&, args[1]);
        return VAR(self.index(sub));
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
        unsigned char* buffer = new unsigned char[self.length()];
        memcpy(buffer, self.data, self.length());
        return VAR(Bytes(buffer, self.length()));
    });

    _vm->bind_method<1>("str", "join", [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        const Str& self = _CAST(Str&, args[0]);
        SStream ss;
        PyObject* it = vm->py_iter(args[1]);     // strong ref
        PyObject* obj = vm->py_next(it);
        while(obj != vm->StopIteration){
            if(!ss.empty()) ss << self;
            ss << CAST(Str&, obj);
            obj = vm->py_next(it);
        }
        return VAR(ss.str());
    });

    _vm->bind_method<0>("str", "lower", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.lower());
    });

    _vm->bind_method<0>("str", "upper", [](VM* vm, ArgsView args) {
        const Str& self = _CAST(Str&, args[0]);
        return VAR(self.upper());
    });

    /************ list ************/
    _vm->bind(_vm->_t(_vm->tp_list), "sort(self, key=None, reverse=False)", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* key = args[1];
        if(key == vm->None){
            std::stable_sort(self.begin(), self.end(), [vm](PyObject* a, PyObject* b){
                return vm->py_lt(a, b);
            });
        }else{
            std::stable_sort(self.begin(), self.end(), [vm, key](PyObject* a, PyObject* b){
                return vm->py_lt(vm->call(key, a), vm->call(key, b));
            });
        }
        bool reverse = CAST(bool, args[2]);
        if(reverse) self.reverse();
        return vm->None;
    });

    _vm->bind__repr__(_vm->tp_list, [](VM* vm, PyObject* _0){
        List& iterable = _CAST(List&, _0);
        SStream ss;
        ss << '[';
        for(int i=0; i<iterable.size(); i++){
            ss << CAST(Str&, vm->py_repr(iterable[i]));
            if(i != iterable.size()-1) ss << ", ";
        }
        ss << ']';
        return VAR(ss.str());
    });

    _vm->bind__repr__(_vm->tp_tuple, [](VM* vm, PyObject* _0){
        Tuple& iterable = _CAST(Tuple&, _0);
        SStream ss;
        ss << '(';
        if(iterable.size() == 1){
            ss << CAST(Str&, vm->py_repr(iterable[0]));
            ss << ',';
        }else{
            for(int i=0; i<iterable.size(); i++){
                ss << CAST(Str&, vm->py_repr(iterable[i]));
                if(i != iterable.size()-1) ss << ", ";
            }
        }
        ss << ')';
        return VAR(ss.str());
    });

    _vm->bind_constructor<-1>("list", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(List());
        if(args.size() == 1+1){
            return vm->py_list(args[1]);
        }
        vm->TypeError("list() takes 0 or 1 arguments");
        return vm->None;
    });

    _vm->bind__contains__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* item) {
        List& self = _CAST(List&, obj);
        for(PyObject* i: self) if(vm->py_eq(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>("list", "count", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        List& a = _CAST(List&, lhs);
        if(!is_non_tagged_type(rhs, vm->tp_list)) return vm->NotImplemented;
        List& b = _CAST(List&, rhs);
        if(a.size() != b.size()) return vm->False;
        for(int i=0; i<a.size(); i++){
            if(!vm->py_eq(a[i], b[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind_method<1>("list", "index", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_eq(self[i], obj)) return VAR(i);
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<1>("list", "remove", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        PyObject* obj = args[1];
        for(int i=0; i<self.size(); i++){
            if(vm->py_eq(self[i], obj)){
                self.erase(i);
                return vm->None;
            }
        }
        vm->ValueError(_CAST(Str&, vm->py_repr(obj)) + " is not in list");
        return vm->None;
    });

    _vm->bind_method<-1>("list", "pop", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        if(args.size() == 1+0){
            if(self.empty()) vm->IndexError("pop from empty list");
            return self.popx_back();
        }
        if(args.size() == 1+1){
            int index = CAST(int, args[1]);
            index = vm->normalized_index(index, self.size());
            PyObject* ret = self[index];
            self.erase(index);
            return ret;
        }
        vm->TypeError("pop() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind_method<1>("list", "append", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        self.push_back(args[1]);
        return vm->None;
    });

    _vm->bind_method<1>("list", "extend", [](VM* vm, ArgsView args) {
        auto _lock = vm->heap.gc_scope_lock();
        List& self = _CAST(List&, args[0]);
        PyObject* it = vm->py_iter(args[1]);     // strong ref
        PyObject* obj = vm->py_next(it);
        while(obj != vm->StopIteration){
            self.push_back(obj);
            obj = vm->py_next(it);
        }
        return vm->None;
    });

    _vm->bind_method<0>("list", "reverse", [](VM* vm, ArgsView args) {
        List& self = _CAST(List&, args[0]);
        std::reverse(self.begin(), self.end());
        return vm->None;
    });

    _vm->bind__mul__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const List& self = _CAST(List&, lhs);
        if(!is_int(rhs)) return vm->NotImplemented;
        int n = _CAST(int, rhs);
        List result;
        result.reserve(self.size() * n);
        for(int i = 0; i < n; i++) result.extend(self);
        return VAR(std::move(result));
    });
    _vm->bind_method<1>("list", "__rmul__", [](VM* vm, ArgsView args) {
        const List& self = _CAST(List&, args[0]);
        if(!is_int(args[1])) return vm->NotImplemented;
        int n = _CAST(int, args[1]);
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

    _vm->bind_method<0>("list", "copy", PK_LAMBDA(VAR(_CAST(List, args[0]))));

#define BIND_RICH_CMP(name, op, _t, _T)    \
    _vm->bind__##name##__(_vm->_t, [](VM* vm, PyObject* lhs, PyObject* rhs){        \
        if(!is_non_tagged_type(rhs, vm->_t)) return vm->NotImplemented;             \
        auto& a = _CAST(_T&, lhs);                                                  \
        auto& b = _CAST(_T&, rhs);                                                  \
        for(int i=0; i<a.size() && i<b.size(); i++){                                \
            if(vm->py_eq(a[i], b[i])) continue;                                     \
            return VAR(vm->py_##name(a[i], b[i]));                                  \
        }                                                                           \
        return VAR(a.size() op b.size());                                           \
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

    _vm->bind__add__(_vm->tp_list, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const List& self = _CAST(List&, lhs);
        const List& other = CAST(List&, rhs);
        List new_list(self);    // copy construct
        new_list.extend(other);
        return VAR(std::move(new_list));
    });

    _vm->bind__len__(_vm->tp_list, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(List&, obj).size();
    });
    _vm->bind__iter__(_vm->tp_list, [](VM* vm, PyObject* obj) {
        List& self = _CAST(List&, obj);
        return VAR_T(ArrayIter, obj, self.begin(), self.end());
    });
    _vm->bind__getitem__(_vm->tp_list, PyArrayGetItem<List>);
    _vm->bind__setitem__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){
        List& self = _CAST(List&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        self[i] = value;
    });
    _vm->bind__delitem__(_vm->tp_list, [](VM* vm, PyObject* obj, PyObject* index){
        List& self = _CAST(List&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        self.erase(i);
    });

    /************ tuple ************/
    _vm->bind_constructor<-1>("tuple", [](VM* vm, ArgsView args) {
        if(args.size() == 1+0) return VAR(Tuple(0));
        if(args.size() == 1+1){
            List list = CAST(List, vm->py_list(args[1]));
            return VAR(Tuple(std::move(list)));
        }
        vm->TypeError("tuple() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__contains__(_vm->tp_tuple, [](VM* vm, PyObject* obj, PyObject* item) {
        Tuple& self = _CAST(Tuple&, obj);
        for(PyObject* i: self) if(vm->py_eq(i, item)) return vm->True;
        return vm->False;
    });

    _vm->bind_method<1>("tuple", "count", [](VM* vm, ArgsView args) {
        Tuple& self = _CAST(Tuple&, args[0]);
        int count = 0;
        for(PyObject* i: self) if(vm->py_eq(i, args[1])) count++;
        return VAR(count);
    });

    _vm->bind__eq__(_vm->tp_tuple, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        const Tuple& self = _CAST(Tuple&, lhs);
        if(!is_non_tagged_type(rhs, vm->tp_tuple)) return vm->NotImplemented;
        const Tuple& other = _CAST(Tuple&, rhs);
        if(self.size() != other.size()) return vm->False;
        for(int i = 0; i < self.size(); i++) {
            if(!vm->py_eq(self[i], other[i])) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__hash__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        i64 x = 1000003;
        const Tuple& items = CAST(Tuple&, obj);
        for (int i=0; i<items.size(); i++) {
            i64 y = vm->py_hash(items[i]);
            // recommended by Github Copilot
            x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
        }
        return x;
    });

    _vm->bind__iter__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        Tuple& self = _CAST(Tuple&, obj);
        return VAR_T(ArrayIter, obj, self.begin(), self.end());
    });
    _vm->bind__getitem__(_vm->tp_tuple, PyArrayGetItem<Tuple>);
    _vm->bind__len__(_vm->tp_tuple, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Tuple&, obj).size();
    });

    /************ bool ************/
    _vm->bind_constructor<2>("bool", PK_LAMBDA(VAR(vm->py_bool(args[1]))));
    _vm->bind__hash__(_vm->tp_bool, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(bool, obj);
    });
    _vm->bind__repr__(_vm->tp_bool, [](VM* vm, PyObject* self) {
        bool val = _CAST(bool, self);
        return VAR(val ? "True" : "False");
    });

    _vm->bind__and__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) && CAST(bool, rhs));
    });
    _vm->bind__or__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) || CAST(bool, rhs));
    });
    _vm->bind__xor__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        return VAR(_CAST(bool, lhs) != CAST(bool, rhs));
    });
    _vm->bind__eq__(_vm->tp_bool, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        if(is_non_tagged_type(rhs, vm->tp_bool)) return VAR(lhs == rhs);
        if(is_int(rhs)) return VAR(_CAST(bool, lhs) == (bool)CAST(i64, rhs));
        return vm->NotImplemented;
    });
    _vm->bind__repr__(_vm->_type("ellipsis"), [](VM* vm, PyObject* self) {
        return VAR("...");
    });
    _vm->bind__repr__(_vm->_type("NotImplementedType"), [](VM* vm, PyObject* self) {
        return VAR("NotImplemented");
    });

    /************ bytes ************/
    _vm->bind_constructor<2>("bytes", [](VM* vm, ArgsView args){
        List& list = CAST(List&, args[1]);
        std::vector<unsigned char> buffer(list.size());
        for(int i=0; i<list.size(); i++){
            i64 b = CAST(i64, list[i]);
            if(b<0 || b>255) vm->ValueError("byte must be in range[0, 256)");
            buffer[i] = (char)b;
        }
        return VAR(Bytes(buffer));
    });

    _vm->bind__getitem__(_vm->tp_bytes, [](VM* vm, PyObject* obj, PyObject* index) {
        const Bytes& self = _CAST(Bytes&, obj);
        int i = CAST(int, index);
        i = vm->normalized_index(i, self.size());
        return VAR(self[i]);
    });

    _vm->bind__hash__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        const Bytes& self = _CAST(Bytes&, obj);
        std::string_view view((char*)self.data(), self.size());
        return (i64)std::hash<std::string_view>()(view);
    });

    _vm->bind__repr__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        const Bytes& self = _CAST(Bytes&, obj);
        SStream ss;
        ss << "b'";
        for(int i=0; i<self.size(); i++){
            ss << "\\x"; // << std::hex << std::setw(2) << std::setfill('0') << self[i];
            ss << "0123456789ABCDEF"[self[i] >> 4];
            ss << "0123456789ABCDEF"[self[i] & 0xf];
        }
        ss << "'";
        return VAR(ss.str());
    });
    _vm->bind__len__(_vm->tp_bytes, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Bytes&, obj).size();
    });

    _vm->bind_method<0>("bytes", "decode", [](VM* vm, ArgsView args) {
        const Bytes& self = _CAST(Bytes&, args[0]);
        // TODO: check encoding is utf-8
        return VAR(Str(self.str()));
    });

    _vm->bind__eq__(_vm->tp_bytes, [](VM* vm, PyObject* lhs, PyObject* rhs) {
        if(!is_non_tagged_type(rhs, vm->tp_bytes)) return vm->NotImplemented;
        return VAR(_CAST(Bytes&, lhs) == _CAST(Bytes&, rhs));
    });
    /************ slice ************/
    _vm->bind_constructor<4>("slice", [](VM* vm, ArgsView args) {
        return VAR(Slice(args[1], args[2], args[3]));
    });

    _vm->bind__repr__(_vm->tp_slice, [](VM* vm, PyObject* obj) {
        const Slice& self = _CAST(Slice&, obj);
        SStream ss;
        ss << "slice(";
        ss << CAST(Str, vm->py_repr(self.start)) << ", ";
        ss << CAST(Str, vm->py_repr(self.stop)) << ", ";
        ss << CAST(Str, vm->py_repr(self.step)) << ")";
        return VAR(ss.str());
    });

    /************ mappingproxy ************/
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

    _vm->bind__len__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(MappingProxy&, obj).attr().size();
    });

    _vm->bind__eq__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj, PyObject* other){
        MappingProxy& a = _CAST(MappingProxy&, obj);
        if(!is_non_tagged_type(other, vm->tp_mappingproxy)){
            return vm->NotImplemented;
        }
        MappingProxy& b = _CAST(MappingProxy&, other);
        return VAR(a.obj == b.obj);
    });

    _vm->bind__getitem__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj, PyObject* index) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        StrName key = CAST(Str&, index);
        PyObject* ret = self.attr().try_get(key);
        if(ret == nullptr) vm->AttributeError(key.sv());
        return ret;
    });

    _vm->bind__repr__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        SStream ss;
        ss << "mappingproxy({";
        bool first = true;
        for(auto& item : self.attr().items()){
            if(!first) ss << ", ";
            first = false;
            ss << item.first.escape() << ": " << CAST(Str, vm->py_repr(item.second));
        }
        ss << "})";
        return VAR(ss.str());
    });

    _vm->bind__contains__(_vm->tp_mappingproxy, [](VM* vm, PyObject* obj, PyObject* key) {
        MappingProxy& self = _CAST(MappingProxy&, obj);
        return VAR(self.attr().contains(CAST(Str&, key)));
    });

    /************ dict ************/
    _vm->bind_constructor<-1>("dict", [](VM* vm, ArgsView args){
        return VAR(Dict(vm));
    });

    _vm->bind_method<-1>("dict", "__init__", [](VM* vm, ArgsView args){
        if(args.size() == 1+0) return vm->None;
        if(args.size() == 1+1){
            auto _lock = vm->heap.gc_scope_lock();
            Dict& self = _CAST(Dict&, args[0]);
            List& list = CAST(List&, args[1]);
            for(PyObject* item : list){
                Tuple& t = CAST(Tuple&, item);
                if(t.size() != 2){
                    vm->ValueError("dict() takes an iterable of tuples (key, value)");
                    return vm->None;
                }
                self.set(t[0], t[1]);
            }
            return vm->None;
        }
        vm->TypeError("dict() takes at most 1 argument");
        return vm->None;
    });

    _vm->bind__len__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        return (i64)_CAST(Dict&, obj).size();
    });

    _vm->bind__getitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* index) {
        Dict& self = _CAST(Dict&, obj);
        PyObject* ret = self.try_get(index);
        if(ret == nullptr) vm->KeyError(index);
        return ret;
    });

    _vm->bind__setitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key, PyObject* value) {
        Dict& self = _CAST(Dict&, obj);
        self.set(key, value);
    });

    _vm->bind__delitem__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key) {
        Dict& self = _CAST(Dict&, obj);
        bool ok = self.erase(key);
        if(!ok) vm->KeyError(key);
    });

    _vm->bind_method<-1>("dict", "pop", [](VM* vm, ArgsView args) {
        if(args.size() != 2 && args.size() != 3){
            vm->TypeError("pop() expected 1 or 2 arguments");
            return vm->None;
        }
        Dict& self = _CAST(Dict&, args[0]);
        PyObject* value = self.try_get(args[1]);
        if(value == nullptr){
            if(args.size() == 2) vm->KeyError(args[1]);
            if(args.size() == 3){
                return args[2];
            }
        }
        self.erase(args[1]);
        return value;
    });

    // _vm->bind_method<0>("dict", "_data", [](VM* vm, ArgsView args) {
    //     Dict& self = _CAST(Dict&, args[0]);
    //     SStream ss;
    //     ss << "[\n";
    //     for(int i=0; i<self._capacity; i++){
    //         auto item = self._items[i];
    //         Str key("None");
    //         Str value("None");
    //         if(item.first != nullptr){
    //             key = CAST(Str&, vm->py_repr(item.first));
    //         }
    //         if(item.second != nullptr){
    //             value = CAST(Str&, vm->py_repr(item.second));
    //         }
    //         int prev = self._nodes[i].prev;
    //         int next = self._nodes[i].next;
    //         ss << "  [" << key << ", " << value << ", " << prev << ", " << next << "],\n";
    //     }
    //     ss << "]\n";
    //     vm->stdout_write(ss.str());
    //     return vm->None;
    // });

    _vm->bind__contains__(_vm->tp_dict, [](VM* vm, PyObject* obj, PyObject* key) {
        Dict& self = _CAST(Dict&, obj);
        return VAR(self.contains(key));
    });

    _vm->bind__iter__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        const Dict& self = _CAST(Dict&, obj);
        return vm->py_iter(VAR(self.keys()));
    });

    _vm->bind_method<-1>("dict", "get", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        if(args.size() == 1+1){
            PyObject* ret = self.try_get(args[1]);
            if(ret != nullptr) return ret;
            return vm->None;
        }else if(args.size() == 1+2){
            PyObject* ret = self.try_get(args[1]);
            if(ret != nullptr) return ret;
            return args[2];
        }
        vm->TypeError("get() takes at most 2 arguments");
        return vm->None;
    });

    _vm->bind_method<0>("dict", "keys", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.keys());
    });

    _vm->bind_method<0>("dict", "values", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self.values());
    });

    _vm->bind_method<0>("dict", "items", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        Tuple items(self.size());
        int j = 0;
        self.apply([&](PyObject* k, PyObject* v){
            items[j++] = VAR(Tuple({k, v}));
        });
        return VAR(std::move(items));
    });

    _vm->bind_method<1>("dict", "update", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        const Dict& other = CAST(Dict&, args[1]);
        self.update(other);
        return vm->None;
    });

    _vm->bind_method<0>("dict", "copy", [](VM* vm, ArgsView args) {
        const Dict& self = _CAST(Dict&, args[0]);
        return VAR(self);
    });

    _vm->bind_method<0>("dict", "clear", [](VM* vm, ArgsView args) {
        Dict& self = _CAST(Dict&, args[0]);
        self.clear();
        return vm->None;
    });

    _vm->bind__repr__(_vm->tp_dict, [](VM* vm, PyObject* obj) {
        Dict& self = _CAST(Dict&, obj);
        SStream ss;
        ss << "{";
        bool first = true;

        self.apply([&](PyObject* k, PyObject* v){
            if(!first) ss << ", ";
            first = false;
            Str key = CAST(Str&, vm->py_repr(k));
            Str value = CAST(Str&, vm->py_repr(v));
            ss << key << ": " << value;
        });

        ss << "}";
        return VAR(ss.str());
    });

    _vm->bind__eq__(_vm->tp_dict, [](VM* vm, PyObject* a, PyObject* b) {
        Dict& self = _CAST(Dict&, a);
        if(!is_non_tagged_type(b, vm->tp_dict)) return vm->NotImplemented;
        Dict& other = _CAST(Dict&, b);
        if(self.size() != other.size()) return vm->False;
        for(int i=0; i<self._capacity; i++){
            auto item = self._items[i];
            if(item.first == nullptr) continue;
            PyObject* value = other.try_get(item.first);
            if(value == nullptr) return vm->False;
            if(!vm->py_eq(item.second, value)) return vm->False;
        }
        return vm->True;
    });

    _vm->bind__repr__(_vm->tp_module, [](VM* vm, PyObject* obj) {
        const Str& path = CAST(Str&, obj->attr(__path__));
        return VAR(fmt("<module ", path.escape(), ">"));
    });

    /************ property ************/
    _vm->bind_constructor<-1>("property", [](VM* vm, ArgsView args) {
        if(args.size() == 1+1){
            return VAR(Property(args[1], vm->None, ""));
        }else if(args.size() == 1+2){
            return VAR(Property(args[1], args[2], ""));
        }else if(args.size() == 1+3){
            return VAR(Property(args[1], args[2], CAST(Str, args[3])));
        }
        vm->TypeError("property() takes at most 3 arguments");
        return vm->None;
    });

    _vm->bind_property(_vm->_t(_vm->tp_property), "__signature__", [](VM* vm, ArgsView args){
        Property& self = _CAST(Property&, args[0]);
        return VAR(self.signature);
    });
    
    _vm->bind_property(_vm->_t(_vm->tp_function), "__doc__", [](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->docstring);
    });

    _vm->bind_property(_vm->_t(_vm->tp_native_func), "__doc__", [](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->docstring);
        return VAR("");
    });

    _vm->bind_property(_vm->_t(_vm->tp_function), "__signature__", [](VM* vm, ArgsView args) {
        Function& func = _CAST(Function&, args[0]);
        return VAR(func.decl->signature);
    });

    // _vm->bind_property(_vm->_t(_vm->tp_function), "__call__", [](VM* vm, ArgsView args) {
    //     return args[0];
    // });

    _vm->bind_property(_vm->_t(_vm->tp_native_func), "__signature__", [](VM* vm, ArgsView args) {
        NativeFunc& func = _CAST(NativeFunc&, args[0]);
        if(func.decl != nullptr) return VAR(func.decl->signature);
        return VAR("");
    });

    // _vm->bind_property(_vm->_t(_vm->tp_native_func), "__call__", [](VM* vm, ArgsView args) {
    //     return args[0];
    // });

    RangeIter::register_class(_vm, _vm->builtins);
    ArrayIter::register_class(_vm, _vm->builtins);
    StringIter::register_class(_vm, _vm->builtins);
    Generator::register_class(_vm, _vm->builtins);
}


void add_module_timeit(VM* vm){
    PyObject* mod = vm->new_module("timeit");
    vm->bind_func<2>(mod, "timeit", [](VM* vm, ArgsView args) {
        PyObject* f = args[0];
        i64 iters = CAST(i64, args[1]);
        auto now = std::chrono::system_clock::now();
        for(i64 i=0; i<iters; i++) vm->call(f);
        auto end = std::chrono::system_clock::now();
        f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count() / 1000.0;
        return VAR(elapsed);
    });
}

void add_module_operator(VM* vm){
    PyObject* mod = vm->new_module("operator");
    vm->bind_func<2>(mod, "lt", [](VM* vm, ArgsView args) { return VAR(vm->py_lt(args[0], args[1]));});
    vm->bind_func<2>(mod, "le", [](VM* vm, ArgsView args) { return VAR(vm->py_le(args[0], args[1]));});
    vm->bind_func<2>(mod, "eq", [](VM* vm, ArgsView args) { return VAR(vm->py_eq(args[0], args[1]));});
    vm->bind_func<2>(mod, "ne", [](VM* vm, ArgsView args) { return VAR(vm->py_ne(args[0], args[1]));});
    vm->bind_func<2>(mod, "ge", [](VM* vm, ArgsView args) { return VAR(vm->py_ge(args[0], args[1]));});
    vm->bind_func<2>(mod, "gt", [](VM* vm, ArgsView args) { return VAR(vm->py_gt(args[0], args[1]));});
}

struct PyStructTime{
    PY_CLASS(PyStructTime, time, struct_time)

    int tm_year;
    int tm_mon;
    int tm_mday;
    int tm_hour;
    int tm_min;
    int tm_sec;
    int tm_wday;
    int tm_yday;
    int tm_isdst;

    PyStructTime(std::time_t t){
        std::tm* tm = std::localtime(&t);
        tm_year = tm->tm_year + 1900;
        tm_mon = tm->tm_mon + 1;
        tm_mday = tm->tm_mday;
        tm_hour = tm->tm_hour;
        tm_min = tm->tm_min;
        tm_sec = tm->tm_sec;
        tm_wday = (tm->tm_wday + 6) % 7;
        tm_yday = tm->tm_yday + 1;
        tm_isdst = tm->tm_isdst;
    }

    PyStructTime* _() { return this; }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyStructTime>(type);
        PY_READONLY_FIELD(PyStructTime, "tm_year", _, tm_year);
        PY_READONLY_FIELD(PyStructTime, "tm_mon", _, tm_mon);
        PY_READONLY_FIELD(PyStructTime, "tm_mday", _, tm_mday);
        PY_READONLY_FIELD(PyStructTime, "tm_hour", _, tm_hour);
        PY_READONLY_FIELD(PyStructTime, "tm_min", _, tm_min);
        PY_READONLY_FIELD(PyStructTime, "tm_sec", _, tm_sec);
        PY_READONLY_FIELD(PyStructTime, "tm_wday", _, tm_wday);
        PY_READONLY_FIELD(PyStructTime, "tm_yday", _, tm_yday);
        PY_READONLY_FIELD(PyStructTime, "tm_isdst", _, tm_isdst);
    }
};

void add_module_time(VM* vm){
    PyObject* mod = vm->new_module("time");
    PyStructTime::register_class(vm, mod);

    vm->bind_func<0>(mod, "time", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        return VAR(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 1000.0);
    });

    vm->bind_func<1>(mod, "sleep", [](VM* vm, ArgsView args) {
        f64 seconds = CAST_F(args[0]);
        auto begin = std::chrono::system_clock::now();
        while(true){
            auto now = std::chrono::system_clock::now();
            f64 elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() / 1000.0;
            if(elapsed >= seconds) break;
        }
        return vm->None;
    });

    vm->bind_func<0>(mod, "localtime", [](VM* vm, ArgsView args) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        return VAR_T(PyStructTime, t);
    });
}

void add_module_sys(VM* vm){
    PyObject* mod = vm->new_module("sys");
    vm->setattr(mod, "version", VAR(PK_VERSION));
    vm->setattr(mod, "platform", VAR(PK_SYS_PLATFORM));

    PyObject* stdout_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    PyObject* stderr_ = vm->heap.gcnew<DummyInstance>(vm->tp_object);
    vm->setattr(mod, "stdout", stdout_);
    vm->setattr(mod, "stderr", stderr_);

    vm->bind_func<1>(stdout_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->_stdout(vm, s.data, s.size);
        return vm->None;
    });

    vm->bind_func<1>(stderr_, "write", [](VM* vm, ArgsView args) {
        Str& s = CAST(Str&, args[0]);
        vm->_stderr(vm, s.data, s.size);
        return vm->None;
    });
}

void add_module_json(VM* vm){
    PyObject* mod = vm->new_module("json");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        std::string_view sv;
        if(is_non_tagged_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        CodeObject_ code = vm->compile(sv, "<json>", JSON_MODE);
        return vm->_exec(code, vm->top_frame()->_module);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
    });
}


// https://docs.python.org/3.5/library/math.html
void add_module_math(VM* vm){
    PyObject* mod = vm->new_module("math");
    mod->attr().set("pi", VAR(3.1415926535897932384));
    mod->attr().set("e" , VAR(2.7182818284590452354));
    mod->attr().set("inf", VAR(std::numeric_limits<double>::infinity()));
    mod->attr().set("nan", VAR(std::numeric_limits<double>::quiet_NaN()));

    vm->bind_func<1>(mod, "ceil", PK_LAMBDA(VAR((i64)std::ceil(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fabs", PK_LAMBDA(VAR(std::fabs(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "floor", PK_LAMBDA(VAR((i64)std::floor(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "fsum", [](VM* vm, ArgsView args) {
        List& list = CAST(List&, args[0]);
        double sum = 0;
        double c = 0;
        for(PyObject* arg : list){
            double x = CAST_F(arg);
            double y = x - c;
            double t = sum + y;
            c = (t - sum) - y;
            sum = t;
        }
        return VAR(sum);
    });
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

    vm->bind_func<1>(mod, "isfinite", PK_LAMBDA(VAR(std::isfinite(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isinf", PK_LAMBDA(VAR(std::isinf(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "isnan", PK_LAMBDA(VAR(std::isnan(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "exp", PK_LAMBDA(VAR(std::exp(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log", PK_LAMBDA(VAR(std::log(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log2", PK_LAMBDA(VAR(std::log2(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "log10", PK_LAMBDA(VAR(std::log10(CAST_F(args[0])))));

    vm->bind_func<2>(mod, "pow", PK_LAMBDA(VAR(std::pow(CAST_F(args[0]), CAST_F(args[1])))));
    vm->bind_func<1>(mod, "sqrt", PK_LAMBDA(VAR(std::sqrt(CAST_F(args[0])))));

    vm->bind_func<1>(mod, "acos", PK_LAMBDA(VAR(std::acos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "asin", PK_LAMBDA(VAR(std::asin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "atan", PK_LAMBDA(VAR(std::atan(CAST_F(args[0])))));
    vm->bind_func<2>(mod, "atan2", PK_LAMBDA(VAR(std::atan2(CAST_F(args[0]), CAST_F(args[1])))));

    vm->bind_func<1>(mod, "cos", PK_LAMBDA(VAR(std::cos(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "sin", PK_LAMBDA(VAR(std::sin(CAST_F(args[0])))));
    vm->bind_func<1>(mod, "tan", PK_LAMBDA(VAR(std::tan(CAST_F(args[0])))));
    
    vm->bind_func<1>(mod, "degrees", PK_LAMBDA(VAR(CAST_F(args[0]) * 180 / 3.1415926535897932384)));
    vm->bind_func<1>(mod, "radians", PK_LAMBDA(VAR(CAST_F(args[0]) * 3.1415926535897932384 / 180)));

    vm->bind_func<1>(mod, "modf", [](VM* vm, ArgsView args) {
        f64 i;
        f64 f = std::modf(CAST_F(args[0]), &i);
        return VAR(Tuple({VAR(f), VAR(i)}));
    });

    vm->bind_func<1>(mod, "factorial", [](VM* vm, ArgsView args) {
        i64 n = CAST(i64, args[0]);
        if(n < 0) vm->ValueError("factorial() not defined for negative values");
        i64 r = 1;
        for(i64 i=2; i<=n; i++) r *= i;
        return VAR(r);
    });
}

void add_module_traceback(VM* vm){
    PyObject* mod = vm->new_module("traceback");
    vm->bind_func<0>(mod, "print_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = CAST(Exception&, vm->_last_exception);
        Str sum = e.summary();
        vm->_stdout(vm, sum.data, sum.size);
        return vm->None;
    });

    vm->bind_func<0>(mod, "format_exc", [](VM* vm, ArgsView args) {
        if(vm->_last_exception==nullptr) vm->ValueError("no exception");
        Exception& e = CAST(Exception&, vm->_last_exception);
        return VAR(e.summary());
    });
}

void add_module_dis(VM* vm){
    PyObject* mod = vm->new_module("dis");

    static const auto get_code = [](VM* vm, PyObject* obj)->CodeObject_{
        if(is_type(obj, vm->tp_str)){
            const Str& source = CAST(Str, obj);
            return vm->compile(source, "<dis>", EXEC_MODE);
        }
        PyObject* f = obj;
        if(is_type(f, vm->tp_bound_method)) f = CAST(BoundMethod, obj).func;
        return CAST(Function&, f).decl->code;
    };

    vm->bind_func<1>(mod, "dis", [](VM* vm, ArgsView args) {
        CodeObject_ code = get_code(vm, args[0]);
        Str msg = vm->disassemble(code);
        vm->_stdout(vm, msg.data, msg.size);
        return vm->None;
    });

    // vm->bind_func<1>(mod, "_s", [](VM* vm, ArgsView args) {
    //     CodeObject_ code = get_code(vm, args[0]);
    //     return VAR(code->serialize(vm));
    // });
}

void add_module_gc(VM* vm){
    PyObject* mod = vm->new_module("gc");
    vm->bind_func<0>(mod, "collect", PK_LAMBDA(VAR(vm->heap.collect())));
}


void VM::post_init(){
    init_builtins(this);

    bind_method<1>("property", "setter", [](VM* vm, ArgsView args) {
        Property& self = _CAST(Property&, args[0]);
        // The setter's name is not necessary to be the same as the property's name
        // However, for cpython compatibility, we recommend to use the same name
        self.setter = args[1];
        return args[0];
    });

    // type
    bind__getitem__(tp_type, [](VM* vm, PyObject* self, PyObject* _){
        PK_UNUSED(_);
        return self;        // for generics
    });

    bind__repr__(tp_type, [](VM* vm, PyObject* self){
        SStream ss;
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, self)];
        ss << "<class '" << info.name << "'>";
        return VAR(ss.str());
    });

    bind_property(_t(tp_object), "__class__", PK_LAMBDA(vm->_t(args[0])));
    bind_property(_t(tp_type), "__base__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return info.base.index == -1 ? vm->None : vm->_all_types[info.base].obj;
    });
    bind_property(_t(tp_type), "__name__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        return VAR(info.name);
    });
    bind_property(_t(tp_type), "__module__", [](VM* vm, ArgsView args){
        const PyTypeInfo& info = vm->_all_types[PK_OBJ_GET(Type, args[0])];
        if(info.mod == nullptr) return vm->None;
        return info.mod;
    });
    bind_property(_t(tp_bound_method), "__self__", [](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).self;
    });
    bind_property(_t(tp_bound_method), "__func__", [](VM* vm, ArgsView args){
        return CAST(BoundMethod&, args[0]).func;
    });

    bind__eq__(tp_bound_method, [](VM* vm, PyObject* lhs, PyObject* rhs){
        if(!is_non_tagged_type(rhs, vm->tp_bound_method)) return vm->NotImplemented;
        return VAR(_CAST(BoundMethod&, lhs) == _CAST(BoundMethod&, rhs));
    });

    bind_property(_t(tp_slice), "start", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).start;
    });
    bind_property(_t(tp_slice), "stop", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).stop;
    });
    bind_property(_t(tp_slice), "step", [](VM* vm, ArgsView args){
        return CAST(Slice&, args[0]).step;
    });

    bind_property(_t(tp_object), "__dict__", [](VM* vm, ArgsView args){
        if(is_tagged(args[0]) || !args[0]->is_attr_valid()) return vm->None;
        return VAR(MappingProxy(args[0]));
    });

#if !PK_DEBUG_NO_BUILTINS
    add_module_sys(this);
    add_module_traceback(this);
    add_module_time(this);
    add_module_json(this);
    add_module_math(this);
    add_module_re(this);
    add_module_dis(this);
    add_module_c(this);
    add_module_gc(this);
    add_module_random(this);
    add_module_base64(this);
    add_module_timeit(this);
    add_module_operator(this);

    for(const char* name: {"this", "functools", "heapq", "bisect", "pickle", "_long", "colorsys", "typing", "datetime"}){
        _lazy_modules[name] = kPythonLibs[name];
    }

    try{
        CodeObject_ code = compile(kPythonLibs["builtins"], "<builtins>", EXEC_MODE);
        this->_exec(code, this->builtins);
        code = compile(kPythonLibs["_set"], "<set>", EXEC_MODE);
        this->_exec(code, this->builtins);
    }catch(Exception& e){
        std::cerr << e.summary() << std::endl;
        std::cerr << "failed to load builtins module!!" << std::endl;
        exit(1);
    }

    if(enable_os){
        add_module_io(this);
        add_module_os(this);
        _import_handler = _default_import_handler;
    }

    add_module_linalg(this);
    add_module_easing(this);
    add_module_collections(this);

#ifdef PK_USE_BOX2D
    add_module_box2d(this);
#endif
#ifdef PK_USE_CJSON
    add_module_cjson(this);
#endif

#endif
}

CodeObject_ VM::compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(Exception& e){
#if PK_DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e);
        return nullptr;
    }
}

}   // namespace pkpy