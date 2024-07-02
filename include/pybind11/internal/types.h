#pragma once
#include "object.h"

namespace pybind11 {

template <typename T>
handle cast(T&& value, return_value_policy policy = return_value_policy::automatic_reference, handle parent = {});

struct arg {
    const char* name;
    handle default_;

    arg(const char* name) : name(name), default_() {}

    template <typename T>
    arg& operator= (T&& value) {
        default_ = cast(std::forward<T>(value));
        return *this;
    }
};

// undef in pybind11.h
#define PYBIND11_REGISTER_INIT(func)                                                                                   \
    static inline int _register = [] {                                                                                 \
        interpreter::register_init(func);                                                                              \
        return 0;                                                                                                      \
    }();

class none : public object {
#if PK_VERSION_MAJOR == 2
    PYBIND11_TYPE_IMPLEMENT(object, empty, vm->tp_none_type);
#else
    PYBIND11_TYPE_IMPLEMENT(object, empty, [](const handle& obj) {
        return obj.is_none();
    });
#endif

public:
    none() : object(vm->None) {}
};

/// corresponding to type in Python
class type : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Type, vm->tp_type);

public:
    template <typename T>
    static handle handle_of() {
        return type_visitor::type<T>();
    }

    template <typename T>
    static type of() {
        return type_visitor::type<T>();
    }

    static type of(const handle& obj) { return type(vm->_t(obj.ptr())); }
};

/// corresponding to bool in Python
class bool_ : public object {
    PYBIND11_TYPE_IMPLEMENT(object, bool, vm->tp_bool);

public:
    bool_(bool value) : object(create(value)) {}

    operator bool () const { return self(); }
};

/// corresponding to int in Python
class int_ : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::i64, vm->tp_int);

public:
    int_(int64_t value) : object(create(value)) {}

    operator int64_t () const { return self(); }
};

/// corresponding to float in Python
class float_ : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::f64, vm->tp_float);

public:
    float_(double value) : object(create(value)) {}

    operator double () const { return self(); }
};

class iterable : public object {
    PYBIND11_TYPE_IMPLEMENT(object, empty, [](const handle& obj) {
        return vm->getattr(obj.ptr(), pkpy::__iter__, false) != nullptr;
    });
};

class iterator : public object {
    PYBIND11_TYPE_IMPLEMENT(object, empty, [](const handle& obj) {
        return vm->getattr(obj.ptr(), pkpy::__next__, false) != nullptr &&
               vm->getattr(obj.ptr(), pkpy::__iter__, false) != nullptr;
    });

    handle m_value;

    iterator(pkpy::PyVar n, pkpy::PyVar s) : object(n), m_value(s) {}

public:
    iterator(const handle& obj) : object(obj) { m_value = vm->py_next(obj.ptr()); }

    iterator operator++ () {
        m_value = vm->py_next(m_ptr);
        return *this;
    }

    iterator operator++ (int) {
        m_value = vm->py_next(m_ptr);
        return *this;
    }

    const handle& operator* () const { return m_value; }

    friend bool operator== (const iterator& lhs, const iterator& rhs) { return lhs.m_value.is(rhs.m_value); }

    friend bool operator!= (const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

    static iterator sentinel() { return iterator(vm->None, vm->StopIteration); }
};

class str : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Str, vm->tp_str);

public:
    str(const char* c, int len) : object(create(c, len)) {};

    str(const char* c = "") : str(c, strlen(c)) {}

    str(const std::string& s) : str(s.data(), s.size()) {}

    str(std::string_view sv) : str(sv.data(), sv.size()) {}

    // explicit str(const bytes& b);
    explicit str(handle h);

    operator std::string_view () const { return self().sv(); }

    template <typename... Args>
    str format(Args&&... args) const;
};

// class bytes : public object {
// public:
//     using object::object;
// };

// class bytearray : public object {
// public:
//     using object::object;
// };

class tuple : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Tuple, vm->tp_tuple);

public:
    tuple(int n) : object(create(n)) {}

    template <typename... Args, std::enable_if_t<(sizeof...(Args) > 1)>* = nullptr>
    tuple(Args&&... args) : object(create(sizeof...(Args))) {
        int index = 0;
        ((self()[index++] = pybind11::cast(std::forward<Args>(args)).ptr()), ...);
    }

    int size() const { return self().size(); }

    bool empty() const { return size() == 0; }

    tuple_accessor operator[] (int i) const;
};

class list : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::List, vm->tp_list)

public:
    list() : object(create(0)) {}

    list(int n) : object(create(n)) {}

    template <typename... Args, std::enable_if_t<(sizeof...(Args) > 1)>* = nullptr>
    list(Args&&... args) : object(create(sizeof...(Args))) {
        int index = 0;
        ((self()[index++] = pybind11::cast(std::forward<Args>(args)).ptr()), ...);
    }

    int size() const { return self().size(); }

    bool empty() const { return size() == 0; }

    void clear() { self().clear(); }

    list_accessor operator[] (int i) const;

    void append(const handle& obj) { self().push_back(obj.ptr()); }

    void extend(const handle& iterable) {
        for(auto& item: iterable) {
            append(item);
        }
    }

    void insert(int index, const handle& obj) {
#if PK_VERSION_MAJOR == 2
        const auto pos = self().begin() + index;
        self().insert(pos, obj.ptr());
#else
        self().insert(index, obj.ptr());
#endif
    }
};

class slice : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Slice, vm->tp_slice);

public:
};

// class set : public object {
// public:
//     using object::object;
//     // set() : object(vm->new_object<pkpy::Se>(pkpy::VM::tp_set), true) {}
// };
//

class dict : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Dict, vm->tp_dict);

public:
#if PK_VERSION_MAJOR == 2
    dict() : object(create()) {}

    template <typename... Args, typename = std::enable_if_t<(std::is_same_v<remove_cvref_t<Args>, arg> && ...)>>
    dict(Args&&... args) : object(create()) {
        auto foreach_ = [&](pybind11::arg& arg) {
            setitem(str(arg.name), arg.default_);
        };
        (foreach_(args), ...);
    }

    void setitem(const handle& key, const handle& value) { self().set(vm, key.ptr(), value.ptr()); }

    handle getitem(const handle& key) const { return self().try_get(vm, key.ptr()); }

    struct iterator {
        pkpy_DictIter iter;
        std::pair<handle, handle> value;

        iterator operator++ () {
            bool is_ended = pkpy_DictIter__next(&iter, (PyVar*)&value.first, (PyVar*)&value.second);
            if(!is_ended) {
                iter._dict = nullptr;
                iter._index = -1;
            }
            return *this;
        }

        std::pair<handle, handle> operator* () const { return value; }

        bool operator== (const iterator& other) const {
            return iter._dict == other.iter._dict && iter._index == other.iter._index;
        }

        bool operator!= (const iterator& other) const { return !(*this == other); }
    };

    iterator begin() const {
        iterator iter{self().iter(), {}};
        ++iter;
        return iter;
    }

    iterator end() const { return {nullptr, -1}; }
#else
    dict() : object(create(vm)) {}

    template <typename... Args, typename = std::enable_if_t<(std::is_same_v<remove_cvref_t<Args>, arg> && ...)>>
    dict(Args&&... args) : object(create(vm)) {
        auto foreach_ = [&](pybind11::arg& arg) {
            setitem(str(arg.name), arg.default_);
        };
        (foreach_(args), ...);
    }

    void setitem(const handle& key, const handle& value) { self().set(key.ptr(), value.ptr()); }

    handle getitem(const handle& key) const { return self().try_get(key.ptr()); }

    struct iterator {
        pkpy::Dict::Item* items;
        pkpy::Dict::ItemNode* nodes;
        int index;

        iterator operator++ () {
            index = nodes[index].next;
            if(index == -1) {
                items = nullptr;
                nodes = nullptr;
            }
            return *this;
        }

        std::pair<handle, handle> operator* () const { return {items[index].first, items[index].second}; }

        bool operator== (const iterator& other) const {
            return items == other.items && nodes == other.nodes && index == other.index;
        }

        bool operator!= (const iterator& other) const { return !(*this == other); }
    };

    iterator begin() const {
        auto index = self()._head_idx;
        if(index == -1) {
            return end();
        } else {
            return {self()._items, self()._nodes, index};
        }
    }

    iterator end() const { return {nullptr, nullptr, -1}; }

    template <typename Key>
    bool contains(Key&& key) const {
        return self().contains(vm, pybind11::cast(std::forward<Key>(key)).ptr());
    }
#endif

    int size() const { return self().size(); }

    bool empty() const { return size() == 0; }

    void clear() { self().clear(); }

    dict_accessor operator[] (int index) const;
    dict_accessor operator[] (std::string_view) const;
    dict_accessor operator[] (const handle& key) const;
};

class function : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Function, vm->tp_function);
};

//
// class buffer : public object {
// public:
//    using object::object;
//};
//
// class memory_view : public object {
// public:
//    using object::object;
//};
//
class capsule : public object {
    PYBIND11_REGISTER_INIT([] {
        type_visitor::create<impl::capsule>(vm->builtins, "capsule", true);
    });

    PYBIND11_TYPE_IMPLEMENT(object, impl::capsule, handle(vm->builtins->attr("capsule"))._as<pkpy::Type>());

public:
    capsule(void* ptr, void (*destructor)(void*) = nullptr) : object(create(ptr, destructor)) {}

    void* data() const { return self().ptr; }

    template <typename T>
    T& cast() const {
        return *static_cast<T*>(self().ptr);
    }
};

class property : public object {
    PYBIND11_TYPE_IMPLEMENT(object, pkpy::Property, vm->tp_property);

public:
#if PK_VERSION_MAJOR == 2
    property(handle getter, handle setter) : object(create(getter.ptr(), setter.ptr())) {}
#else
    property(handle getter, handle setter) : object(create(pkpy::Property{getter.ptr(), setter.ptr()})) {}
#endif

    handle getter() const { return self().getter; }

    handle setter() const { return self().setter; }
};

class args : public tuple {
    PYBIND11_TYPE_IMPLEMENT(tuple, pybind11::empty, vm->tp_tuple);
};

class kwargs : public dict {
    PYBIND11_TYPE_IMPLEMENT(dict, pybind11::empty, vm->tp_dict);
};

}  // namespace pybind11
