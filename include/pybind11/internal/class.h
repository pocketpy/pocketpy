#pragma once

#include "cpp_function.h"

namespace pybind11 {

    class module : public object {

    public:
        using object::object;

        static module import(const char* name) {
            if(name == std::string_view{"__main__"}) {
                return module{vm->_main, true};
            } else {
                return module{vm->py_import(name, false), true};
            }
        }
    };

    // TODO:
    // 1. inheritance
    // 2. virtual function
    // 3. factory function

    template <typename T, typename... Others>
    class class_ : public type {
    public:
        using type::type;

        template <typename... Args>
        class_(const handle& scope, const char* name, Args&&... args) :
            type(vm->new_type_object(scope.ptr(),
                                     name,
                                     vm->tp_object,
                                     false,
                                     pkpy::PyTypeInfo::Vt::get<instance>()),
                 true) {
            pkpy::PyVar mod = scope.ptr();
            mod->attr().set(name, m_ptr);
            vm->_cxx_typeid_map[typeid(T)] = _builtin_cast<pkpy::Type>(m_ptr);
            vm->bind_func(m_ptr, "__new__", -1, [](pkpy::VM* vm, pkpy::ArgsView args) {
                auto cls = _builtin_cast<pkpy::Type>(args[0]);
                return instance::create<T>(cls);
            });
        }

        /// bind constructor
        template <typename... Args, typename... Extra>
        class_& def(init<Args...>, const Extra&... extra) {
            if constexpr(!std::is_constructible_v<T, Args...>) {
                static_assert(std::is_constructible_v<T, Args...>, "Invalid constructor arguments");
            } else {
                bind_function(
                    *this,
                    "__init__",
                    [](T* self, Args... args) { new (self) T(args...); },
                    pkpy::BindType::DEFAULT,
                    extra...);
                return *this;
            }
        }

        /// bind member function
        template <typename Fn, typename... Extra>
        class_& def(const char* name, Fn&& f, const Extra&... extra) {
            using first = std::tuple_element_t<0, callable_args_t<remove_cvref_t<Fn>>>;
            constexpr bool is_first_base_of_v =
                std::is_reference_v<first> && std::is_base_of_v<T, remove_cvref_t<first>>;

            if constexpr(!is_first_base_of_v) {
                static_assert(
                    is_first_base_of_v,
                    "If you want to bind member function, the first argument must be the base class");
            } else {
                bind_function(*this, name, std::forward<Fn>(f), pkpy::BindType::DEFAULT, extra...);
            }

            return *this;
        }

        /// bind operators
        template <typename Operator, typename... Extras>
        class_& def(Operator op, const Extras&... extras) {
            op.execute(*this, extras...);
            return *this;
        }

        // TODO: factory function

        /// bind static function
        template <typename Fn, typename... Extra>
        class_& def_static(const char* name, Fn&& f, const Extra&... extra) {
            bind_function(*this, name, std::forward<Fn>(f), pkpy::BindType::STATICMETHOD, extra...);
            return *this;
        }

        template <typename MP, typename... Extras>
        class_& def_readwrite(const char* name, MP mp, const Extras&... extras) {
            if constexpr(!std::is_member_object_pointer_v<MP>) {
                static_assert(std::is_member_object_pointer_v<MP>,
                              "def_readwrite only supports pointer to data member");
            } else {
                bind_property(*this, name, mp, mp, extras...);
            }
            return *this;
        }

        template <typename MP, typename... Extras>
        class_& def_readonly(const char* name, MP mp, const Extras&... extras) {
            if constexpr(!std::is_member_object_pointer_v<MP>) {
                static_assert(std::is_member_object_pointer_v<MP>,
                              "def_readonly only supports pointer to data member");
            } else {
                bind_property(*this, name, mp, nullptr, extras...);
            }
            return *this;
        }

        template <typename Getter, typename Setter, typename... Extras>
        class_& def_property(const char* name, Getter&& g, Setter&& s, const Extras&... extras) {
            bind_property(*this, name, std::forward<Getter>(g), std::forward<Setter>(s), extras...);
            return *this;
        }

        template <typename Getter, typename... Extras>
        class_& def_property_readonly(const char* name, Getter&& mp, const Extras&... extras) {
            bind_property(*this, name, std::forward<Getter>(mp), nullptr, extras...);
            return *this;
        }

        template <typename Var, typename... Extras>
        class_& def_readwrite_static(const char* name, Var& mp, const Extras&... extras) {
            static_assert(
                dependent_false<Var>,
                "define static properties requires metaclass. This is a complex feature with few use cases, so it may never be implemented.");
            return *this;
        }

        template <typename Var, typename... Extras>
        class_& def_readonly_static(const char* name, Var& mp, const Extras&... extras) {
            static_assert(
                dependent_false<Var>,
                "define static properties requires metaclass. This is a complex feature with few use cases, so it may never be implemented.");
            return *this;
        }

        template <typename Getter, typename Setter, typename... Extras>
        class_&
            def_property_static(const char* name, Getter&& g, Setter&& s, const Extras&... extras) {
            static_assert(
                dependent_false<Getter>,
                "define static properties requires metaclass. This is a complex feature with few use cases, so it may never be implemented.");
            return *this;
        }
    };

    template <typename T, typename... Others>
    class enum_ : public class_<T, Others...> {
        std::map<const char*, pkpy::PyVar> m_values;

    public:
        using class_<T, Others...>::class_;

        template <typename... Args>
        enum_(const handle& scope, const char* name, Args&&... args) :
            class_<T, Others...>(scope, name, std::forward<Args>(args)...) {}

        enum_& value(const char* name, T value) {
            handle var = type_caster<T>::cast(value, return_value_policy::copy);
            this->m_ptr->attr().set(name, var.ptr());
            m_values[name] = var.ptr();
            return *this;
        }

        enum_& export_values() {
            pkpy::PyVar mod = this->m_ptr->attr("__module__");
            for(auto& [name, value]: m_values) {
                mod->attr().set(name, value);
            }
            return *this;
        }
    };
}  // namespace pybind11
