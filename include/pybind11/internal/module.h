#pragma once

#include "function.h"

namespace pkbind {

class module : public object {
    PKBIND_TYPE_IMPL(object, module, tp_module)

    static module __main__() { return module(py_getmodule("__main__"), object::ref_t{}); }

    static module import(const char* name) {
        raise_call<py_import>(name);
        return borrow<module>(py_retval());
    }

    static module create(const char* name) {
        auto m = py_newmodule(name);
        return steal<module>(m);
    }

    module def_submodule(const char* name, const char* doc = nullptr) {
        // auto package = (attr("__package__").cast<std::string>() += ".") +=
        // attr("__name__").cast<std::string_view>();
        auto fname = (attr("__name__").cast<std::string>() += ".") += name;
        auto m = py_newmodule(fname.c_str());
        setattr(*this, name, m);
        return module(m, object::ref_t{});
    }

    template <typename Fn, typename... Extras>
    module& def(const char* name, Fn&& fn, const Extras... extras) {
        impl::bind_function<false, false>(*this, name, std::forward<Fn>(fn), extras...);
        return *this;
    }
};

using module_ = module;

#define PYBIND11_EMBEDDED_MODULE(name, variable)                                                   \
    static void _pkbind_register_##name(::pkbind::module& variable);                               \
    namespace pkbind::impl {                                                                       \
    auto _module_##name = [] {                                                                     \
        ::pkbind::action::register_start([] {                                                      \
            auto m = ::pkbind::module(py_newmodule(#name), ::pkbind::object::ref_t{});             \
            _pkbind_register_##name(m);                                                            \
        });                                                                                        \
        return 1;                                                                                  \
    }();                                                                                           \
    }                                                                                              \
    static void _pkbind_register_##name(::pkbind::module& variable)

#define PYBIND11_MODULE(name, variable)                                                            \
    static void _pkbind_register_##name(::pkbind::module& variable);                               \
    extern "C" PK_EXPORT bool py_module_initialize() {                                             \
        auto m = ::pkbind::module::create(#name);                                                  \
        _pkbind_register_##name(m);                                                                \
        py_assign(py_retval(), m.ptr());                                                           \
        return true;                                                                               \
    }                                                                                              \
    static void _pkbind_register_##name(::pkbind::module& variable)

}  // namespace pkbind
