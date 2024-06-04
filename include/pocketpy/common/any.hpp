#pragma once

#include "pocketpy/common/traits.hpp"

#include <typeindex>
#include <cassert>
#include <utility>
#include <cstdlib>
#include <cstring>

namespace pkpy {

template <typename T>
constexpr inline bool is_any_sso_v = is_pod_v<T> && sizeof(T) <= sizeof(void*);

struct any {
    struct vtable {
        const std::type_index type;
        void (*deleter)(void*);

        template <typename T>
        inline static vtable* get() {
            static_assert(std::is_same_v<T, std::decay_t<T>>);
            if constexpr(is_any_sso_v<T>) {
                static vtable vt{typeid(T), nullptr};
                return &vt;
            } else {
                static vtable vt{typeid(T), [](void* ptr) {
                                     delete static_cast<T*>(ptr);
                                 }};
                return &vt;
            }
        }
    };

    void* data;
    vtable* _vt;

    any() : data(nullptr), _vt(nullptr) {}

    explicit operator bool () const { return _vt != nullptr; }

    template <typename T>
    any(T&& value) {
        using U = std::decay_t<T>;
        static_assert(!std::is_same_v<U, any>, "any(const any&) is deleted");
        static_assert(sizeof(U) == sizeof(T));
        if constexpr(is_any_sso_v<U>) {
            std::memcpy(&data, &value, sizeof(U));
        } else {
            data = new U(std::forward<T>(value));
        }
        _vt = vtable::get<U>();
    }

    any(any&& other) noexcept;
    any& operator= (any&& other) noexcept;

    const std::type_index type_id() const { return _vt ? _vt->type : typeid(void); }

    any(const any& other) = delete;
    any& operator= (const any& other) = delete;

    ~any() {
        if(_vt && _vt->deleter) _vt->deleter(data);
    }

    template <typename T>
    T& _cast() const noexcept {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        if constexpr(is_any_sso_v<T>) {
            return *((T*)(&data));
        } else {
            return *(static_cast<T*>(data));
        }
    }

    template <typename T>
    T& cast() const {
        static_assert(std::is_same_v<T, std::decay_t<T>>);
        if(type_id() != typeid(T)) __bad_any_cast(typeid(T), type_id());
        return _cast<T>();
    }

    static void __bad_any_cast(const std::type_index expected, const std::type_index actual);
};

template <typename T>
struct function;

template <typename Ret, typename... Params>
struct function<Ret(Params...)> {
    any _impl;
    Ret (*_wrapper)(const any&, Params...);

    function() : _impl(), _wrapper(nullptr) {}

    explicit operator bool () const { return _wrapper != nullptr; }

    template <typename F>
    function(F&& f) : _impl(std::forward<F>(f)) {
        _wrapper = [](const any& impl, Params... params) -> Ret {
            return impl._cast<std::decay_t<F>>()(std::forward<Params>(params)...);
        };
    }

    Ret operator() (Params... params) const {
        assert(_wrapper);
        return _wrapper(_impl, std::forward<Params>(params)...);
    }
};

}  // namespace pkpy
