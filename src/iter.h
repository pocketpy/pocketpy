#pragma once

#include "vm.h"

class RangeIter : public BaseIter {
    i64 current;
    _Range r;
public:
    RangeIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        this->r = OBJ_GET(_Range, _ref);
        this->current = r.start;
    }

    bool hasNext(){
        return r.step > 0 ? current < r.stop : current > r.stop;
    }

    PyVar next(){
        PyVar val = vm->PyInt(current);
        current += r.step;
        return val;
    }
};

class VectorIter : public BaseIter {
    size_t index = 0;
    const PyVarList* vec;
public:
    VectorIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        vec = &OBJ_GET(PyVarList, _ref);
    }

    bool hasNext(){ return index < vec->size(); }
    PyVar next(){ return vec->operator[](index++); }
};

class StringIter : public BaseIter {
    int index = 0;
    _Str str;
public:
    StringIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        str = OBJ_GET(_Str, _ref);
    }

    bool hasNext(){ return index < str.u8_length(); }
    PyVar next() { return vm->PyStr(str.u8_getitem(index++)); }
};
