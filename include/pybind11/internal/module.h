#pragma once
#include "cpp_function.h"

namespace pybind11 {

class module_ : public object {

public:
    using object::object;

    static module_ __main__() { return vm->_main; }

    static module_ import(const char* name) {
        if(name == std::string_view{"__main__"}) {
            return vm->_main;
        } else {
            return vm->py_import(name, false);
        }
    }

    module_ def_submodule(const char* name, const char* doc = nullptr) {
        // TODO: resolve package
        //auto package = this->package()._as<pkpy::Str>() + "." + this->name()._as<pkpy::Str>();
        auto fname = this->name()._as<pkpy::Str>() + "." + name;
        auto m = vm->new_module(fname, "");
        setattr(*this, name, m);
        return m;
    }

    template <typename Fn, typename... Extras>
    module_& def(const char* name, Fn&& fn, const Extras... extras) {
        impl::bind_function<false>(*this, name, std::forward<Fn>(fn), pkpy::BindType::DEFAULT, extras...);
        return *this;
    }
};

#define PYBIND11_EMBEDDED_MODULE(name, variable)                                                                       \
    static void _pybind11_register_##name(pybind11::module_& variable);                                                \
    namespace pybind11::impl {                                                                                         \
    auto _module_##name = [] {                                                                                         \
        interpreter::register_init([] {                                                                                \
            pybind11::module_ m = vm->new_module(#name, "");                                                           \
            _pybind11_register_##name(m);                                                                              \
        });                                                                                                            \
        return 1;                                                                                                      \
    }();                                                                                                               \
    }                                                                                                                  \
    static void _pybind11_register_##name(pybind11::module_& variable)

}  // namespace pybind11
