#pragma once

#include "ceval.h"
#include "frame.h"

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
    T* array;
    int index;
public:
    ArrayIter(VM* vm, PyObject* ref) : BaseIter(vm), ref(ref) {
        array = &OBJ_GET(T, ref);
        index = 0;
    }

    PyObject* next() override{
        if(index >= array->size()) return nullptr;
        return array->operator[](index++);
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
    // reset frame._sp_base
    frame._sp_base = frame._s->_sp;
    frame._locals.a = frame._s->_sp;
    // restore the context
    for(PyObject* obj: s_backup) frame._s->push(obj);
    s_backup.clear();
    vm->callstack.push(std::move(frame));
    PyObject* ret = vm->_run_top_frame();
    if(ret == vm->_py_op_yield){
        // backup the context
        frame = std::move(vm->callstack.top());
        PyObject* ret = frame._s->popx();
        for(PyObject* obj: frame.stack_view()) s_backup.push_back(obj);
        vm->_pop_frame();
        state = 1;
        return ret;
    }else{
        state = 2;
        return nullptr;
    }
}

inline void Generator::_gc_mark() const{
    frame._gc_mark();
    for(PyObject* obj: s_backup) OBJ_MARK(obj);
}

template<typename T>
void gc_mark(T& t) {
    if constexpr(std::is_base_of_v<BaseIter, T>){
        t._gc_mark();
    }
}

} // namespace pkpy