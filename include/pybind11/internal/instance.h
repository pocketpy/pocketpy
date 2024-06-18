#pragma once
#include "kernel.h"

namespace pybind11 {

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
                ((T*)ptr)->~T();
                operator delete (ptr);
            },
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

public:
    instance() noexcept : flag(Flag::None), data(nullptr), type(nullptr), parent(nullptr) {}

    instance(const instance&) = delete;

    instance(instance&& other) noexcept : flag(other.flag), data(other.data), type(other.type), parent(other.parent) {
        other.flag = Flag::None;
        other.data = nullptr;
        other.type = nullptr;
        other.parent = nullptr;
    }

    static pkpy::PyVar create(pkpy::Type type, const type_info* info) noexcept {
        instance instance;
        instance.type = info;
        instance.data = operator new (info->size);
        instance.flag = Flag::Own;
        return vm->heap.gcnew<pybind11::instance>(type, std::move(instance));
    }

    template <typename T>
    static pkpy::PyVar create(T&& value,
                              pkpy::Type type,
                              return_value_policy policy = return_value_policy::automatic_reference,
                              pkpy::PyVar parent = nullptr) noexcept {
        using underlying_type = remove_cvref_t<T>;

        auto& _value = [&]() -> auto& {
            // note that, pybind11 will ignore the const qualifier.
            // in fact, try to modify a const value will result in undefined behavior.
            if constexpr(std::is_pointer_v<underlying_type>) {
                return *reinterpret_cast<underlying_type*>(value);
            } else {
                return const_cast<underlying_type&>(value);
            }
        }();

        using primary = std::remove_pointer_t<underlying_type>;
        instance instance;
        instance.type = &type_info::of<primary>();

        if(policy == return_value_policy::take_ownership) {
            instance.data = &_value;
            instance.flag = Flag::Own;
        } else if(policy == return_value_policy::copy) {
            if constexpr(std::is_copy_constructible_v<primary>) {
                instance.data = ::new auto(_value);
                instance.flag = Flag::Own;
            } else {
                std::string msg = "cannot use copy policy on non-copyable type: ";
                msg += type_name<primary>();
                vm->RuntimeError(msg);
            }
        } else if(policy == return_value_policy::move) {
            if constexpr(std::is_move_constructible_v<primary>) {
                instance.data = ::new auto(std::move(_value));
                instance.flag = Flag::Own;
            } else {
                std::string msg = "cannot use move policy on non-moveable type: ";
                msg += type_name<primary>();
                vm->RuntimeError(msg);
            }
        } else if(policy == return_value_policy::reference) {
            instance.data = &_value;
            instance.flag = Flag::None;
        } else if(policy == return_value_policy::reference_internal) {
            instance.data = &_value;
            instance.flag = Flag::Ref;
            instance.parent = parent;
        }

        return vm->heap.gcnew<pybind11::instance>(type, std::move(instance));
    }

    ~instance() {
        if(flag & Flag::Own) { type->destructor(data); }
    }

    template <typename T>
    T& _as() noexcept {
        return *static_cast<T*>(data);
    }

#if PK_VERSION_MAJOR == 2
    void _gc_mark(pkpy::VM* vm) const noexcept {
        if(parent && (flag & Flag::Ref)) { PK_OBJ_MARK(parent); }
    }
#else
    void _gc_mark() const noexcept {
        if(parent && (flag & Flag::Ref)) { PK_OBJ_MARK(parent); }
    }
#endif
};

}  // namespace pybind11
