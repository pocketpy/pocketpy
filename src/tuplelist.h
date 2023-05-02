#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"
#include "vector.h"

namespace pkpy {

using List = pod_vector<PyObject*>;

class Tuple {
    PyObject** _args;
    int _size;

    void _alloc(int n){
        this->_args = (n==0) ? nullptr : (PyObject**)pool64.alloc(n * sizeof(void*));
        this->_size = n;
    }

public:
    Tuple(int n){ _alloc(n); }

    Tuple(const Tuple& other){
        _alloc(other._size);
        for(int i=0; i<_size; i++) _args[i] = other._args[i];
    }

    Tuple(Tuple&& other) noexcept {
        this->_args = other._args;
        this->_size = other._size;
        other._args = nullptr;
        other._size = 0;
    }

    Tuple(std::initializer_list<PyObject*> list) : Tuple(list.size()){
        int i = 0;
        for(PyObject* p : list) _args[i++] = p;
    }

    // TODO: poor performance
    // List is allocated by pool128 while tuple is by pool64
    // ...
    Tuple(List&& other) noexcept : Tuple(other.size()){
        for(int i=0; i<_size; i++) _args[i] = other[i];
        other.clear();
    }

    PyObject*& operator[](int i){ return _args[i]; }
    PyObject* operator[](int i) const { return _args[i]; }

    Tuple& operator=(Tuple&& other) noexcept {
        if(_args!=nullptr) pool64.dealloc(_args);
        this->_args = other._args;
        this->_size = other._size;
        other._args = nullptr;
        other._size = 0;
        return *this;
    }

    int size() const { return _size; }

    PyObject** begin() const { return _args; }
    PyObject** end() const { return _args + _size; }

    ~Tuple(){ if(_args!=nullptr) pool64.dealloc(_args); }
};

// a lightweight view for function args, it does not own the memory
struct ArgsView{
    PyObject** _begin;
    PyObject** _end;

    ArgsView(PyObject** begin, PyObject** end) : _begin(begin), _end(end) {}
    ArgsView(const Tuple& t) : _begin(t.begin()), _end(t.end()) {}
    ArgsView(): _begin(nullptr), _end(nullptr) {}

    PyObject** begin() const { return _begin; }
    PyObject** end() const { return _end; }
    int size() const { return _end - _begin; }
    bool empty() const { return _begin == _end; }
    PyObject* operator[](int i) const { return _begin[i]; }

    List to_list() const{
        List ret(size());
        for(int i=0; i<size(); i++) ret[i] = _begin[i];
        return ret;
    }

    Tuple to_tuple() const{
        Tuple ret(size());
        for(int i=0; i<size(); i++) ret[i] = _begin[i];
        return ret;
    }
};
}   // namespace pkpy
