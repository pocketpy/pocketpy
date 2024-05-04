#pragma once

#include "cffi.h"

namespace pkpy{

struct NativeProxyFuncCBase {
    virtual PyObject* operator()(VM* vm, ArgsView args) = 0;
};

template<typename Ret, typename... Params>
struct NativeProxyFuncC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFuncC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        PK_DEBUG_ASSERT(args.size() == N);
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

template<typename Ret, typename T, typename... Params>
struct NativeProxyMethodC final: NativeProxyFuncCBase {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(T::*)(Params...);
    _Fp func;
    NativeProxyMethodC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        PK_DEBUG_ASSERT(args.size() == N+1);
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    PyObject* call(VM* vm, ArgsView args, std::index_sequence<Is...>){
        T& self = PK_OBJ_GET(T, args[0]);   // use unsafe cast for derived classes
        if constexpr(std::is_void_v<__Ret>){
            (self.*func)(py_cast<Params>(vm, args[Is+1])...);
            return vm->None;
        }else{
            __Ret ret = (self.*func)(py_cast<Params>(vm, args[Is+1])...);
            return VAR(std::move(ret));
        }
    }
};

inline PyObject* proxy_wrapper(VM* vm, ArgsView args){
    NativeProxyFuncCBase* pf = lambda_get_userdata<NativeProxyFuncCBase*>(args.begin());
    return (*pf)(vm, args);
}

template<typename Ret, typename... Params>
void _bind(VM* vm, PyObject* obj, const char* sig, Ret(*func)(Params...)){
    auto proxy = new NativeProxyFuncC<Ret, Params...>(func);
    vm->bind(obj, sig, proxy_wrapper, proxy);
}

template<typename Ret, typename T, typename... Params>
void _bind(VM* vm, PyObject* obj, const char* sig, Ret(T::*func)(Params...)){
    auto proxy = new NativeProxyMethodC<Ret, T, Params...>(func);
    vm->bind(obj, sig, proxy_wrapper, proxy);
}
/*****************************************************************/
#define PY_FIELD_EX(T, NAME, REF, EXPR)       \
        vm->bind_property(type, NAME,               \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                return VAR(self.REF()->EXPR);       \
            },                                      \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                self.REF()->EXPR = CAST(decltype(self.REF()->EXPR), args[1]);       \
                return vm->None;                                                    \
            });

#define PY_READONLY_FIELD_EX(T, NAME, REF, EXPR)            \
        vm->bind_property(type, NAME,                       \
            [](VM* vm, ArgsView args){                      \
                T& self = PK_OBJ_GET(T, args[0]);           \
                return VAR(self.REF()->EXPR);               \
            });

#define PY_PROPERTY_EX(T, NAME, REF, FGET, FSET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.REF()->FGET());         \
            },                                          \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                using __NT = decltype(self.REF()->FGET());   \
                self.REF()->FSET(CAST(__NT, args[1]));       \
                return vm->None;                            \
            });

#define PY_READONLY_PROPERTY_EX(T, NAME, REF, FGET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.REF()->FGET());         \
            });
/*****************************************************************/
#define PY_FIELD(T, NAME, EXPR)       \
        vm->bind_property(type, NAME,               \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                return VAR(self.EXPR);       \
            },                                      \
            [](VM* vm, ArgsView args){              \
                T& self = PK_OBJ_GET(T, args[0]);   \
                self.EXPR = CAST(decltype(self.EXPR), args[1]);       \
                return vm->None;                                                    \
            });

#define PY_READONLY_FIELD(T, NAME, EXPR)            \
        vm->bind_property(type, NAME,                       \
            [](VM* vm, ArgsView args){                      \
                T& self = PK_OBJ_GET(T, args[0]);           \
                return VAR(self.EXPR);               \
            });

#define PY_PROPERTY(T, NAME, FGET, FSET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.FGET());         \
            },                                          \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                using __NT = decltype(self.FGET());   \
                self.FSET(CAST(__NT, args[1]));       \
                return vm->None;                            \
            });

#define PY_READONLY_PROPERTY(T, NAME, FGET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = PK_OBJ_GET(T, args[0]);       \
                return VAR(self.FGET());                \
            });
/*****************************************************************/

#define PY_STRUCT_LIKE(wT)   \
        static_assert(std::is_trivially_copyable<wT>::value);                       \
        type->attr().set("__struct__", vm->True);                                   \
        vm->bind_func(type, "fromstruct", 1, [](VM* vm, ArgsView args){             \
            Struct& s = CAST(Struct&, args[0]);                                     \
            if(s.size != sizeof(wT)) vm->ValueError("size mismatch");               \
            PyObject* obj = vm->new_user_object<wT>();                              \
            memcpy(&_CAST(wT&, obj), s.p, sizeof(wT));                              \
            return obj;                                                             \
        }, {}, BindType::STATICMETHOD);                                             \
        vm->bind_func(type, "tostruct", 1, [](VM* vm, ArgsView args){             \
            wT& self = _CAST(wT&, args[0]);                                         \
            return vm->new_user_object<Struct>(&self, sizeof(wT));                  \
        });                                                                         \
        vm->bind_func(type, "addr", 1, [](VM* vm, ArgsView args){                 \
            wT& self = _CAST(wT&, args[0]);                                         \
            return vm->new_user_object<VoidP>(&self);                               \
        });                                                                         \
        vm->bind_func(type, "copy", 1, [](VM* vm, ArgsView args){                 \
            wT& self = _CAST(wT&, args[0]);                                         \
            return vm->new_user_object<wT>(self);                                   \
        });                                                                         \
        vm->bind_func(type, "sizeof", 1, [](VM* vm, ArgsView args){               \
            return VAR(sizeof(wT));                                                 \
        });                                                                         \
        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            wT& self = _CAST(wT&, _0);                                              \
            if(!vm->isinstance(_1, vm->_tp_user<wT>())) return vm->NotImplemented;  \
            wT& other = _CAST(wT&, _1);                                             \
            return VAR(self == other);                                              \
        });                                                                         \

#define PY_POINTER_SETGETITEM(T) \
        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1){  \
            VoidP& self = PK_OBJ_GET(VoidP, _0);                                       \
            i64 i = CAST(i64, _1);                                                      \
            T* tgt = reinterpret_cast<T*>(self.ptr);                                    \
            return VAR(tgt[i]);                                                         \
        });                                                                             \
        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* _0, PyObject* _1, PyObject* _2){  \
            VoidP& self = PK_OBJ_GET(VoidP, _0);                                       \
            i64 i = CAST(i64, _1);                                                      \
            T* tgt = reinterpret_cast<T*>(self.ptr);                                    \
            tgt[i] = CAST(T, _2);                                                       \
        });                                                                         \

}   // namespace pkpy