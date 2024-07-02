#pragma once
#include <vector>
#include <cassert>
#include <pocketpy.h>

#include "type_traits.h"

namespace pybind11::impl {
struct capsule {
    void* ptr;
    void (*destructor)(void*);

    template <typename T, typename = std::enable_if_t<!(std::is_same_v<remove_cvref_t<T>, capsule>)>>
    capsule(T&& value) :
        ptr(new auto(std::forward<T>(value))), destructor([](void* ptr) {
            delete static_cast<std::decay_t<T>*>(ptr);
        }) {}

    capsule(void* ptr, void (*destructor)(void*)) : ptr(ptr), destructor(destructor) {}

    capsule(const capsule&) = delete;

    capsule(capsule&& other) noexcept : ptr(other.ptr), destructor(other.destructor) {
        other.ptr = nullptr;
        other.destructor = nullptr;
    }

    ~capsule() {
        if(ptr && destructor) destructor(ptr);
    }
};
}  // namespace pybind11::impl

namespace pybind11 {

class handle;
class object;
class iterator;
class str;
struct arg;
struct args_proxy;
struct kwargs_proxy;

inline pkpy::VM* vm = nullptr;

class interpreter {
    inline static std::vector<impl::capsule>* _capsules = nullptr;
    inline static std::vector<void (*)()>* _init = nullptr;

public:
    inline static void initialize(bool enable_os = true) {
        if(vm == nullptr) {
            vm = new pkpy::VM();
            if(_init != nullptr) {
                for(auto& fn: *_init)
                    fn();
            }
        }
    }

    inline static void finalize() {
        if(_capsules != nullptr) {
            delete _capsules;
            _capsules = nullptr;
        }

        if(vm != nullptr) {
            delete vm;
            vm = nullptr;
        }
    }

    template <typename T>
    inline static void* take_ownership(T&& value) {
        if(_capsules == nullptr) _capsules = new std::vector<impl::capsule>();
        _capsules->emplace_back(std::forward<T>(value));
        return _capsules->back().ptr;
    }

    inline static void register_init(void (*init)()) {
        if(_init == nullptr) _init = new std::vector<void (*)()>();
        _init->push_back(init);
    }

    inline static pkpy::PyVar bind_func(pkpy::PyVar scope,
                                        pkpy::StrName name,
                                        int argc,
                                        pkpy::NativeFuncC fn,
                                        pkpy::any any = {},
                                        pkpy::BindType type = pkpy::BindType::DEFAULT) {
#if PK_VERSION_MAJOR == 2
        return vm->bind_func(scope.get(), name, argc, fn, any, type);
#else
        return vm->bind_func(scope, name, argc, fn, std::move(any), type);
#endif
    }

    template <typename... Args>
    inline static handle vectorcall(const handle& self, const handle& func, const Args&... args);
};

template <typename T>
constexpr inline bool need_host = !(std::is_trivially_copyable_v<T> && (sizeof(T) <= 8));

template <typename T>
decltype(auto) unpack(pkpy::ArgsView view) {
    if constexpr(need_host<T>) {
        void* data = pkpy::lambda_get_userdata<void*>(view.begin());
        return *static_cast<T*>(data);
    } else {
        return pkpy::lambda_get_userdata<T>(view.begin());
    }
}

template <typename policy>
class accessor;

namespace policy {
struct attr;
struct item;
struct tuple;
struct list;
struct dict;
}  // namespace policy

using attr_accessor = accessor<policy::attr>;
using item_accessor = accessor<policy::item>;
using tuple_accessor = accessor<policy::tuple>;
using list_accessor = accessor<policy::list>;
using dict_accessor = accessor<policy::dict>;

template <typename T>
T cast(const handle& obj, bool convert = false);

enum class return_value_policy : uint8_t {
    /**
     *  This is the default return value policy, which falls back to the policy
     *  return_value_policy::take_ownership when the return value is a pointer.
     *  Otherwise, it uses return_value::move or return_value::copy for rvalue
     *  and lvalue references, respectively. See below for a description of what
     *  all of these different policies do.
     */
    automatic = 0,

    /**
     *  As above, but use policy return_value_policy::reference when the return
     *  value is a pointer. This is the default conversion policy for function
     *  arguments when calling Python functions manually from C++ code (i.e. via
     *  handle::operator()). You probably won't need to use this.
     */
    automatic_reference,

    /**
     *  Reference an existing object (i.e. do not create a new copy) and take
     *  ownership. Python will call the destructor and delete operator when the
     *  object's reference count reaches zero. Undefined behavior ensues when
     *  the C++ side does the same..
     */
    take_ownership,

    /**
     *  Create a new copy of the returned object, which will be owned by
     *  Python. This policy is comparably safe because the lifetimes of the two
     *  instances are decoupled.
     */
    copy,

    /**
     *  Use std::move to move the return value contents into a new instance
     *  that will be owned by Python. This policy is comparably safe because the
     *  lifetimes of the two instances (move source and destination) are
     *  decoupled.
     */
    move,

    /**
     *  Reference an existing object, but do not take ownership. The C++ side
     *  is responsible for managing the object's lifetime and deallocating it
     *  when it is no longer used. Warning: undefined behavior will ensue when
     *  the C++ side deletes an object that is still referenced and used by
     *  Python.
     */
    reference,

    /**
     *  This policy only applies to methods and properties. It references the
     *  object without taking ownership similar to the above
     *  return_value_policy::reference policy. In contrast to that policy, the
     *  function or property's implicit this argument (called the parent) is
     *  considered to be the the owner of the return value (the child).
     *  pybind11 then couples the lifetime of the parent to the child via a
     *  reference relationship that ensures that the parent cannot be garbage
     *  collected while Python is still using the child. More advanced
     *  variations of this scheme are also possible using combinations of
     *  return_value_policy::reference and the keep_alive call policy
     */
    reference_internal
};

struct empty {};

template <typename... Args>
void print(Args&&... args);

class object;

template <typename T>
constexpr inline bool is_pyobject_v = std::is_base_of_v<object, T>;

#if PK_VERSION_MAJOR == 2
using error_already_set = pkpy::TopLevelException;
#else
using error_already_set = pkpy::Exception;
#endif

inline void setattr(const handle& obj, const handle& name, const handle& value);
inline void setattr(const handle& obj, const char* name, const handle& value);

}  // namespace pybind11
