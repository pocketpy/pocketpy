#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    static Type _type(VM* vm) {                 \
        static const StrName __x0(#mod);        \
        static const StrName __x1(#name);       \
        return OBJ_GET(Type, vm->_modules[__x0]->attr(__x1));               \
    }                                                                       \
    static void _check_type(VM* vm, PyObject* val){                          \
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

struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    int base_offset;
    VoidP(void* ptr): ptr(ptr), base_offset(1){}
    VoidP(): ptr(nullptr), base_offset(1){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<VoidP>(type);

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            std::stringstream ss;
            ss << "<void* at " << self.ptr;
            if(self.base_offset != 1) ss << ", base_offset=" << self.base_offset;
            ss << ">";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            VoidP& other = CAST(VoidP&, args[1]);
            return VAR(self.ptr == other.ptr && self.base_offset == other.base_offset);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            VoidP& other = CAST(VoidP&, args[1]);
            return VAR(self.ptr != other.ptr || self.base_offset != other.base_offset);
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

        vm->bind_method<1>(type, "__add__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 offset = CAST(i64, args[1]);
            return VAR_T(VoidP, (char*)self.ptr + offset);
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 offset = CAST(i64, args[1]);
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

        vm->bind_method<0>(type, "read_void_p", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR_T(VoidP, *(void**)self.ptr);
        });
        vm->bind_method<1>(type, "write_void_p", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            VoidP& other = CAST(VoidP&, args[0]);
            self.ptr = other.ptr;
            return vm->None;
        });

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
};

struct C99Struct{
    PY_CLASS(C99Struct, c, struct)

    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    void _init(int size){
        this->size = size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
    }

    template<typename T>
    C99Struct(const T& data){
        static_assert(std::is_pod_v<T>);
        static_assert(!std::is_pointer_v<T>);
        _init(sizeof(T));
        memcpy(p, &data, this->size);
    }

    C99Struct() { p = _inlined; }
    C99Struct(void* p, int size){
        _init(size);
        if(p!=nullptr) memcpy(this->p, p, size);
    }
    ~C99Struct(){ if(p!=_inlined) free(p); }

    C99Struct(const C99Struct& other){
        _init(other.size);
        memcpy(p, other.p, size);
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<C99Struct>(type);

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


struct ReflType{
    std::string_view name;
    size_t size;
    std::map<std::string_view, int> fields;
};
inline static std::map<std::string_view, ReflType> _refl_types;

inline void add_refl_type(std::string_view name, size_t size, std::initializer_list<std::pair<std::string_view, int>> fields){
    ReflType type{name, size, {}};
    for(auto& [name, offset] : fields){
        type.fields[name] = offset;
    }
    _refl_types[name] = std::move(type);
}

inline static int c99_sizeof(VM* vm, const Str& type){
    static const std::map<std::string_view, i64> c99_sizes = {
        {"char", sizeof(char)},
        {"uchar", sizeof(unsigned char)},
        {"short", sizeof(short)},
        {"ushort", sizeof(unsigned short)},
        {"int", sizeof(int)},
        {"uint", sizeof(unsigned int)},
        {"long", sizeof(long)},
        {"ulong", sizeof(unsigned long)},
        {"longlong", sizeof(long long)},
        {"ulonglong", sizeof(unsigned long long)},
        {"float", sizeof(float)},
        {"double", sizeof(double)},
        {"bool", sizeof(bool)},
        {"void_p", sizeof(void*)}
    };
    auto it = c99_sizes.find(type.sv());
    if(it != c99_sizes.end()) return it->second;
    auto it2 = _refl_types.find(type.sv());
    if(it2 != _refl_types.end()) return it2->second.size;
    vm->ValueError("not a valid c99 type");
    return 0;
}

struct C99ReflType final: ReflType{
    PY_CLASS(C99ReflType, c, "_refl")

    C99ReflType(const ReflType& type){
        this->name = type.name;
        this->size = type.size;
        this->fields = type.fields;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<-1>(type, CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__call__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR_T(C99Struct, nullptr, self.size);
        });

        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.name);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<1>(type, "__getitem__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            const Str& name = CAST(Str&, args[1]);
            auto it = self.fields.find(name.sv());
            if(it == self.fields.end()){
                vm->_error("KeyError", name.escape());
                return vm->None;
            }
            return VAR(it->second);
        });
    }
};

static_assert(sizeof(Py_<C99Struct>) <= 64);

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
            vm->TypeError("expected " + std::to_string(n) + " arguments, but got " + std::to_string(args.size()));
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
    static const StrName m_proxy("__proxy__");
    NativeProxyFuncCBase* pf = CAST(NativeProxyFuncCBase*, args[-2]->attr(m_proxy));
    return (*pf)(vm, args);
}

template<typename T>
inline void bind_any_c_fp(VM* vm, PyObject* obj, Str name, T fp){
    static_assert(std::is_pod_v<T>);
    static_assert(std::is_pointer_v<T>);
    auto proxy = new NativeProxyFuncC(fp);
    PyObject* func = VAR(NativeFunc(_any_c_wrapper, proxy->N, false));
    func->attr().set("__proxy__", VAR_T(VoidP, proxy));
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
        i64 c = CAST(i64, args[1]);
        i64 size = CAST(i64, args[2]);
        memset(p, c, size);
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
}

}   // namespace pkpy