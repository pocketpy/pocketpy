#pragma once

#include "accessor.h"

namespace pkbind {

struct type_info {
    std::string_view name;
    std::size_t size;
    std::size_t alignment;
    void (*destructor)(void*);
    const std::type_info* type;

    template <typename T>
    static type_info& of() {
        static_assert(!std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>,
                      "T must not be a reference type or const type.");
        static type_info info = {
            type_name<T>(),
            sizeof(T),
            alignof(T),
            [](void* ptr) {
                delete static_cast<T*>(ptr);
            },
            &typeid(T),
        };
        return info;
    }
};

// all registered C++ class will be ensured as instance type.
struct instance {
    // use to record the type information of C++ class.
    enum Flag {
        None = 0,
        Own = 1 << 0,  // if the instance is owned by C++ side.
        Ref = 1 << 1,  // need to mark the parent object.
    };

    Flag flag;
    void* data;
    const type_info* info;
    object parent;

public:
    template <typename Value>
    static object create(type type, Value&& value_, handle parent_, return_value_policy policy) {
        using underlying_type = remove_cvref_t<Value>;

        auto& value = [&]() -> auto& {
            // note that, pybind11 will ignore the const qualifier.
            // in fact, try to modify a const value will result in undefined behavior.
            if constexpr(std::is_pointer_v<underlying_type>) {
                return *reinterpret_cast<underlying_type*>(value_);
            } else {
                return const_cast<underlying_type&>(value_);
            }
        }();

        using primary = std::remove_pointer_t<underlying_type>;

        auto info = &type_info::of<primary>();

        void* data = nullptr;
        Flag flag = Flag::None;
        object parent;

        if(policy == return_value_policy::take_ownership) {
            data = &value;
            flag = Flag::Own;
        } else if(policy == return_value_policy::copy) {
            if constexpr(std::is_copy_constructible_v<primary>) {
                data = new auto(value);
                flag = Flag::Own;
            } else {
                std::string msg = "cannot use copy policy on non-copyable type: ";
                msg += type_name<primary>();
                throw std::runtime_error(msg);
            }
        } else if(policy == return_value_policy::move) {
            if constexpr(std::is_move_constructible_v<primary>) {
                data = new auto(std::move(value));
                flag = Flag::Own;
            } else {
                std::string msg = "cannot use move policy on non-moveable type: ";
                msg += type_name<primary>();
                throw std::runtime_error(msg);
            }
        } else if(policy == return_value_policy::reference) {
            data = &value;
            flag = Flag::None;
        } else if(policy == return_value_policy::reference_internal) {
            data = &value;
            flag = Flag::Ref;
            parent = borrow(parent_);
        }

        object result(object::alloc_t{});
        void* temp = py_newobject(result.ptr(), type.index(), 1, sizeof(instance));
        new (temp) instance{flag, data, info, std::move(parent)};
        return result;
    }

    ~instance() {
        if(flag & Flag::Own) {
            info->destructor(data);
        }
    }

    template <typename T>
    T& as() noexcept {
        return *static_cast<T*>(data);
    }
};

}  // namespace pkbind
