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
        PK_ASSERT(args.size() == N);
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
        PK_ASSERT(args.size() == N+1);
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    PyObject* call(VM* vm, ArgsView args, std::index_sequence<Is...>){
        T& self = py_cast<T&>(vm, args[0]);
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
#define PK_REGISTER_FIELD(T, NAME, REF, EXPR)       \
        vm->bind_property(type, NAME,               \
            [](VM* vm, ArgsView args){              \
                T& self = _CAST(T&, args[0]);       \
                return VAR(self.REF().EXPR);        \
            },                                      \
            [](VM* vm, ArgsView args){              \
                T& self = _CAST(T&, args[0]);       \
                self.REF().EXPR = CAST(decltype(self.REF().EXPR), args[1]);     \
                return vm->None;                                                \
            });

#define PK_REGISTER_READONLY_FIELD(T, NAME, REF, EXPR)          \
        vm->bind_property(type, NAME,                           \
            [](VM* vm, ArgsView args){              \
                T& self = _CAST(T&, args[0]);       \
                return VAR(self.REF().EXPR);        \
            });

#define PK_REGISTER_PROPERTY(T, NAME, REF, FGET, FSET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                return VAR(self.REF().FGET());          \
            },                                          \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                using __NT = decltype(self.REF().FGET());   \
                self.REF().FSET(CAST(__NT, args[1]));       \
                return vm->None;                            \
            });

#define PK_REGISTER_READONLY_PROPERTY(T, NAME, REF, FGET)  \
        vm->bind_property(type, NAME,                   \
            [](VM* vm, ArgsView args){                  \
                T& self = _CAST(T&, args[0]);           \
                return VAR(self.REF().FGET());          \
            });

}   // namespace pkpy