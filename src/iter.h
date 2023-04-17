#pragma once

#include "ceval.h"

namespace pkpy{

class RangeIter final: public BaseIter {
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
class ArrayIter final: public BaseIter {
    PyObject* ref;
    int index;
public:
    ArrayIter(VM* vm, PyObject* ref) : BaseIter(vm), ref(ref), index(0) {}

    PyObject* next() override{
        const T* p = &OBJ_GET(T, ref);
        if(index == p->size()) return nullptr;
        return p->operator[](index++); 
    }

    void _gc_mark() const override {
        OBJ_MARK(ref);
    }
};

class StringIter final: public BaseIter {
    PyObject* ref;
    int index;
public:
    StringIter(VM* vm, PyObject* ref) : BaseIter(vm), ref(ref), index(0) {}

    PyObject* next() override{
        // TODO: optimize this to use iterator
        // operator[] is O(n) complexity
        Str* str = &OBJ_GET(Str, ref);
        if(index == str->u8_length()) return nullptr;
        return VAR(str->u8_getitem(index++));
    }

    void _gc_mark() const override {
        OBJ_MARK(ref);
    }
};

inline PyObject* Generator::next(){
    if(state == 2) return nullptr;
    // restore the context
    for(PyObject* obj: s_data) frame._s->push(obj);
    s_data.clear();
    vm->callstack.push(std::move(frame));
    PyObject* ret = vm->_run_top_frame();
    if(ret == vm->_py_op_yield){
        // backup the context
        frame = std::move(vm->callstack.top());
        for(PyObject* obj: frame.stack_view()) s_data.push_back(obj);
        vm->_pop_frame();
        state = 1;
        return frame._s->popx();
    }else{
        state = 2;
        return nullptr;
    }
}

inline void Generator::_gc_mark() const{
    frame._gc_mark();
}

template<typename T>
void gc_mark(T& t) {
    if constexpr(std::is_base_of_v<BaseIter, T>){
        t._gc_mark();
    }
}

} // namespace pkpy