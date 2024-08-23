#pragma once

#include "builtins.h"

namespace pkbind {

template <typename policy>
class accessor : public interface<accessor<policy>> {
    using key_type = typename policy::key_type;

    friend interface<handle>;
    friend interface<accessor<policy>>;
    friend tuple;
    friend list;
    friend dict;

    accessor(handle obj, key_type key) : m_obj(obj), m_value(), m_key(key) {}

public:
    auto ptr() const {
        if(m_value.empty()) {
            m_value = borrow(policy::get(m_obj, m_key));
        }
        return m_value.ptr();
    }

    template <typename Value>
    accessor& operator= (Value&& value) && {
        policy::set(m_obj, m_key, pkbind::cast(std::forward<Value>(value)));
        return *this;
    }

    template <typename Value>
    accessor& operator= (Value&& value) & {
        m_value = std::forward<Value>(value);
        return *this;
    }

    operator handle () const { return ptr(); }

private:
    handle m_obj;
    mutable object m_value;
    key_type m_key;
};

namespace policy {

struct attr {
    using key_type = name;

    static handle get(handle obj, name key) {
        raise_call<py_getattr>(obj.ptr(), key.index());
        return py_retval();
    }

    static void set(handle obj, name key, handle value) { raise_call<py_setattr>(obj.ptr(), key.index(), value.ptr()); }
};

template <typename Key>
struct item {
    using key_type = Key;

    static handle get(handle obj, int key) { return get(obj, int_(key)); }

    static handle get(handle obj, name key) { return get(obj, str(key)); }

    static handle get(handle obj, handle key) {
        raise_call<py_getitem>(obj.ptr(), key.ptr());
        return py_retval();
    }

    static void set(handle obj, int key, handle value) { set(obj, int_(key), value); }

    static void set(handle obj, name key, handle value) { set(obj, str(key), value); }

    static void set(handle obj, handle key, handle value) { raise_call<py_setitem>(obj.ptr(), key.ptr(), value.ptr()); }
};

struct tuple {
    using key_type = int;

    static handle get(handle obj, int key) { return py_tuple_getitem(obj.ptr(), key); }

    static void set(handle obj, int key, handle value) { py_tuple_setitem(obj.ptr(), key, value.ptr()); }
};

struct list {
    using key_type = int;

    static handle get(handle obj, int key) { return py_list_getitem(obj.ptr(), key); }

    static void set(handle obj, int key, handle value) { py_list_setitem(obj.ptr(), key, value.ptr()); }
};

template <typename Key>
struct dict {
    using key_type = Key;

    static handle get(handle obj, int key) { return get(obj, int_(key)); }

    static handle get(handle obj, name key) { return get(obj, str(key)); }

    static handle get(handle obj, handle key) {
        raise_call<py_dict_getitem>(obj.ptr(), key.ptr());
        return py_retval();
    }

    static void set(handle obj, int key, handle value) { set(obj, int_(key), value); }

    static void set(handle obj, name key, handle value) { set(obj, str(key), value); }

    static void set(handle obj, handle key, handle value) {
        raise_call<py_dict_setitem>(obj.ptr(), key.ptr(), value.ptr());
    }
};

}  // namespace policy

// implement other methods of interface

template <typename Derived>
inline attr_accessor interface<Derived>::attr(name key) const {
    return {ptr(), key};
}

template <typename Derived>
inline item_accessor<int> interface<Derived>::operator[] (int key) const {
    return {ptr(), key};
}

template <typename Derived>
inline item_accessor<name> interface<Derived>::operator[] (name key) const {
    return {ptr(), key};
}

template <typename Derived>
inline item_accessor<handle> interface<Derived>::operator[] (handle key) const {
    return {ptr(), key};
}

template <typename... Args>
object str::format(Args&&... args) {
    return attr("format")(std::forward<Args>(args)...);
}

inline tuple_accessor tuple::operator[] (int index) const { return {m_ptr, index}; }

inline list_accessor list::operator[] (int index) const { return {m_ptr, index}; };

inline dict_accessor<int> dict::operator[] (int key) const { return {m_ptr, key}; }

inline dict_accessor<name> dict::operator[] (name key) const { return {m_ptr, key}; }

inline dict_accessor<handle> dict::operator[] (handle key) const { return {m_ptr, key}; }

inline dict::iterator::iterator(handle h) : items(h.attr("items")()), iter(items.begin()) {}

inline std::pair<object, object> dict::iterator::operator* () const {
    tuple pair = *iter;
    return {borrow(pair[0]), borrow(pair[1])};
}

}  // namespace pkbind
