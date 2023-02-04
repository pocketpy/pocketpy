#pragma once

#include "obj.h"

class RangeIterator : public BaseIterator {
private:
    i64 current;
    _Range r;
public:
    RangeIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        this->r = OBJ_GET(_Range, _ref);
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

class VectorIterator : public BaseIterator {
private:
    size_t index = 0;
    const PyVarList* vec;
public:
    VectorIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        vec = &OBJ_GET(PyVarList, _ref);
    }

    bool hasNext(){
        return index < vec->size();
    }

    PyVar next(){
        return vec->operator[](index++);
    }
};

class StringIterator : public BaseIterator {
private:
    int index = 0;
    _Str str;
public:
    StringIterator(VM* vm, PyVar _ref) : BaseIterator(vm, _ref) {
        str = OBJ_GET(_Str, _ref);
    }

    bool hasNext(){
        return index < str.u8_length();
    }

    PyVar next();
};
