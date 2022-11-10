#pragma once

#include "obj.h"

class RangeIterator : public _Iterator {
private:
    _Int current;
    _Range r;
public:
    RangeIterator(VM* vm, PyVar _ref) : _Iterator(vm, _ref) {
        this->r = std::get<_Range>(_ref->_native);
        this->current = r.start;
    }

    bool hasNext() override {
        if(r.step > 0){
            return current < r.stop;
        }else{
            return current > r.stop;
        }
    }

    PyVar next() override;
};

class VectorIterator : public _Iterator {
private:
    size_t index = 0;
    const PyVarList* vec;
public:
    VectorIterator(VM* vm, PyVar _ref) : _Iterator(vm, _ref) {
        vec = &std::get<PyVarList>(_ref->_native);
    }

    bool hasNext(){
        return index < vec->size();
    }

    PyVar next(){
        return vec->operator[](index++);
    }
};

class StringIterator : public _Iterator {
private:
    size_t index = 0;
    const _Str* str;
public:
    StringIterator(VM* vm, PyVar _ref) : _Iterator(vm, _ref) {
        str = &std::get<_Str>(_ref->_native);
    }

    bool hasNext(){
        return index < str->u8_length();
    }

    PyVar next();
};
