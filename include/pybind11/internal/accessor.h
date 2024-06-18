#pragma once
#include "builtins.h"

namespace pybind11 {

// implement iterator methods for interface
template <typename Derived>
inline iterator interface<Derived>::begin() const {
    return handle(vm->py_iter(this->ptr()));
}

template <typename Derived>
inline iterator interface<Derived>::end() const {
    return iterator::sentinel();
}

template <typename Derived>
inline str interface<Derived>::package() const {
    return handle(this->attr(pkpy::__package__));
}

template <typename Derived>
inline str interface<Derived>::name() const {
    return handle(this->attr(pkpy::__name__));
}

template <typename Derived>
inline str interface<Derived>::repr() const {
    return handle(str(vm->py_repr(this->ptr())));
}

template <typename policy>
class accessor : public interface<accessor<policy>> {

    using key_type = typename policy::key_type;

    handle m_obj;
    mutable handle m_value;
    key_type m_key;

    friend interface<handle>;
    friend interface<accessor<policy>>;
    friend tuple;
    friend list;
    friend dict;

    accessor(const handle& obj, key_type key) : m_obj(obj), m_value(), m_key(key) {}

public:
    pkpy::PyVar ptr() const {
        if(!m_value) { m_value = policy::get(m_obj, m_key); }
        return m_value.ptr();
    }

    template <typename Value>
    accessor& operator= (Value&& value) && {
        policy::set(m_obj, m_key, std::forward<Value>(value));
        return *this;
    }

    template <typename Value>
    accessor& operator= (Value&& value) & {
        m_value = std::forward<Value>(value);
        return *this;
    }

    template <typename T>
    T cast() const {
        return operator handle ().template cast<T>();
    }

    operator handle () const { return ptr(); }
};

namespace policy {
struct attr {
    using key_type = pkpy::StrName;

    static handle get(const handle& obj, pkpy::StrName key) { return vm->getattr(obj.ptr(), key); }

    static void set(const handle& obj, pkpy::StrName key, const handle& value) {
        vm->setattr(obj.ptr(), key, value.ptr());
    }
};

struct item {
    using key_type = handle;

    static handle get(const handle& obj, const handle& key) {
        return vm->call(vm->py_op("getitem"), obj.ptr(), key.ptr());
    }

    static void set(const handle& obj, const handle& key, const handle& value) {
        vm->call(vm->py_op("setitem"), obj.ptr(), key.ptr(), value.ptr());
    }
};

struct tuple {
    using key_type = int;

    static handle get(const handle& obj, int key) { return obj._as<pkpy::Tuple>()[key]; }

    static void set(const handle& obj, size_t key, const handle& value) { obj._as<pkpy::Tuple>()[key] = value.ptr(); }
};

struct list {
    using key_type = int;

    static handle get(const handle& obj, size_t key) { return obj._as<pkpy::List>()[key]; }

    static void set(const handle& obj, size_t key, const handle& value) { obj._as<pkpy::List>()[key] = value.ptr(); }
};

struct dict {
    using key_type = handle;

    static handle get(const handle& obj, const handle& key) { return obj.cast<pybind11::dict>().getitem(key); }

    static void set(const handle& obj, const handle& key, const handle& value) {
        obj.cast<pybind11::dict>().setitem(key, value);
    }
};

}  // namespace policy

// implement other methods of interface

template <typename Derived>
inline attr_accessor interface<Derived>::attr(pkpy::StrName key) const {
    return attr_accessor(this->ptr(), key);
}

template <typename Derived>
inline attr_accessor interface<Derived>::attr(const char* key) const {
    return attr_accessor(this->ptr(), pkpy::StrName(key));
}

template <typename Derived>
inline attr_accessor interface<Derived>::attr(const handle& key) const {
    return attr_accessor(this->ptr(), pkpy::StrName(key._as<pkpy::Str>()));
}

template <typename Derived>
inline attr_accessor interface<Derived>::doc() const {
    return attr_accessor(this->ptr(), pkpy::StrName("__doc__"));
}

template <typename Derived>
inline item_accessor interface<Derived>::operator[] (int index) const {
    return item_accessor(this->ptr(), int_(index));
}

template <typename Derived>
inline item_accessor interface<Derived>::operator[] (const char* key) const {
    return item_accessor(this->ptr(), str(key));
}

template <typename Derived>
inline item_accessor interface<Derived>::operator[] (const handle& key) const {
    return item_accessor(this->ptr(), key);
}

inline tuple_accessor tuple::operator[] (int i) const { return tuple_accessor(this->ptr(), i); }

inline list_accessor list::operator[] (int i) const { return list_accessor(this->ptr(), i); }

inline dict_accessor dict::operator[] (int index) const { return dict_accessor(this->ptr(), int_(index)); }

inline dict_accessor dict::operator[] (std::string_view key) const { return dict_accessor(this->ptr(), str(key)); }

inline dict_accessor dict::operator[] (const handle& key) const { return dict_accessor(this->ptr(), key); }

}  // namespace pybind11
