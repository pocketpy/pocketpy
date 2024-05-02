#pragma once

#include "cffi.h"
#include "common.h"
#include "frame.h"

namespace pkpy{

struct RangeIter{
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct ArrayIter{
    PyObject* ref;
    PyObject** begin;
    PyObject** end;
    PyObject** current;

    ArrayIter(PyObject* ref, PyObject** begin, PyObject** end)
        : ref(ref), begin(begin), end(end), current(begin) {}

    void _gc_mark() const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct StringIter{
    PyObject* ref;
    int i;      // byte index
    StringIter(PyObject* ref) : ref(ref), i(0) {}
    void _gc_mark() const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct Generator{
    Frame frame;
    int state;      // 0,1,2
    List s_backup;

    Generator(Frame&& frame, ArgsView buffer): frame(std::move(frame)), state(0) {
        for(PyObject* obj: buffer) s_backup.push_back(obj);
    }

    void _gc_mark() const{
        frame._gc_mark();
        for(PyObject* obj: s_backup) PK_OBJ_MARK(obj);
    }

    PyObject* next(VM* vm);
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct DictItemsIter{
    PyObject* ref;
    int i;
    DictItemsIter(PyObject* ref) : ref(ref) {
        i = PK_OBJ_GET(Dict, ref)._head_idx;
    }
    void _gc_mark() const{ PK_OBJ_MARK(ref); }
    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

} // namespace pkpy