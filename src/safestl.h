#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

struct PyObject;
typedef pkpy::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef PyVar PyVarRef;

#include "hash_table5.hpp"
namespace pkpy {
	template<typename... Args>
	using HashMap = emhash5::HashMap<Args...>;
}

namespace pkpy{
class List: public std::vector<PyVar> {
    PyVar& at(size_t) = delete;

    inline void _check_index(size_t i) const {
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
    }
public:
    PyVar& operator[](size_t i) {
        _check_index(i);
        return std::vector<PyVar>::operator[](i);
    }

    const PyVar& operator[](size_t i) const {
        _check_index(i);
        return std::vector<PyVar>::operator[](i);
    }

    using std::vector<PyVar>::vector;
};


}

namespace pkpy {
    typedef HashMap<StrName, PyVar> NameDict;

    class Args {
        static THREAD_LOCAL SmallArrayPool<PyVar, 10> _pool;

        PyVar* _args;
        int _size;

        inline void _alloc(int n){
            this->_args = _pool.alloc(n);
            this->_size = n;
        }

    public:
        Args(int n){ _alloc(n); }

        Args(const Args& other){
            _alloc(other._size);
            for(int i=0; i<_size; i++) _args[i] = other._args[i];
        }

        Args(std::initializer_list<PyVar> a){
            _alloc(a.size());
            int i = 0;
            for(auto& v: a) _args[i++] = v;
        }

        Args(Args&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        Args(pkpy::List&& other) noexcept {
            _alloc(other.size());
            memcpy((void*)_args, (void*)other.data(), sizeof(PyVar)*_size);
            memset((void*)other.data(), 0, sizeof(PyVar)*_size);
            other.clear();
        }

        PyVar& operator[](int i){ return _args[i]; }
        const PyVar& operator[](int i) const { return _args[i]; }

        Args& operator=(Args&& other) noexcept {
            _pool.dealloc(_args, _size);
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
            return *this;
        }

        inline int size() const { return _size; }

        pkpy::List move_to_list() noexcept {
            pkpy::List ret(_size);
            memcpy((void*)ret.data(), (void*)_args, sizeof(PyVar)*_size);
            memset((void*)_args, 0, sizeof(PyVar)*_size);
            return ret;
        }

        void extend_self(const PyVar& self){
            static_assert(std::is_standard_layout_v<PyVar>);
            PyVar* old_args = _args;
            int old_size = _size;
            _alloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(PyVar)*old_size);
            memset((void*)old_args, 0, sizeof(PyVar)*old_size);
            _pool.dealloc(old_args, old_size);
        }

        ~Args(){ _pool.dealloc(_args, _size); }
    };

    static const Args _zero(0);
    inline const Args& no_arg() { return _zero; }

    template<typename T>
    Args one_arg(T&& a) {
        Args ret(1);
        ret[0] = std::forward<T>(a);
        return ret;
    }

    template<typename T1, typename T2>
    Args two_args(T1&& a, T2&& b) {
        Args ret(2);
        ret[0] = std::forward<T1>(a);
        ret[1] = std::forward<T2>(b);
        return ret;
    }

    typedef Args Tuple;

    // declare static members
    THREAD_LOCAL SmallArrayPool<PyVar, 10> Args::_pool;
    // THREAD_LOCAL SmallArrayPool<NameDictNode, 1> NameDict::_pool;
}   // namespace pkpy