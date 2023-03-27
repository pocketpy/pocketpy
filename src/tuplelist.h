#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy {
    using List = std::vector<PyObject*>;

    class Args {
        static THREAD_LOCAL SmallArrayPool<PyObject*, 10> _pool;

        PyObject** _args;
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

        Args(Args&& other) noexcept {
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
        }

        static pkpy::Args from_list(List&& other) noexcept {
            Args ret(other.size());
            memcpy((void*)ret._args, (void*)other.data(), sizeof(PyObject*)*ret.size());
            memset((void*)other.data(), 0, sizeof(PyObject*)*ret.size());
            other.clear();
            return ret;
        }

        PyObject*& operator[](int i){ return _args[i]; }
        PyObject* operator[](int i) const { return _args[i]; }

        Args& operator=(Args&& other) noexcept {
            _pool.dealloc(_args, _size);
            this->_args = other._args;
            this->_size = other._size;
            other._args = nullptr;
            other._size = 0;
            return *this;
        }

        inline int size() const { return _size; }

        List move_to_list() noexcept {
            List ret(_size);
            memcpy((void*)ret.data(), (void*)_args, sizeof(PyObject*)*_size);
            memset((void*)_args, 0, sizeof(PyObject*)*_size);
            return ret;
        }

        void extend_self(PyObject* self){
            PyObject** old_args = _args;
            int old_size = _size;
            _alloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(PyObject*)*old_size);
            memset((void*)old_args, 0, sizeof(PyObject*)*old_size);
            _pool.dealloc(old_args, old_size);
        }

        ~Args(){ _pool.dealloc(_args, _size); }
    };

    inline const Args& no_arg() {
        static const Args _zero(0);
        return _zero;
    }

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

    template<typename T1, typename T2, typename T3>
    Args three_args(T1&& a, T2&& b, T3&& c) {
        Args ret(3);
        ret[0] = std::forward<T1>(a);
        ret[1] = std::forward<T2>(b);
        ret[2] = std::forward<T3>(c);
        return ret;
    }

    typedef Args Tuple;
    THREAD_LOCAL SmallArrayPool<PyObject*, 10> Args::_pool;
}   // namespace pkpy