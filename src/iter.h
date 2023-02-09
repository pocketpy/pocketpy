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

    bool has_next(){
        return r.step > 0 ? current < r.stop : current > r.stop;
    }

    PyVar next(){
        current += r.step;
        return vm->PyInt(current-r.step);
    }
};

template <typename T>
class ArrayIter : public BaseIter {
    size_t index = 0;
    const T* p;
public:
    ArrayIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) { p = &OBJ_GET(T, _ref);}
    bool has_next(){ return index < p->size(); }
    PyVar next(){ return p->operator[](index++); }
};

class StringIter : public BaseIter {
    int index = 0;
    _Str str;
public:
    StringIter(VM* vm, PyVar _ref) : BaseIter(vm, _ref) {
        str = OBJ_GET(_Str, _ref);
    }

    bool has_next(){ return index < str.u8_length(); }
    PyVar next() { return vm->PyStr(str.u8_getitem(index++)); }
};
