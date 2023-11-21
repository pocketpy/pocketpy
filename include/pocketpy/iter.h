#pragma once

#include "cffi.h"
#include "common.h"
#include "frame.h"

namespace pkpy{

struct RangeIter{
    PY_CLASS(RangeIter, builtins, _range_iterator)
    Range r;
    i64 current;
    RangeIter(Range r) : r(r), current(r.start) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct ArrayIter{
    PY_CLASS(ArrayIter, builtins, _array_iterator)
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
    PY_CLASS(StringIter, builtins, _string_iterator)
    PyObject* ref;
    Str* str;
    int index;      // byte index

    StringIter(PyObject* ref) : ref(ref), str(&PK_OBJ_GET(Str, ref)), index(0) {}

    void _gc_mark() const{ PK_OBJ_MARK(ref); }

    static void _register(VM* vm, PyObject* mod, PyObject* type);
};

struct Generator{
    PY_CLASS(Generator, builtins, generator)
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

} // namespace pkpy