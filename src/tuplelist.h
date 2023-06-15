#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"
#include "vector.h"

namespace pkpy {

using List = pod_vector<PyObject*>;

class Tuple {
    PyObject** _args;
    PyObject* _inlined[3];
    int _size;

    bool is_inlined() const { return _args == _inlined; }

    void _alloc(int n){
        if(n <= 3){
            this->_args = _inlined;
        }else{
            this->_args = (PyObject**)pool64.alloc(n * sizeof(void*));
        }
        this->_size = n;
    }

public:
    Tuple(int n){ _alloc(n); }

    Tuple(const Tuple& other){
        _alloc(other._size);
        for(int i=0; i<_size; i++) _args[i] = other._args[i];
    }

    Tuple(Tuple&& other) noexcept {
        _size = other._size;
        if(other.is_inlined()){
            _args = _inlined;
            for(int i=0; i<_size; i++) _args[i] = other._args[i];
        }else{
            _args = other._args;
            other._args = other._inlined;
            other._size = 0;
        }
    }

    Tuple(List&& other) noexcept {
        _size = other.size();
        _args = other._data;
        other._data = nullptr;
    }

    Tuple(std::initializer_list<PyObject*> list) {
        _alloc(list.size());
        int i = 0;
        for(PyObject* obj: list) _args[i++] = obj;
    }

    PyObject*& operator[](int i){ return _args[i]; }
    PyObject* operator[](int i) const { return _args[i]; }

    int size() const { return _size; }

    PyObject** begin() const { return _args; }
    PyObject** end() const { return _args + _size; }

    ~Tuple(){ if(!is_inlined()) pool64.dealloc(_args); }
};

// a lightweight view for function args, it does not own the memory
struct ArgsView{
    PyObject** _begin;
    PyObject** _end;

    ArgsView(PyObject** begin, PyObject** end) : _begin(begin), _end(end) {}
    ArgsView(const Tuple& t) : _begin(t.begin()), _end(t.end()) {}

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