#pragma once

#include "function.h"

namespace pkbind {

class module_ : public object {
    PKBIND_TYPE_IMPL(object, module_, tp_module)

    static module_ __main__() { return module_(py_getmodule("__main__"), object::ref_t{}); }

    static module_ import(const char* name) {
        raise_call<py_import>(name);
        return borrow<module_>(py_retval());
    }

    static module_ create(const char* name) {
        auto m = py_newmodule(name);
        return steal<module_>(m);
    }

    void reload() {
        bool ok = py_importlib_reload(ptr());
        if(!ok) { throw error_already_set(); }
    }

    module_ def_submodule(const char* name, const char* doc = nullptr) {
        // auto package = (attr("__package__").cast<std::string>() += ".") +=
        // attr("__name__").cast<std::string_view>();
        auto fname = (attr("__name__").cast<std::string>() += ".") += name;
        auto m = py_newmodule(fname.c_str());
        setattr(*this, name, m);
        return module_(m, object::ref_t{});
    }

    template <typename Fn, typename... Extras>
    module_& def(const char* name, Fn&& fn, const Extras... extras) {
        impl::bind_function<false, false>(*this, name, std::forward<Fn>(fn), extras...);
        return *this;
    }
};

using module = module_;

#define PYBIND11_EMBEDDED_MODULE(name, variable)                                                   \
    static void _pkbind_register_##name(::pkbind::module_& variable);                              \
    namespace pkbind::impl {                                                                       \
    auto _module_##name = [] {                                                                     \
        ::pkbind::action::register_start([] {                                                      \
            auto m = ::pkbind::module_::create(#name);                                             \
            _pkbind_register_##name(m);                                                            \
        });                                                                                        \
        return 1;                                                                                  \
    }();                                                                                           \
    }                                                                                              \
    static void _pkbind_register_##name(::pkbind::module_& variable)

#define PYBIND11_MODULE(name, variable)                                                            \
    static void _pkbind_register_##name(::pkbind::module_& variable);                              \
    extern "C" PK_EXPORT bool py_module_initialize() {                                             \
        auto m = ::pkbind::module_::create(#name);                                                 \
        _pkbind_register_##name(m);                                                                \
        py_assign(py_retval(), m.ptr());                                                           \
        return true;                                                                               \
    }                                                                                              \
    static void _pkbind_register_##name(::pkbind::module_& variable)

}  // namespace pkbind
