#pragma once

#include "object.h"

namespace pybind11 {
    class type : public object {
    public:
        using object::object;
        template <typename T>
        static handle handle_of();
    };

    class iterable : public object {
    public:
        using object::object;
        iterable() = delete;
    };

    class iterator : public object {
    public:
        using object::object;
        iterator() = delete;
    };

    class list : public object {
    public:
        using object::object;

        list() : object(vm->new_object<pkpy::List>(pkpy::VM::tp_list), true) {}
    };

    class tuple : public object {
    public:
        using object::object;

        tuple(int n) : object(vm->new_object<pkpy::Tuple>(pkpy::VM::tp_tuple, n), true) {}

        //& operator[](int i){ return _args[i]; }
        // PyVar operator[](int i) const { return _args[i]; }
    };

    class set : public object {
    public:
        using object::object;
        // set() : object(vm->new_object<pkpy::Se>(pkpy::VM::tp_set), true) {}
    };

    class dict : public object {
    public:
        using object::object;

        dict() : object(vm->new_object<pkpy::Dict>(pkpy::VM::tp_dict), true) {}
    };

    class str : public object {

    public:
        using object::object;
        str(const char* c, int len) :
            object(vm->new_object<pkpy::Str>(pkpy::VM::tp_str, c, len), true) {

            };

        str(const char* c = "") : str(c, strlen(c)) {}

        str(const std::string& s) : str(s.data(), s.size()) {}

        str(std::string_view sv) : str(sv.data(), sv.size()) {}

        explicit str(const bytes& b);
        explicit str(handle h);
        operator std::string () const;

        template <typename... Args>
        str format(Args&&... args) const;
    };

    class int_ : public object {
    public:
        using object::object;

        int_(int64_t value) : object(pkpy::py_var(vm, value), true) {}
    };

    class float_ : public object {
    public:
        using object::object;

        float_(double value) : object(pkpy::py_var(vm, value), true) {}
    };

    class bool_ : public object {
    public:
        using object::object;

        bool_(bool value) : object(pkpy::py_var(vm, value), true) {}
    };

    class function : public object {
    public:
        using object::object;
    };

    class attr_accessor : public object {
    private:
        object key;

    public:
        template <typename T>
        attr_accessor(const object& obj, T&& key) : object(obj), key(std::forward<T>(key)){};

        template <typename T>
        attr_accessor& operator= (T&& value) & {
            static_assert(std::is_base_of_v<object, std::decay_t<T>>,
                          "T must be derived from object");
            m_ptr = std::forward<T>(value);
            return *this;
        }

        template <typename T>
        attr_accessor& operator= (T&& value) && {
            static_assert(std::is_base_of_v<object, std::decay_t<T>>,
                          "T must be derived from object");
            setattr(*this, key, std::forward<T>(value));
            return *this;
        }
    };

    inline attr_accessor handle::attr(const char* name) const {
        return attr_accessor(reinterpret_borrow<object>(*this), str(name));
    }

    inline attr_accessor handle::attr(const handle& name) const {
        return attr_accessor(reinterpret_borrow<object>(*this), reinterpret_borrow<object>(name));
    }

    inline attr_accessor handle::attr(object&& name) const {
        return attr_accessor(reinterpret_borrow<object>(*this), std::move(name));
    }

    class item_accessor : public object {
    public:
        object key;

    public:
        template <typename T>
        item_accessor(const object& obj, T&& key) : object(obj), key(std::forward<T>(key)){};

        template <typename T>
        item_accessor& operator= (T&& value) & {
            static_assert(std::is_base_of_v<object, std::decay_t<T>>,
                          "T must be derived from object");
            m_ptr = std::forward<T>(value);
        }

        template <typename T>
        item_accessor& operator= (object&& value) && {
            static_assert(std::is_base_of_v<object, std::decay_t<T>>,
                          "T must be derived from object");
            setitem(*this, key, std::forward<T>(value));
        }
    };

    inline item_accessor handle::operator[] (int64_t key) const {
        return item_accessor(reinterpret_borrow<object>(*this), int_(key));
    }

    inline item_accessor handle::operator[] (const char* key) const {
        return item_accessor(reinterpret_borrow<object>(*this), str(key));
    }

    inline item_accessor handle::operator[] (const handle& key) const {
        return item_accessor(reinterpret_borrow<object>(*this), reinterpret_borrow<object>(key));
    }

    inline item_accessor handle::operator[] (object&& key) const {
        return item_accessor(reinterpret_borrow<object>(*this), std::move(key));
    }

    class args : public tuple {
        using tuple::tuple;
    };

    class kwargs : public dict {
        using dict::dict;
    };

    template <typename T>
    handle type::handle_of() {
        if constexpr(std::is_same_v<T, object>) {
            return vm->_t(vm->tp_object);
        }
#define PYBIND11_TYPE_MAPPER(type, tp)                                                             \
    else if constexpr(std::is_same_v<T, type>) {                                                   \
        return vm->_t(vm->tp);                                                                     \
    }
        PYBIND11_TYPE_MAPPER(type, tp_type)
        PYBIND11_TYPE_MAPPER(str, tp_str)
        PYBIND11_TYPE_MAPPER(int_, tp_int)
        PYBIND11_TYPE_MAPPER(float_, tp_float)
        PYBIND11_TYPE_MAPPER(bool_, tp_bool)
        PYBIND11_TYPE_MAPPER(list, tp_list)
        PYBIND11_TYPE_MAPPER(tuple, tp_tuple)
        PYBIND11_TYPE_MAPPER(args, tp_tuple)
        PYBIND11_TYPE_MAPPER(dict, tp_dict)
        PYBIND11_TYPE_MAPPER(kwargs, tp_dict)
#undef PYBIND11_TYPE_MAPPER
        else {
            auto result = vm->_cxx_typeid_map.find(typeid(T));
            if(result != vm->_cxx_typeid_map.end()) {
                return vm->_t(result->second);
            }

            vm->TypeError("Type not registered");
        }
    }

}  // namespace pybind11
