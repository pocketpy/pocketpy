#pragma once

#include "common.h"

namespace pkpy {

struct any{
    struct vtable{
        const std::type_index type;
        void (*deleter)(void*);

        template<typename T>
        inline static vtable* get(){
            static_assert(std::is_same_v<T, std::decay_t<T>>);
            if constexpr (is_sso_v<T>){
                static vtable vt{ typeid(T), [](void*){} };
                return &vt;
            }else{
                static vtable vt{ typeid(T), [](void* ptr){ delete static_cast<T*>(ptr); } };
                return &vt;
            }
        }
    };

    void* data;
    vtable* _vt;

    any() : data(nullptr), _vt(nullptr) {}

    explicit operator bool() const { return data != nullptr; }

    template<typename T>
    any(T&& value){
        using U = std::decay_t<T>;
        static_assert(!std::is_same_v<U, any>, "any(const any&) is deleted");
        if constexpr (is_sso_v<U>){
            memcpy(&data, &value, sizeof(U));
        }else{
            data = new U(std::forward<T>(value));
        }
        _vt = vtable::get<U>();
    }

    any(any&& other) noexcept;
    any& operator=(any&& other) noexcept;

    const std::type_index type_id() const{
        return _vt ? _vt->type : typeid(void);
    }

    any(const any& other) = delete;
    any& operator=(const any& other) = delete;

    ~any() { if(data) _vt->deleter(data); }
};

template<typename T>
bool any_cast(const any& a, T** out){
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    if(a.type_id() != typeid(T)) return false;
    if constexpr (is_sso_v<T>){
        *out = (T*)(&a.data);
    }else{
        *out = static_cast<T*>(a.data);
    }
    return true;
}

using UserData = any;

} // namespace pkpy