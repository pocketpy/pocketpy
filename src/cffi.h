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