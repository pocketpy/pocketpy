#pragma once

#include "obj.h"

typedef std::function<PyVar (_Int)> _PyIntFn;

class RangeIterator : public _Iterator {
private:
    _Int current;
    _Range r;
    _PyIntFn fn;
public:
    RangeIterator(PyVar _ref, _PyIntFn fn) : _Iterator(_ref), fn(fn) {
        this->r = std::get<_Range>(_ref->_native);
        this->current = r.start;
    }

    PyVar next() override {
        PyVar val = fn(current);
        current += r.step;
        return val;
    }

    bool hasNext() override {
        if(r.step > 0){
            return current < r.stop;
        }else{
            return current > r.stop;
        }
    }
};

class VectorIterator : public _Iterator {
private:
    size_t index = 0;
    const PyVarList* vec;
public:
    VectorIterator(PyVar _ref) : _Iterator(_ref) {
        vec = &std::get<PyVarList>(_ref->_native);
    }

    bool hasNext(){
        return index < vec->size();
    }

    PyVar next(){
        return vec->operator[](index++);
    }
};