#pragma once

#include "cffi.h"
#include "common.h"
#include "frame.h"

namespace pkpy{

struct RangeIter{       // step > 0
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyVar mod, PyVar type);
};

struct RangeIterR{      // step < 0
    Range r;
    i64 current;
    RangeIterR(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyVar mod, PyVar type);
};

struct ArrayIter{
    PyObject* ref;
    PyVar* end;
    PyVar* current;

    ArrayIter(PyObject* ref, PyVar* begin, PyVar* end)
        : ref(ref), end(end), current(begin) {}

    void _gc_mark(VM* vm) const{ vm->__obj_gc_mark(ref); }
    static void _register(VM* vm, PyVar mod, PyVar type);
};

struct StringIter{
    PyVar ref;
    int i;      // byte index
    StringIter(PyVar ref) : ref(ref), i(0) {}
    void _gc_mark(VM* vm) const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyVar mod, PyVar type);
};

struct Generator{
    Frame frame;
    int state;      // 0,1,2
    List s_backup;

    Generator(Frame&& frame, ArgsView buffer): frame(std::move(frame)), state(0) {
        for(PyVar obj: buffer) s_backup.push_back(obj);
    }

    void _gc_mark(VM* vm) const{
        frame._gc_mark(vm);
        for(PyVar obj: s_backup) PK_OBJ_MARK(obj);
    }

    PyVar next(VM* vm);
    static void _register(VM* vm, PyVar mod, PyVar type);
};

struct DictItemsIter{
    PyVar ref;
    int i;
    DictItemsIter(PyVar ref) : ref(ref) {
        i = PK_OBJ_GET(Dict, ref)._head_idx;
    }
    void _gc_mark(VM* vm) const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyVar mod, PyVar type);
};

} // namespace pkpy