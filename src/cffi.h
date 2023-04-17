#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy {

template<typename Ret, typename... Params>
struct NativeProxyFunc {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFunc(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) {
        if (args.size() != N) {
            vm->TypeError("expected " + std::to_string(N) + " arguments, but got " + std::to_string(args.size()));
        }
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyObject*> call(VM* vm, ArgsView args, std::index_sequence<Is...>) {
        func(py_cast<Params>(vm, args[Is])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyObject*> call(VM* vm, ArgsView args, std::index_sequence<Is...>) {
        __Ret ret = func(py_cast<Params>(vm, args[Is])...);
        return VAR(std::move(ret));
    }
};

template<typename Ret, typename T, typename... Params>
struct NativeProxyMethod {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(T::*)(Params...);
    _Fp func;
    NativeProxyMethod(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, ArgsView args) {
        int actual_size = args.size() - 1;
        if (actual_size != N) {
            vm->TypeError("expected " + std::to_string(N) + " arguments, but got " + std::to_string(actual_size));
        }
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyObject*> call(VM* vm, ArgsView args, std::index_sequence<Is...>) {
        T& self = py_cast<T&>(vm, args[0]);
        (self.*func)(py_cast<Params>(vm, args[Is+1])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyObject*> call(VM* vm, ArgsView args, std::index_sequence<Is...>) {
        T& self = py_cast<T&>(vm, args[0]);
        __Ret ret = (self.*func)(py_cast<Params>(vm, args[Is+1])...);
        return VAR(std::move(ret));
    }
};

template<typename Ret, typename... Params>
auto native_proxy_callable(Ret(*func)(Params...)) {
    return NativeProxyFunc<Ret, Params...>(func);
}

template<typename Ret, typename T, typename... Params>
auto native_proxy_callable(Ret(T::*func)(Params...)) {
    return NativeProxyMethod<Ret, T, Params...>(func);
}

struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    VoidP(void* ptr): ptr(ptr){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_static_method<1>(type, "__repr__", [](VM* vm, const Args& args){
            VoidP& self = CAST(VoidP&, args[0]);
            std::stringstream ss;
            ss << "<void* at " << self.ptr << ">";
            return VAR(ss.str());
        });
    }
};

inline void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    VoidP::register_class(vm, mod);
}

inline PyObject* py_var(VM* vm, void* p){
    return VAR_T(VoidP, p);
}

inline PyObject* py_var(VM* vm, char* p){
    return VAR_T(VoidP, p);
}
/***********************************************/

template<typename T>
struct _pointer {
    static constexpr int level = 0;
    using baseT = T;
};

template<typename T>
struct _pointer<T*> {
    static constexpr int level = _pointer<T>::level + 1;
    using baseT = typename _pointer<T>::baseT;
};

template<typename T>
struct pointer {
    static constexpr int level = _pointer<std::decay_t<T>>::level;
    using baseT = typename _pointer<std::decay_t<T>>::baseT;
};

template<typename T>
T py_pointer_cast(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<T>);
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<T>(p.ptr);
}
}   // namespace pkpy