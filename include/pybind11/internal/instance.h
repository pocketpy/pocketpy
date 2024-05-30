#pragma once

#include "kernel.h"

namespace pybind11 {
    struct type_info {
        const char* name;
        std::size_t size;
        std::size_t alignment;
        void (*destructor)(void*);
        void (*copy)(void*, const void*);
        void (*move)(void*, void*);
        const std::type_info* type;

        template <typename T>
        static type_info& of() {
            static_assert(!std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>,
                          "T must not be a reference type or const type.");
            static type_info info = {
                typeid(T).name(),
                sizeof(T),
                alignof(T),
                [](void* ptr) {
                    ((T*)ptr)->~T();
                    operator delete (ptr);
                },
                [](void* dst, const void* src) { new (dst) T(*(const T*)src); },
                [](void* dst, void* src) { new (dst) T(std::move(*(T*)src)); },
                &typeid(T),
            };
            return info;
        }
    };

    // all registered C++ class will be ensured as instance type.
    class instance {
    public:
        // use to record the type information of C++ class.

    private:
        enum Flag {
            None = 0,
            Own = 1 << 0,  // if the instance is owned by C++ side.
            Ref = 1 << 1,  // need to mark the parent object.
        };

        Flag flag;
        void* data;
        const type_info* type;
        pkpy::PyVar parent;
        // pkpy::PyVar

    public:
        instance() noexcept : flag(Flag::None), data(nullptr), type(nullptr), parent(nullptr) {}

        instance(const instance&) = delete;

        instance(instance&& other) noexcept :
            flag(other.flag), data(other.data), type(other.type), parent(other.parent) {
            other.flag = Flag::None;
            other.data = nullptr;
            other.type = nullptr;
            other.parent = nullptr;
        }

        template <typename T>
        static pkpy::PyVar create(pkpy::Type type) {
            instance instance;
            instance.type = &type_info::of<T>();
            instance.data = operator new (sizeof(T));
            instance.flag = Flag::Own;
            return vm->new_object<pybind11::instance>(type, std::move(instance));
        }

        template <typename T>
        static pkpy::PyVar
            create(T&& value,
                   pkpy::Type type,
                   return_value_policy policy = return_value_policy::automatic_reference,
                   pkpy::PyVar parent = nullptr) noexcept {
            using underlying_type = std::remove_cv_t<std::remove_reference_t<T>>;

            // resolve for automatic policy.
            if(policy == return_value_policy::automatic) {
                policy = std::is_pointer_v<underlying_type> ? return_value_policy::take_ownership
                         : std::is_lvalue_reference_v<T&&>  ? return_value_policy::copy
                                                            : return_value_policy::move;
            } else if(policy == return_value_policy::automatic_reference) {
                policy = std::is_pointer_v<underlying_type> ? return_value_policy::reference
                         : std::is_lvalue_reference_v<T&&>  ? return_value_policy::copy
                                                            : return_value_policy::move;
            }

            auto& _value = [&]() -> auto& {
                /**
                 * note that, pybind11 will ignore the const qualifier.
                 * in fact, try to modify a const value will result in undefined behavior.
                 */
                if constexpr(std::is_pointer_v<underlying_type>) {
                    return *reinterpret_cast<underlying_type*>(value);
                } else {
                    return const_cast<underlying_type&>(value);
                }
            }();

            instance instance;
            instance.type = &type_info::of<underlying_type>();

            if(policy == return_value_policy::take_ownership) {
                instance.data = &_value;
                instance.flag = Flag::Own;
            } else if(policy == return_value_policy::copy) {
                instance.data = ::new auto(_value);
                instance.flag = Flag::Own;
            } else if(policy == return_value_policy::move) {
                instance.data = ::new auto(std::move(_value));
                instance.flag = Flag::Own;
            } else if(policy == return_value_policy::reference) {
                instance.data = &_value;
                instance.flag = Flag::None;
            } else if(policy == return_value_policy::reference_internal) {
                instance.data = &_value;
                instance.flag = Flag::Ref;
                instance.parent = parent;
            }

            return vm->new_object<pybind11::instance>(type, std::move(instance));
        }

        ~instance() {
            if(flag & Flag::Own) {
                type->destructor(data);
            }
        }

        void _gc_mark(pkpy::VM* vm) const noexcept {
            if(parent && (flag & Flag::Ref)) {
                PK_OBJ_MARK(parent);
            }
        }

        template <typename T>
        T& cast() noexcept {
            return *static_cast<T*>(data);
        }
    };
}  // namespace pybind11
