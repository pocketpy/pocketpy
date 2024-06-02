#pragma once

#include "pocketpy/interpreter/bindings.hpp"

namespace pkpy{

struct RangeIter{       // step > 0
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct RangeIterR{      // step < 0
    Range r;
    i64 current;
    RangeIterR(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct ArrayIter{
    PyObject* ref;
    PyVar* end;
    PyVar* current;

    ArrayIter(PyObject* ref, PyVar* begin, PyVar* end)
        : ref(ref), end(end), current(begin) {}

    void _gc_mark(VM* vm) const{ vm->__obj_gc_mark(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct StringIter{
    PyVar ref;
    int i;      // byte index
    StringIter(PyVar ref) : ref(ref), i(0) {}
    void _gc_mark(VM* vm) const{ vm->obj_gc_mark(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct Generator{
    LinkedFrame* lf;
    int state;      // 0,1,2
    List s_backup;

    Generator(LinkedFrame* lf, ArgsView buffer): lf(lf), state(0) {
        for(PyVar obj: buffer) s_backup.push_back(obj);
    }

    void _gc_mark(VM* vm) {
        if(lf == nullptr) return;
        lf->frame._gc_mark(vm);
        vm->__stack_gc_mark(s_backup.begin(), s_backup.end());
    }

    PyVar next(VM* vm);
    static void _register(VM* vm, PyObject* mod, PyObject* type);

    ~Generator(){
        if(lf){
            lf->~LinkedFrame();
            pool128_dealloc(lf);
        }
    }
};

struct DictItemsIter{
    PyVar ref;
    int i;
    DictItemsIter(PyVar ref) : ref(ref) {
        i = PK_OBJ_GET(Dict, ref)._head_idx;
    }
    void _gc_mark(VM* vm) const{ vm->obj_gc_mark(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

} // namespace pkpy