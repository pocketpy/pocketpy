#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "iter.h"
#include "cffi.h"
#include <deque>

namespace pkpy
{
    // STARTING HERE
    struct PyDeque
    {
        PY_CLASS(PyDeque, mycollections, deque);

        std::deque<PyObject*>dequeItems;

        void appendLeft(PyObject *item);
        void append(PyObject *item);
        PyObject *popLeft();
        PyObject *pop();

        bool insert(int index, PyObject *item);
        bool remove(VM *vm, PyObject *item);
        void rotate(int n);

        std::stringstream getRepr(VM *vm);
        void reverse();
        int findIndex(VM *vm, PyObject *obj, int startPos, int endPos); // vm is needed for the py_equals
        int count(VM *vm, PyObject *obj); // vm is needed for the py_equals
        void clear();
        
        PyDeque(){}
        static void _register(VM *vm, PyObject *mod, PyObject *type);

        void _gc_mark() const; // needed for container types
    };

    void add_module_mycollections(VM *vm);
} // namespace pkpy