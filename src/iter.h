#pragma once

#include "ceval.h"

namespace pkpy{

class RangeIter : public BaseIter {
    i64 current;
    Range r;    // copy by value, so we don't need to keep ref
public:
    RangeIter(VM* vm, PyObject* ref) : BaseIter(vm) {
        this->r = OBJ_GET(Range, ref);
        this->current = r.start;
    }

    bool _has_next(){
        return r.step > 0 ? current < r.stop : current > r.stop;
    }

    PyObject* next(){
        if(!_has_next()) return nullptr;
        current += r.step;
        return VAR(current-r.step);
    }
};

template <typename T>
class ArrayIter : public BaseIter {
    int index;
    PyObject* ref;
public:
    ArrayIter(VM* vm, PyObject* ref) : BaseIter(vm), ref(ref), index(0) {}

    PyObject* next() override{
        const T* p = &OBJ_GET(T, ref);
        if(index == p->size()) return nullptr;
        return p->operator[](index++); 
    }

    void _mark() override {
        OBJ_MARK(ref);
    }
};

class StringIter : public BaseIter {
    int index = 0;
    PyObject* ref;
public:
    StringIter(VM* vm, PyObject* ref) : BaseIter(vm), ref(ref) {}

    PyObject* next() override{
        Str* str = &OBJ_GET(Str, ref);
        if(index == str->u8_length()) return nullptr;
        return VAR(str->u8_getitem(index++));
    }

    void _mark() override {
        OBJ_MARK(ref);
    }
};

inline PyObject* Generator::next(){
    if(state == 2) return nullptr;
    vm->callstack.push(std::move(frame));
    PyObject* ret = vm->_exec();
    if(ret == vm->_py_op_yield){
        frame = std::move(vm->callstack.top());
        vm->callstack.pop();
        state = 1;
        return frame->popx();
    }else{
        state = 2;
        return nullptr;
    }
}

inline void Generator::_mark(){
    if(frame!=nullptr) frame->_mark();
}

template<typename T>
void _mark(T& t){
    if constexpr(std::is_base_of_v<BaseIter, T>){
        t._mark();
    }
}

} // namespace pkpy