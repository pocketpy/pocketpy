#pragma once

#include "kernel.h"

namespace pybind11 {
    class handle;
    class object;
    class attr_accessor;
    class item_accessor;
    class iterator;
    class str;
    class bytes;
    class iterable;
    class tuple;
    class dict;
    class list;
    class set;
    class function;
    class module;
    class type;
    class bool_;
    class int_;
    class float_;
    class str;
    class bytes;

    template <typename T>
    T& _builtin_cast(const handle& obj);

    template <typename T>
    T reinterpret_borrow(const handle& h);

    template <typename T>
    T reinterpret_steal(const handle& h);

    class handle {
    protected:
        pkpy::PyVar m_ptr = nullptr;
        mutable int* ref_count = nullptr;

    public:
        handle() = default;
        handle(const handle& h) = default;
        handle& operator= (const handle& other) = default;

        handle(pkpy::PyVar ptr) : m_ptr(ptr) {}

        pkpy::PyVar ptr() const { return m_ptr; }

        int reference_count() const { return ref_count == nullptr ? 0 : *ref_count; }

        const handle& inc_ref() const {
            PK_DEBUG_ASSERT(m_ptr != nullptr);
            if(ref_count == nullptr) {
                auto iter = _ref_counts_map->find(m_ptr);
                if(iter == _ref_counts_map->end()) {
                    ref_count = ::new int(1);
                    _ref_counts_map->insert({m_ptr, ref_count});
                } else {
                    ref_count = iter->second;
                    *ref_count += 1;
                }
            } else {
                *ref_count += 1;
            }
            return *this;
        }

        const handle& dec_ref() const {
            PK_DEBUG_ASSERT(m_ptr != nullptr);
            PK_DEBUG_ASSERT(ref_count != nullptr);

            *ref_count -= 1;
            try {
                if(*ref_count == 0) {
                    _ref_counts_map->erase(m_ptr);
                    ::delete ref_count;
                    ref_count = nullptr;
                }
            } catch(std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }

            return *this;
        }

    public:
        template <typename T>
        T cast() const;

        explicit operator bool () const { return m_ptr.operator bool (); }

        bool is(const handle& other) const { return m_ptr == other.m_ptr; }

        bool is_none() const { return m_ptr == vm->None; }

        bool in(const handle& other) const {
            return pkpy::py_cast<bool>(vm, vm->call(vm->py_op("contains"), other.m_ptr, m_ptr));
        }

        bool contains(const handle& other) const {
            return pkpy::py_cast<bool>(vm, vm->call(vm->py_op("contains"), m_ptr, other.m_ptr));
        }

        iterator begin() const;
        iterator end() const;

        str doc() const;

        attr_accessor attr(const char* name) const;
        attr_accessor attr(const handle& name) const;
        attr_accessor attr(object&& name) const;

        item_accessor operator[] (int64_t key) const;
        item_accessor operator[] (const char* key) const;
        item_accessor operator[] (const handle& key) const;
        item_accessor operator[] (object&& key) const;

        object operator- () const;
        object operator~() const;

        template <return_value_policy policy = return_value_policy::automatic, typename... Args>
        object operator() (Args&&... args) const;

    private:
        friend object operator+ (const handle& lhs, const handle& rhs);
        friend object operator- (const handle& lhs, const handle& rhs);
        friend object operator* (const handle& lhs, const handle& rhs);
        friend object operator% (const handle& lhs, const handle& rhs);
        friend object operator/ (const handle& lhs, const handle& rhs);
        friend object operator| (const handle& lhs, const handle& rhs);
        friend object operator& (const handle& lhs, const handle& rhs);
        friend object operator^ (const handle& lhs, const handle& rhs);
        friend object operator<< (const handle& lhs, const handle& rhs);
        friend object operator>> (const handle& lhs, const handle& rhs);

        friend object operator+= (const handle& lhs, const handle& rhs);
        friend object operator-= (const handle& lhs, const handle& rhs);
        friend object operator*= (const handle& lhs, const handle& rhs);
        friend object operator/= (const handle& lhs, const handle& rhs);
        friend object operator%= (const handle& lhs, const handle& rhs);
        friend object operator|= (const handle& lhs, const handle& rhs);
        friend object operator&= (const handle& lhs, const handle& rhs);
        friend object operator^= (const handle& lhs, const handle& rhs);
        friend object operator<<= (const handle& lhs, const handle& rhs);
        friend object operator>>= (const handle& lhs, const handle& rhs);

        friend object operator== (const handle& lhs, const handle& rhs);
        friend object operator!= (const handle& lhs, const handle& rhs);
        friend object operator< (const handle& lhs, const handle& rhs);
        friend object operator> (const handle& lhs, const handle& rhs);
        friend object operator<= (const handle& lhs, const handle& rhs);
        friend object operator>= (const handle& lhs, const handle& rhs);

        template <typename T>
        friend T& _builtin_cast(const handle& obj) {
            // FIXME: 2.0 does not use Py_<T> anymore
            static_assert(!std::is_reference_v<T>, "T must not be a reference type.");
            return obj.ptr().obj_get<T>();
        }
    };

    static_assert(std::is_trivially_copyable_v<handle>);

    class object : public handle {
    public:
        object(const object& other) : handle(other) { inc_ref(); }

        object(object&& other) noexcept : handle(other) {
            other.m_ptr = nullptr;
            other.ref_count = nullptr;
        }

        object& operator= (const object& other) {
            if(this != &other) {
                dec_ref();
                m_ptr = other.m_ptr;
                ref_count = other.ref_count;
                inc_ref();
            }
            return *this;
        }

        object& operator= (object&& other) noexcept {
            if(this != &other) {
                dec_ref();
                m_ptr = other.m_ptr;
                ref_count = other.ref_count;
                other.m_ptr = nullptr;
                other.ref_count = nullptr;
            }
            return *this;
        }

        ~object() {
            if(m_ptr != nullptr) {
                dec_ref();
            }
        }

    protected:
        object(const handle& h, bool borrow) : handle(h) {
            if(borrow) {
                inc_ref();
            }
        }

        template <typename T>
        friend T reinterpret_borrow(const handle& h) {
            return {h, true};
        }

        template <typename T>
        friend T reinterpret_steal(const handle& h) {
            return {h, false};
        }
    };

    inline void setattr(const handle& obj, const handle& name, const handle& value);
    inline void setitem(const handle& obj, const handle& key, const handle& value);

#define PYBIND11_BINARY_OPERATOR(OP, NAME)                                                         \
    inline object operator OP (const handle& lhs, const handle& rhs) {                             \
        return reinterpret_borrow<object>(vm->call(vm->py_op(NAME), lhs.m_ptr, rhs.m_ptr));        \
    }

    PYBIND11_BINARY_OPERATOR(+, "add");
    PYBIND11_BINARY_OPERATOR(-, "sub");
    PYBIND11_BINARY_OPERATOR(*, "mul");
    PYBIND11_BINARY_OPERATOR(/, "truediv");
    PYBIND11_BINARY_OPERATOR(%, "mod");
    PYBIND11_BINARY_OPERATOR(|, "or_");
    PYBIND11_BINARY_OPERATOR(&, "and_");
    PYBIND11_BINARY_OPERATOR(^, "xor");
    PYBIND11_BINARY_OPERATOR(<<, "lshift");
    PYBIND11_BINARY_OPERATOR(>>, "rshift");

    PYBIND11_BINARY_OPERATOR(+=, "iadd");
    PYBIND11_BINARY_OPERATOR(-=, "isub");
    PYBIND11_BINARY_OPERATOR(*=, "imul");
    PYBIND11_BINARY_OPERATOR(/=, "itruediv");
    PYBIND11_BINARY_OPERATOR(%=, "imod");
    PYBIND11_BINARY_OPERATOR(|=, "ior");
    PYBIND11_BINARY_OPERATOR(&=, "iand");
    PYBIND11_BINARY_OPERATOR(^=, "ixor");
    PYBIND11_BINARY_OPERATOR(<<=, "ilshift");
    PYBIND11_BINARY_OPERATOR(>>=, "irshift");

    PYBIND11_BINARY_OPERATOR(==, "eq");
    PYBIND11_BINARY_OPERATOR(!=, "ne");
    PYBIND11_BINARY_OPERATOR(<, "lt");
    PYBIND11_BINARY_OPERATOR(>, "gt");
    PYBIND11_BINARY_OPERATOR(<=, "le");
    PYBIND11_BINARY_OPERATOR(>=, "ge");

#undef PYBIND11_BINARY_OPERATOR

}  // namespace pybind11
