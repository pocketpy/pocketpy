#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    static Type _type(VM* vm) {                 \
        static const StrName __x0(#mod);        \
        static const StrName __x1(#name);       \
        return PK_OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static void _check_type(VM* vm, PyObject* val){                         \
        if(!vm->isinstance(val, T::_type(vm))){                             \
            vm->TypeError("expected '" #mod "." #name "', got " + OBJ_NAME(vm->_t(val)).escape());  \
        }                                                                   \
    }                                                                       \
    static PyObject* register_class(VM* vm, PyObject* mod) {                \
        if(OBJ_NAME(mod) != #mod) {                                         \
            auto msg = fmt("register_class() failed: ", OBJ_NAME(mod), " != ", #mod); \
            throw std::runtime_error(msg);                                  \
        }                                                                   \
        PyObject* type = vm->new_type_object(mod, #name, vm->tp_object);    \
        T::_register(vm, mod, type);                                        \
        type->attr()._try_perfect_rehash();                                 \
        return type;                                                        \
    }                                                                       

#define VAR_T(T, ...) vm->heap.gcnew<T>(T::_type(vm), T(__VA_ARGS__))

static int c99_sizeof(VM*, const Str&);

inline PyObject* py_var(VM* vm, void* p);
inline PyObject* py_var(VM* vm, char* p);

struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    int base_offset;
    VoidP(void* ptr): ptr(ptr), base_offset(1){}
    VoidP(): ptr(nullptr), base_offset(1){}

    bool operator==(const VoidP& other) const {
        return ptr == other.ptr && base_offset == other.base_offset;
    }
    bool operator!=(const VoidP& other) const {
        return ptr != other.ptr || base_offset != other.base_offset;
    }
    bool operator<(const VoidP& other) const { return ptr < other.ptr; }
    bool operator<=(const VoidP& other) const { return ptr <= other.ptr; }
    bool operator>(const VoidP& other) const { return ptr > other.ptr; }
    bool operator>=(const VoidP& other) const { return ptr >= other.ptr; }


    Str hex() const{
        std::stringstream ss;
        ss << std::hex << reinterpret_cast<intptr_t>(ptr);
        return "0x" + ss.str();
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<VoidP>(type);

        vm->bind_func<1>(type, "from_hex", [](VM* vm, ArgsView args){
            std::string s = CAST(Str&, args[0]).str();
            size_t size;
            intptr_t ptr = std::stoll(s, &size, 16);
            if(size != s.size()) vm->ValueError("invalid literal for void_p(): " + s);
            return VAR_T(VoidP, (void*)ptr);
        });
        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.hex());
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            std::stringstream ss;
            ss << "<void* at " << self.hex();
            if(self.base_offset != 1) ss << ", base_offset=" << self.base_offset;
            ss << ">";
            return VAR(ss.str());
        });

#define BIND_CMP(name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){       \
            if(!is_non_tagged_type(rhs, VoidP::_type(vm))) return vm->NotImplemented;       \
            return VAR(_CAST(VoidP&, lhs) op _CAST(VoidP&, rhs));                           \
        });

        BIND_CMP(__eq__, ==)
        BIND_CMP(__lt__, <)
        BIND_CMP(__le__, <=)
        BIND_CMP(__gt__, >)
        BIND_CMP(__ge__, >=)

#undef BIND_CMP

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            return reinterpret_cast<i64>(self.ptr);
        });

        vm->bind_method<1>(type, "set_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            if(is_non_tagged_type(args[1], vm->tp_str)){
                const Str& type = _CAST(Str&, args[1]);
                self.base_offset = c99_sizeof(vm, type);
            }else{
                self.base_offset = CAST(int, args[1]);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "get_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.base_offset);
        });

        vm->bind_method<1>(type, "offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 offset = CAST(i64, args[1]);
            return VAR_T(VoidP, (char*)self.ptr + offset * self.base_offset);
        });

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr + offset);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr - offset);
        });

#define BIND_SETGET(T, name) \
        vm->bind_method<0>(type, "read_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            return VAR(*(T*)self.ptr);                              \
        });                                                         \
        vm->bind_method<1>(type, "write_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            *(T*)self.ptr = CAST(T, args[1]);                       \
            return vm->None;                                        \
        });

        BIND_SETGET(char, "char")
        BIND_SETGET(unsigned char, "uchar")
        BIND_SETGET(short, "short")
        BIND_SETGET(unsigned short, "ushort")
        BIND_SETGET(int, "int")
        BIND_SETGET(unsigned int, "uint")
        BIND_SETGET(long, "long")
        BIND_SETGET(unsigned long, "ulong")
        BIND_SETGET(long long, "longlong")
        BIND_SETGET(unsigned long long, "ulonglong")
        BIND_SETGET(float, "float")
        BIND_SETGET(double, "double")
        BIND_SETGET(bool, "bool")
        BIND_SETGET(void*, "void_p")

        vm->bind_method<1>(type, "read_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 size = CAST(i64, args[1]);
            std::vector<char> buffer(size);
            memcpy(buffer.data(), self.ptr, size);
            return VAR(Bytes(std::move(buffer)));
        });

        vm->bind_method<1>(type, "write_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            Bytes& bytes = CAST(Bytes&, args[1]);
            memcpy(self.ptr, bytes.data(), bytes.size());
            return vm->None;
        });
    }

#undef BIND_SETGET
};

struct C99Struct{
    PY_CLASS(C99Struct, c, struct)

    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    C99Struct(int new_size){
        this->size = new_size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
    }

    template<typename T>
    C99Struct(const T& data): C99Struct(sizeof(T)){
        static_assert(std::is_pod_v<T>);
        static_assert(!std::is_pointer_v<T>);
        memcpy(p, &data, this->size);
    }

    C99Struct(void* p, int size): C99Struct(size){
        if(p != nullptr) memcpy(this->p, p, size);
    }

    C99Struct(const C99Struct& other): C99Struct(other.p, other.size){}

    ~C99Struct(){ if(p!=_inlined) free(p); }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+1){
                if(is_int(args[1])){
                    int size = _CAST(int, args[1]);
                    return VAR_T(C99Struct, size);
                }
                if(is_non_tagged_type(args[1], vm->tp_str)){
                    const Str& s = _CAST(Str&, args[1]);
                    return VAR_T(C99Struct, (void*)s.data, s.size);
                }
                if(is_non_tagged_type(args[1], vm->tp_bytes)){
                    const Bytes& b = _CAST(Bytes&, args[1]);
                    return VAR_T(C99Struct, (void*)b.data(), b.size());
                }
                vm->TypeError("expected int, str or bytes");
                return vm->None;
            }
            if(args.size() == 1+2){
                void* p = CAST(void*, args[1]);
                int size = CAST(int, args[2]);
                return VAR_T(C99Struct, p, size);
            }
            vm->TypeError("expected 1 or 2 arguments");
            return vm->None;
        });

        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(VoidP, self.p);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(C99Struct, self);
        });

        vm->bind_method<0>(type, "to_string", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(Str(self.p, self.size));
        });

        vm->bind_method<0>(type, "to_bytes", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            std::vector<char> buffer(self.size);
            memcpy(buffer.data(), self.p, self.size);
            return VAR(Bytes(std::move(buffer)));
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            C99Struct& self = _CAST(C99Struct&, lhs);
            if(!is_non_tagged_type(rhs, C99Struct::_type(vm))) return vm->NotImplemented;
            C99Struct& other = _CAST(C99Struct&, rhs);
            bool ok = self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
            return VAR(ok);
        });

#define BIND_SETGET(T, name) \
        vm->bind(type, "read_" name "(self, offset=0)", [](VM* vm, ArgsView args){          \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[1]);    \
            void* ptr = self.p + offset;    \
            return VAR(*(T*)ptr);   \
        }); \
        vm->bind(type, "write_" name "(self, value, offset=0)", [](VM* vm, ArgsView args){  \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[2]);    \
            void* ptr = self.p + offset;    \
            *(T*)ptr = CAST(T, args[1]);    \
            return vm->None;    \
        });

        BIND_SETGET(char, "char")
        BIND_SETGET(unsigned char, "uchar")
        BIND_SETGET(short, "short")
        BIND_SETGET(unsigned short, "ushort")
        BIND_SETGET(int, "int")
        BIND_SETGET(unsigned int, "uint")
        BIND_SETGET(long, "long")
        BIND_SETGET(unsigned long, "ulong")
        BIND_SETGET(long long, "longlong")
        BIND_SETGET(unsigned long long, "ulonglong")
        BIND_SETGET(float, "float")
        BIND_SETGET(double, "double")
        BIND_SETGET(bool, "bool")
        BIND_SETGET(void*, "void_p")
#undef BIND_SETGET

        // patch VoidP
        type = vm->_t(VoidP::_type(vm));

        vm->bind_method<1>(type, "read_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            const Str& type = CAST(Str&, args[1]);
            int size = c99_sizeof(vm, type);
            return VAR_T(C99Struct, self.ptr, size);
        });

        vm->bind_method<1>(type, "write_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            C99Struct& other = CAST(C99Struct&, args[1]);
            memcpy(self.ptr, other.p, other.size);
            return vm->None;
        });
    }
};

struct ReflField{
    std::string_view name;
    int offset;
    bool operator<(const ReflField& other) const{ return name < other.name; }
    bool operator==(const ReflField& other) const{ return name == other.name; }
    bool operator!=(const ReflField& other) const{ return name != other.name; }
    bool operator<(std::string_view other) const{ return name < other; }
    bool operator==(std::string_view other) const{ return name == other; }
    bool operator!=(std::string_view other) const{ return name != other; }
};

struct ReflType{
    std::string_view name;
    size_t size;
    std::vector<ReflField> fields;
};
inline static std::map<std::string_view, ReflType> _refl_types;

inline void add_refl_type(std::string_view name, size_t size, std::vector<ReflField> fields){
    ReflType type{name, size, std::move(fields)};
    std::sort(type.fields.begin(), type.fields.end());
    _refl_types[name] = std::move(type);
}

inline static int c99_sizeof(VM* vm, const Str& type){
    auto it = _refl_types.find(type.sv());
    if(it != _refl_types.end()) return it->second.size;
    vm->ValueError("not a valid c99 type");
    return 0;
}

struct C99ReflType final: ReflType{
    PY_CLASS(C99ReflType, c, _refl)

    C99ReflType(const ReflType& type){
        this->name = type.name;
        this->size = type.size;
        this->fields = type.fields;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<C99ReflType>(type);

        vm->bind_method<0>(type, "__call__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR_T(C99Struct, nullptr, self.size);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR("<ctype '" + Str(self.name) + "'>");
        });

        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.name);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.size);
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* key){
            C99ReflType& self = _CAST(C99ReflType&, obj);
            const Str& name = CAST(Str&, key);
            auto it = std::lower_bound(self.fields.begin(), self.fields.end(), name.sv());
            if(it == self.fields.end() || it->name != name.sv()){
                vm->KeyError(key);
                return vm->None;
            }
            return VAR(it->offset);
        });
    }
};

static_assert(sizeof(Py_<C99Struct>) <= 64);
static_assert(sizeof(Py_<Tuple>) <= 64);

inline PyObject* py_var(VM* vm, void* p){
    return VAR_T(VoidP, p);
}

inline PyObject* py_var(VM* vm, char* p){
    return VAR_T(VoidP, p);
}
/***********************************************/
template<typename T>
T to_void_p(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<T>);
    if(var == vm->None) return nullptr;     // None can be casted to any pointer implicitly
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<T>(p.ptr);
}

template<typename T>
T to_c99_struct(VM* vm, PyObject* var){
    static_assert(std::is_pod_v<T>);
    C99Struct& pod = CAST(C99Struct&, var);
    return *reinterpret_cast<T*>(pod.p);
}

template<typename T>
std::enable_if_t<std::is_pod_v<T> && !std::is_pointer_v<T>, PyObject*> py_var(VM* vm, const T& data){
    return VAR_T(C99Struct, data);
}
/*****************************************************************/
struct NativeProxyFuncCBase {
    virtual PyObject* operator()(VM* vm, ArgsView args) = 0;

    static void check_args_size(VM* vm, ArgsView args, int n){
        if (args.size() != n){
            vm->TypeError("expected " + std::to_string(n) + " arguments, got " + std::to_string(args.size()));
        }
    }
};

template<typename Ret, typename... Params>
struct NativeProxyFuncC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFuncC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        check_args_size(vm, args, N);
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    PyObject* call(VM* vm, ArgsView args, std::index_sequence<Is...>){
        if constexpr(std::is_void_v<__Ret>){
            func(py_cast<Params>(vm, args[Is])...);
            return vm->None;
        }else{
            __Ret ret = func(py_cast<Params>(vm, args[Is])...);
            return VAR(std::move(ret));
        }
    }
};

inline PyObject* _any_c_wrapper(VM* vm, ArgsView args){
    NativeProxyFuncCBase* pf = lambda_get_userdata<NativeProxyFuncCBase*>(args.begin());
    return (*pf)(vm, args);
}

template<typename T>
inline void bind_any_c_fp(VM* vm, PyObject* obj, Str name, T fp){
    static_assert(std::is_pod_v<T>);
    static_assert(std::is_pointer_v<T>);
    auto proxy = new NativeProxyFuncC(fp);
    PyObject* func = VAR(NativeFunc(_any_c_wrapper, proxy->N, false));
    _CAST(NativeFunc&, func).set_userdata(proxy);
    obj->attr().set(name, func);
}

inline void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    
    vm->bind_func<1>(mod, "malloc", [](VM* vm, ArgsView args){
        i64 size = CAST(i64, args[0]);
        return VAR(malloc(size));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        free(p);
        return vm->None;
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, ArgsView args){
        const Str& type = CAST(Str&, args[0]);
        i64 size = c99_sizeof(vm, type);
        return VAR(size);
    });

    vm->bind_func<1>(mod, "refl", [](VM* vm, ArgsView args){
        const Str& key = CAST(Str&, args[0]);
        auto it = _refl_types.find(key.sv());
        if(it == _refl_types.end()) vm->ValueError("reflection type not found");
        const ReflType& rt = it->second;
        return VAR_T(C99ReflType, rt);
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        memset(p, CAST(int, args[1]), CAST(size_t, args[2]));
        return vm->None;
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, ArgsView args){
        void* dst = CAST(void*, args[0]);
        void* src = CAST(void*, args[1]);
        i64 size = CAST(i64, args[2]);
        memcpy(dst, src, size);
        return vm->None;
    });

    VoidP::register_class(vm, mod);
    C99Struct::register_class(vm, mod);
    C99ReflType::register_class(vm, mod);
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    add_refl_type("char", sizeof(char), {});
    add_refl_type("uchar", sizeof(unsigned char), {});
    add_refl_type("short", sizeof(short), {});
    add_refl_type("ushort", sizeof(unsigned short), {});
    add_refl_type("int", sizeof(int), {});
    add_refl_type("uint", sizeof(unsigned int), {});
    add_refl_type("long", sizeof(long), {});
    add_refl_type("ulong", sizeof(unsigned long), {});
    add_refl_type("longlong", sizeof(long long), {});
    add_refl_type("ulonglong", sizeof(unsigned long long), {});
    add_refl_type("float", sizeof(float), {});
    add_refl_type("double", sizeof(double), {});
    add_refl_type("bool", sizeof(bool), {});
    add_refl_type("void_p", sizeof(void*), {});

    for(const char* t: {"char", "uchar", "short", "ushort", "int", "uint", "long", "ulong", "longlong", "ulonglong", "float", "double", "bool"}){
        mod->attr().set(Str(t) + "_", VAR_T(C99ReflType, _refl_types[t]));
    }
}

}   // namespace pkpy