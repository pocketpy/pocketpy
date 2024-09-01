/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTL_XVARIANT_HPP
#define XTL_XVARIANT_HPP

#include "xvariant_impl.hpp"
#include "xclosure.hpp"
#include "xmeta_utils.hpp"

namespace xtl
{
    using mpark::variant;
    using mpark::monostate;
    using mpark::bad_variant_access;
    using mpark::variant_size;
#ifdef MPARK_VARIABLE_TEMPLATES
    using mpark::variant_size_v;
#endif
    using mpark::variant_alternative;
    using mpark::variant_alternative_t;
    using mpark::variant_npos;

    using mpark::visit;
    using mpark::holds_alternative;
    using mpark::get;
    using mpark::get_if;

    namespace detail
    {
        template <class T>
        struct xgetter
        {
            template <class... Ts>
            static constexpr T& get(xtl::variant<Ts...>& v)
            {
                return xtl::get<T>(v);
            }

            template <class... Ts>
            static constexpr T&& get(xtl::variant<Ts...>&& v)
            {
                return xtl::get<T>(std::move(v));
            }

            template <class... Ts>
            static constexpr const T& get(const xtl::variant<Ts...>& v)
            {
                return xtl::get<T>(v);
            }

            template <class... Ts>
            static constexpr const T&& get(const xtl::variant<Ts...>&& v)
            {
                return xtl::get<T>(std::move(v));
            }
        };

        template <class T>
        struct xgetter<T&>
        {
            template <class... Ts>
            static constexpr T& get(xtl::variant<Ts...>& v)
            {
                return xtl::get<xtl::xclosure_wrapper<T&>>(v).get();
            }

            template <class... Ts>
            static constexpr T& get(xtl::variant<Ts...>&& v)
            {
                return xtl::get<xtl::xclosure_wrapper<T&>>(std::move(v)).get();
            }

            template <class... Ts>
            static constexpr const T& get(const xtl::variant<Ts...>& v)
            {
                return xtl::get<xtl::xclosure_wrapper<T&>>(v).get();
            }

            template <class... Ts>
            static constexpr const T& get(const xtl::variant<Ts...>&& v)
            {
                return xtl::get<xtl::xclosure_wrapper<T&>>(std::move(v)).get();
            }
        };

        template <class T>
        struct xgetter<const T&>
        {
            template <class... Ts>
            static constexpr const T& get(const xtl::variant<Ts...>& v)
            {
                using cl_type = xtl::xclosure_wrapper<const T&>;
                return get_impl(v, xtl::mpl::contains<xtl::mpl::vector<Ts...>, cl_type>());
            }

            template <class... Ts>
            static constexpr const T& get(const xtl::variant<Ts...>&& v)
            {
                using cl_type = xtl::xclosure_wrapper<const T&>;
                return get_impl(std::move(v), xtl::mpl::contains<xtl::mpl::vector<Ts...>, cl_type>());
            }

            template <class... Ts>
            static constexpr const T& get(xtl::variant<Ts...>& v)
            {
                return get(static_cast<const xtl::variant<Ts...>&>(v));
            }

            template <class... Ts>
            static constexpr const T& get(xtl::variant<Ts...>&& v)
            {
                return get(static_cast<const xtl::variant<Ts...>&&>(v));
            }

        private:

            template <class... Ts>
            static constexpr const T& get_impl(const xtl::variant<Ts...>& v, xtl::mpl::bool_<true>)
            {
                return xtl::get<xtl::xclosure_wrapper<const T&>>(v).get();
            }

            template <class... Ts>
            static constexpr const T& get_impl(const xtl::variant<Ts...>& v, xtl::mpl::bool_<false>)
            {
                return static_cast<const xtl::xclosure_wrapper<T&>&>(xtl::get<xtl::xclosure_wrapper<T&>>(v)).get();
            }

            template <class... Ts>
            static constexpr const T& get_impl(const xtl::variant<Ts...>&& v, xtl::mpl::bool_<true>)
            {
                return xtl::get<xtl::closure_wrapper<const T&>>(std::move(v)).get();
            }

            template <class... Ts>
            static constexpr const T& get_impl(const xtl::variant<Ts...>&& v, xtl::mpl::bool_<false>)
            {
                return static_cast<const xtl::xclosure_wrapper<T&>&&>(xtl::get<xtl::xclosure_wrapper<T&>>(std::move(v))).get();
            }
        };
    }

    template <class T, class... Ts>
    constexpr decltype(auto) xget(xtl::variant<Ts...>& v)
    {
        return detail::xgetter<T>::get(v);
    }

    template <class T, class... Ts>
    constexpr decltype(auto) xget(xtl::variant<Ts...>&& v)
    {
        return detail::xgetter<T>::get(std::move(v));
    }

    template <class T, class... Ts>
    constexpr decltype(auto) xget(const xtl::variant<Ts...>& v)
    {
        return detail::xgetter<T>::get(v);
    }

    template <class T, class... Ts>
    constexpr decltype(auto) xget(const xtl::variant<Ts...>&& v)
    {
        return detail::xgetter<T>::get(std::move(v));
    }

    /************************
     * overload for lambdas *
     ************************/

    // This hierarchy is required since ellipsis in using declarations are not supported until C++17
    template <class... Ts>
    struct overloaded;

    template <class T>
    struct overloaded<T> : T
    {
        overloaded(T arg) : T(arg) {}
        using T::operator();
    };

    template <class T1, class T2, class... Ts>
    struct overloaded<T1, T2, Ts...> : T1, overloaded<T2, Ts...>
    {
        template <class... Us>
        overloaded(T1 t1, T2 t2, Us... args) : T1(t1), overloaded<T2, Ts...>(t2, args...) {}

        using T1::operator();
        using overloaded<T2, Ts...>::operator();
    };

    template <class... Ts>
    inline overloaded<Ts...> make_overload(Ts... arg)
    {
        return overloaded<Ts...>{arg...};
    }
}

#endif
