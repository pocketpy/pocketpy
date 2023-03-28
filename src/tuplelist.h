#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"
#include <initializer_list>

namespace pkpy {
    using List = std::vector<PyObject*>;

    class Args {
        static THREAD_LOCAL SmallArrayPool<PyObject*, 10> _pool;

        PyObject** _args;
        int _size;

        void _alloc(int n){
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

        Args(std::initializer_list<PyObject*> list) : Args(list.size()){
            int i=0;
            for(auto& p : list) _args[i++] = p;
        }

        static pkpy::Args from_list(List&& other) noexcept {
            Args ret(other.size());
            memcpy((void*)ret._args, (void*)other.data(), sizeof(void*)*ret.size());
            memset((void*)other.data(), 0, sizeof(void*)*ret.size());
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

        int size() const { return _size; }

        List move_to_list() noexcept {
            List ret(_size);
            memcpy((void*)ret.data(), (void*)_args, sizeof(void*)*_size);
            memset((void*)_args, 0, sizeof(void*)*_size);
            return ret;
        }

        void extend_self(PyObject* self){
            PyObject** old_args = _args;
            int old_size = _size;
            _alloc(old_size+1);
            _args[0] = self;
            if(old_size == 0) return;

            memcpy((void*)(_args+1), (void*)old_args, sizeof(void*)*old_size);
            memset((void*)old_args, 0, sizeof(void*)*old_size);
            _pool.dealloc(old_args, old_size);
        }

        ~Args(){ _pool.dealloc(_args, _size); }
    };

    inline const Args& no_arg() {
        static const Args _zero(0);
        return _zero;
    }

    typedef Args Tuple;
    inline THREAD_LOCAL SmallArrayPool<PyObject*, 10> Args::_pool;
}   // namespace pkpy