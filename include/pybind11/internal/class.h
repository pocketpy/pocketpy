#pragma once

#include "module.h"
#include "type_traits.h"

namespace pkbind {

struct dynamic_attr {};

template <typename T, typename Base = void>
class class_ : public type {
protected:
    handle m_scope;

public:
    using type::type;
    using underlying_type = T;

    template <typename... Args>
    class_(handle scope, const char* name, const Args&... args) :
        type(py_newtype(name,
                        std::is_same_v<Base, void> ? tp_object : type::of<Base>().index(),
                        scope.ptr(),
                        [](void* data) {
                            static_cast<instance*>(data)->~instance();
                        })),
        m_scope(scope) {
        m_type_map.try_emplace(typeid(T), this->index());

        auto& info = type_info::of<T>();
        info.name = name;

        py_bind(
            py_tpobject(this->index()),
            "__new__(type, *args, **kwargs)",
            [](int, py_Ref stack) {
                auto cls = py_offset(stack, 0);
                [[maybe_unused]] auto args = py_offset(stack, 1);
                [[maybe_unused]] auto kwargs = py_offset(stack, 2);

                auto info = &type_info::of<T>();
                int slot = ((std::is_same_v<dynamic_attr, Args> || ...) ? -1 : 0);
                void* data =
                    py_newobject(py_retval(), steal<type>(cls).index(), slot, sizeof(instance));
                new (data) instance{instance::Flag::Own, operator new (info->size), info};
                return true;
            });
    }

    /// bind constructor
    template <typename... Args, typename... Extra>
    class_& def(impl::constructor<Args...>, const Extra&... extra) {
        if constexpr(!std::is_constructible_v<T, Args...>) {
            static_assert(std::is_constructible_v<T, Args...>, "Invalid constructor arguments");
        } else {
            impl::bind_function<true, false>(
                *this,
                "__init__",
                [](T* self, Args... args) {
                    new (self) T(args...);
                },
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
            impl::bind_function<true, false>(*this, "__init__", factory.make(), extra...);
            return *this;
        }
    }

    /// bind member function
    template <typename Fn, typename... Extra>
    class_& def(const char* name, Fn&& f, const Extra&... extra) {
        using first = remove_cvref_t<std::tuple_element_t<0, callable_args_t<remove_cvref_t<Fn>>>>;
        constexpr bool is_first_base_of_v = std::is_base_of_v<first, T> || std::is_same_v<first, T>;

        if constexpr(!is_first_base_of_v) {
            static_assert(
                is_first_base_of_v,
                "If you want to bind member function, the first argument must be the base class");
        } else {
            impl::bind_function<true, false>(*this, name, std::forward<Fn>(f), extra...);
        }

        return *this;
    }

    /// bind operators
    template <typename Operator, typename... Extras>
    class_& def(Operator op, const Extras&... extras) {
        op.execute(*this, extras...);
        return *this;
    }

    /// bind static function
    template <typename Fn, typename... Extra>
    class_& def_static(const char* name, Fn&& f, const Extra&... extra) {
        impl::bind_function<false, true>(*this, name, std::forward<Fn>(f), extra...);
        return *this;
    }

    template <typename MP, typename... Extras>
    class_& def_readwrite(const char* name, MP mp, const Extras&... extras) {
        if constexpr(!std::is_member_object_pointer_v<MP>) {
            static_assert(std::is_member_object_pointer_v<MP>,
                          "def_readwrite only supports pointer to data member");
        } else {
            impl::bind_property(
                *this,
                name,
                [mp](class_type_t<MP>& self) -> auto& {
                    return self.*mp;
                },
                [mp](class_type_t<MP>& self, const member_type_t<MP>& value) {
                    self.*mp = value;
                },
                extras...);
        }
        return *this;
    }

    template <typename MP, typename... Extras>
    class_& def_readonly(const char* name, MP mp, const Extras&... extras) {
        if constexpr(!std::is_member_object_pointer_v<MP>) {
            static_assert(std::is_member_object_pointer_v<MP>,
                          "def_readonly only supports pointer to data member");
        } else {
            impl::bind_property(
                *this,
                name,
                [mp](class_type_t<MP>& self) -> auto& {
                    return self.*mp;
                },
                nullptr,
                extras...);
        }
        return *this;
    }

    template <typename Getter, typename Setter, typename... Extras>
    class_& def_property(const char* name, Getter&& g, Setter&& s, const Extras&... extras) {
        impl::bind_property(*this,
                            name,
                            std::forward<Getter>(g),
                            std::forward<Setter>(s),
                            extras...);
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
    std::vector<std::pair<const char*, object>> m_values;

public:
    using Base = class_<T, Others...>;

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
        auto var = pkbind::cast(value, return_value_policy::copy);
        setattr(*this, name, var);
        m_values.emplace_back(name, std::move(var));
        return *this;
    }

    enum_& export_values() {
        for(auto& [name, value]: m_values) {
            setattr(Base::m_scope, name, value);
        }
        return *this;
    }
};

}  // namespace pkbind
