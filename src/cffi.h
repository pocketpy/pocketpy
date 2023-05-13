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


struct VoidP{
    PY_CLASS(VoidP, c, void_p)

    void* ptr;
    VoidP(void* ptr): ptr(ptr){}
    VoidP(): ptr(nullptr){}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<VoidP>(type);

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            std::stringstream ss;
            ss << "<void* at " << self.ptr << ">";
            return VAR(ss.str());
        });
    }
};

struct PlainOldData{
    PY_CLASS(PlainOldData, c, pod)

    char* p;

    template<typename T>
    PlainOldData(const T& data){
        static_assert(std::is_pod_v<T>);
        p = new char[sizeof(T)];
        memcpy(p, &data, sizeof(T));
    }

    PlainOldData(): p(nullptr){}
    ~PlainOldData(){ delete[] p; }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<PlainOldData>(type);
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
T to_void_p(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<T>);
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<T>(p.ptr);
}

template<typename T>
T to_plain_old_data(VM* vm, PyObject* var){
    static_assert(std::is_pod_v<T>);
    PlainOldData& pod = CAST(PlainOldData&, var);
    return *reinterpret_cast<T*>(pod.p);
}

template<typename T>
std::enable_if_t<std::is_pod_v<T>, PyObject*> py_var(VM* vm, const T& data){
    return VAR_T(PlainOldData, data);
}
/*****************************************************************/
template<typename Ret, typename... Params>
struct NativeProxyFuncC {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFuncC(_Fp func) : func(func) {}

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

}   // namespace pkpy