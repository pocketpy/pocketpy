#pragma once

#include "module.h"
#include <vector>

namespace pybind11 {

struct dynamic_attr {};

template <typename T, typename Base = void>
class class_ : public type {
protected:
    handle m_scope;

public:
    using type::type;
    using underlying_type = T;

    template <typename... Args>
    class_(const handle& scope, const char* name, const Args&... args) :
        m_scope(scope), type(type_visitor::create<T, Base>(scope, name)) {
        auto& info = type_info::of<T>();
        info.name = name;

        // bind __new__
        interpreter::bind_func(m_ptr, pkpy::__new__, -1, [](pkpy::VM* vm, pkpy::ArgsView args) {
            auto cls = handle(args[0])._as<pkpy::Type>();

            // check if the class has constructor, if not, raise error
            if(vm->find_name_in_mro(cls, pkpy::__init__) == nullptr) {
                vm->RuntimeError("if you want to create instance of bound class, you must bind constructor for it");
            }

            auto var = instance::create(cls, &type_info::of<T>());

            if constexpr(types_count_v<dynamic_attr, Args...> != 0) {
#if PK_VERSION_MAJOR == 2
                var.get()->_attr = new pkpy::NameDict();
#else 
                var->_enable_instance_dict();
#endif
            }

            return var;
        });
    }

    /// bind constructor
    template <typename... Args, typename... Extra>
    class_& def(impl::constructor<Args...>, const Extra&... extra) {
        if constexpr(!std::is_constructible_v<T, Args...>) {
            static_assert(std::is_constructible_v<T, Args...>, "Invalid constructor arguments");
        } else {
            impl::bind_function<true>(
                *this,
                "__init__",
                [](T* self, Args... args) {
                    new (self) T(args...);
                },
                pkpy::BindType::DEFAULT,
                extra...);
            return *this;
        }
    }

    template <typename Fn, typename... Extra>
    class_& def(impl::factory<Fn> factory, const Extra&... extra) {
        using ret = callable_return_t<Fn>;

        if constexpr(!std::is_same_v<T, ret>) {
            static_assert(std::is_same_v<T, ret>, "Factory function must return the class type");
        } else {
            impl::bind_function<true>(*this, "__init__", factory.make(), pkpy::BindType::DEFAULT, extra...);
            return *this;
        }
    }

    /// bind member function
    template <typename Fn, typename... Extra>
    class_& def(const char* name, Fn&& f, const Extra&... extra) {
        using first = remove_cvref_t<std::tuple_element_t<0, callable_args_t<remove_cvref_t<Fn>>>>;
        constexpr bool is_first_base_of_v = std::is_base_of_v<first, T> || std::is_same_v<first, T>;

        if constexpr(!is_first_base_of_v) {
            static_assert(is_first_base_of_v,
                          "If you want to bind member function, the first argument must be the base class");
        } else {
            impl::bind_function<true>(*this, name, std::forward<Fn>(f), pkpy::BindType::DEFAULT, extra...);
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
        impl::bind_function<false>(*this, name, std::forward<Fn>(f), pkpy::BindType::STATICMETHOD, extra...);
        return *this;
    }

    template <typename MP, typename... Extras>
    class_& def_readwrite(const char* name, MP mp, const Extras&... extras) {
        if constexpr(!std::is_member_object_pointer_v<MP>) {
            static_assert(std::is_member_object_pointer_v<MP>, "def_readwrite only supports pointer to data member");
        } else {
            impl::bind_property(*this, name, mp, mp, extras...);
        }
        return *this;
    }

    template <typename MP, typename... Extras>
    class_& def_readonly(const char* name, MP mp, const Extras&... extras) {
        if constexpr(!std::is_member_object_pointer_v<MP>) {
            static_assert(std::is_member_object_pointer_v<MP>, "def_readonly only supports pointer to data member");
        } else {
            impl::bind_property(*this, name, mp, nullptr, extras...);
        }
        return *this;
    }

    template <typename Getter, typename Setter, typename... Extras>
    class_& def_property(const char* name, Getter&& g, Setter&& s, const Extras&... extras) {
        impl::bind_property(*this, name, std::forward<Getter>(g), std::forward<Setter>(s), extras...);
        return *this;
    }

    template <typename Getter, typename... Extras>
    class_& def_property_readonly(const char* name, Getter&& mp, const Extras&... extras) {
        impl::bind_property(*this, name, std::forward<Getter>(mp), nullptr, extras...);
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
    class_& def_property_static(const char* name, Getter&& g, Setter&& s, const Extras&... extras) {
        static_assert(
            dependent_false<Getter>,
            "define static properties requires metaclass. This is a complex feature with few use cases, so it may never be implemented.");
        return *this;
    }
};

template <typename T, typename... Others>
class enum_ : public class_<T, Others...> {
    std::vector<std::pair<const char*, handle>> m_values;

public:
    using Base = class_<T, Others...>;
    using class_<T, Others...>::class_;

    template <typename... Args>
    enum_(const handle& scope, const char* name, Args&&... args) :
        class_<T, Others...>(scope, name, std::forward<Args>(args)...) {

        Base::def(init([](int value) {
            return static_cast<T>(value);
        }));

        Base::def("__eq__", [](T& self, T& other) {
            return self == other;
        });

        Base::def_property_readonly("value", [](T& self) {
            return int_(static_cast<std::underlying_type_t<T>>(self));
        });
    }

    enum_& value(const char* name, T value) {
        handle var = pybind11::cast(value, return_value_policy::copy);
        setattr(*this, name, var);
        m_values.emplace_back(name, var);
        return *this;
    }

    enum_& export_values() {
        for(auto& [name, value]: m_values) {
            setattr(Base::m_scope, name, value);
        }
        return *this;
    }
};
}  // namespace pybind11

