#pragma once

#include "kernel.h"

namespace pkbind {

class handle;
class object;
class iterator;
class str;

struct arg;
struct arg_with_default;
struct args_proxy;

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

template <typename policy>
class accessor;

namespace policy {

struct attr;
template <typename Key>
struct item;
struct tuple;
struct list;
template <typename Key>
struct dict;

}  // namespace policy

using attr_accessor = accessor<policy::attr>;
template <typename Key>
using item_accessor = accessor<policy::item<Key>>;
using tuple_accessor = accessor<policy::tuple>;
using list_accessor = accessor<policy::list>;
template <typename Key>
using dict_accessor = accessor<policy::dict<Key>>;

/// call a pkpy function which may raise a python exception.
template <auto Fn, typename... Args>
auto raise_call(Args&&... args);

/// an effective representation of a python small string.
class name {
public:
    name() = default;

    name(const name&) = default;

    name& operator= (const name&) = default;

    explicit name(py_Name data) : data(data) {}

    name(const char* str) : data(py_name(str)) {}

    name(const char* data, int size) : data(py_namev({data, size})) {}

    name(std::string_view str) : name(str.data(), static_cast<int>(str.size())) {}

    name(handle h);

    py_Name index() const { return data; }

    const char* c_str() const { return py_name2str(data); }

    operator std::string_view () const {
        auto temp = py_name2sv(data);
        return std::string_view(temp.data, temp.size);
    }

private:
    py_Name data;
};

template <typename Derived>
class interface {
public:
    /// equal to `self is None` in python.
    bool is_none() const { return py_isnone(ptr()); }

    /// equal to `self is other` in python.
    bool is(const interface& other) const { return py_isidentical(ptr(), other.ptr()); }

    void assign(const interface& other) { py_assign(ptr(), other.ptr()); }

    iterator begin() const;
    iterator end() const;

    attr_accessor attr(name key) const;

    object operator- () const;
    object operator~() const;
    args_proxy operator* () const;

    item_accessor<int> operator[] (int index) const;
    item_accessor<name> operator[] (name key) const;
    item_accessor<handle> operator[] (handle key) const;

    template <return_value_policy policy = return_value_policy::automatic, typename... Args>
    object operator() (Args&&... args) const;

    auto doc() const { return attr("__doc__"); }

    template <typename T>
    T cast() const;

private:
    py_Ref ptr() const { return static_cast<const Derived*>(this)->ptr(); }
};

/// a simple wrapper to py_Ref.
/// Note that it does not manage the lifetime of the object.
class handle : public interface<handle> {
public:
    handle() = default;

    handle(const handle&) = default;

    handle& operator= (const handle&) = default;

    handle(py_Ref ptr) : m_ptr(ptr) {}

    auto ptr() const { return m_ptr; }

    explicit operator bool () const { return m_ptr != nullptr; }

protected:
    py_Ref m_ptr = nullptr;
};

class object : public handle {
public:
    object() = default;

    object(const object& other) : handle(other), m_index(other.m_index) {
        if(other.in_pool()) { object_pool::inc_ref(other); }
    }

    object(object&& other) : handle(other), m_index(other.m_index) {
        other.m_ptr = nullptr;
        other.m_index = -1;
    }

    object& operator= (const object& other) {
        if(this != &other) {
            if(in_pool()) { object_pool::dec_ref(*this); }
            if(other.in_pool()) { object_pool::inc_ref(other); }
            m_ptr = other.m_ptr;
            m_index = other.m_index;
        }
        return *this;
    }

    object& operator= (object&& other) {
        if(this != &other) {
            if(in_pool()) { object_pool::dec_ref(*this); }
            m_ptr = other.m_ptr;
            m_index = other.m_index;
            other.m_ptr = nullptr;
            other.m_index = -1;
        }
        return *this;
    }

    ~object() {
        if(in_pool()) { object_pool::dec_ref(*this); }
    }

    bool is_singleton() const { return m_ptr && m_index == -1; }

    bool empty() const { return m_ptr == nullptr && m_index == -1; }

    /// return whether the object is in the object pool.
    bool in_pool() const { return m_ptr && m_index != -1; }

public:
    static auto type_or_check() { return tp_object; }

    struct alloc_t {};

    struct realloc_t {};

    struct ref_t {};

    object(alloc_t) {
        auto ref = object_pool::alloc();
        m_ptr = ref.data;
        m_index = ref.index;
    }

    object(handle h, realloc_t) {
        auto ref = object_pool::realloc(h.ptr());
        m_ptr = ref.data;
        m_index = ref.index;
    }

    object(handle h, ref_t) : handle(h) {}

    static object from_ret() { return object(py_retval(), realloc_t{}); }

    operator object_pool::object_ref () const { return {m_ptr, m_index}; }

    explicit operator bool () const;

protected:
    int m_index = -1;
};

template <typename T = object>
T steal(handle h) {
    return T(h, object::ref_t{});
}

template <typename T = object>
T borrow(handle h) {
    return T(h, object::realloc_t{});
}

static_assert(std::is_trivially_copyable_v<name>);
static_assert(std::is_trivially_copyable_v<handle>);

}  // namespace pkbind
