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
    static constexpr int N = sizeof...(Params) + 1;
    using _Fp = Ret(T::*)(Params...);
    _Fp func;
    NativeProxyMethodC(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) override {
        PK_ASSERT(args.size() == N);
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

template<typename T>
struct OpaquePointer{
    T* ptr;
    OpaquePointer(T* ptr): ptr(ptr){}

    T* operator->(){ return ptr; }
};

#define PK_REGISTER_FIELD(T, NAME) \
        type->attr().set(#NAME, vm->property(   \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                return VAR(self->NAME); \
            },  \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                self->NAME = CAST(decltype(self->NAME), args[1]); \
                return vm->None;    \
            }));

#define PK_REGISTER_READONLY_FIELD(T, NAME) \
        type->attr().set(#NAME, vm->property(   \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                return VAR(self->NAME); \
            }));

#define PK_REGISTER_PROPERTY(T, NAME)   \
        type->attr().set(#NAME, vm->property(   \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                return VAR(self->get_##NAME()); \
            },  \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                using __NT = decltype(self->get_##NAME());    \
                self->set_##NAME(CAST(__NT, args[1])); \
                return vm->None;    \
            }));

#define PK_REGISTER_READONLY_PROPERTY(T, NAME)   \
        type->attr().set(#NAME, vm->property(   \
            [](VM* vm, ArgsView args){  \
                T& self = _CAST(T&, args[0]);   \
                return VAR(self->get_##NAME()); \
            }));

#define PK_REGISTER_CONSTRUCTOR(T, T0)  \
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){ \
            void* p = CAST(void*, args[0]); \
            return VAR_T(T, (T0*)p);    \
        });

}   // namespace pkpy