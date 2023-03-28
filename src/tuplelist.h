#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy {

using List = std::vector<PyObject*>;

class Args {
    inline static THREAD_LOCAL FreeListA<PyObject*, 10> _pool;

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
        int i = 0;
        for(PyObject* p : list) _args[i++] = p;
    }

    Args(List&& other) noexcept : Args(other.size()){
        for(int i=0; i<_size; i++) _args[i] = other[i];
        other.clear();
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

    List to_list() noexcept {
        List ret(_size);
        for(int i=0; i<_size; i++) ret[i] = _args[i];
        return ret;
    }

    void extend_self(PyObject* self){
        PyObject** old_args = _args;
        int old_size = _size;
        _alloc(old_size+1);
        _args[0] = self;
        for(int i=0; i<old_size; i++) _args[i+1] = old_args[i];
        _pool.dealloc(old_args, old_size);
    }

    ~Args(){ _pool.dealloc(_args, _size); }
};

inline const Args& no_arg() {
    static const Args _zero(0);
    return _zero;
}

typedef Args Tuple;

}   // namespace pkpy