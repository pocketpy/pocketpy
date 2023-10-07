#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "cffi.h"

namespace pkpy
{
    struct PyDeque
    {
        PY_CLASS(PyDeque, collections, deque);

        // some fields can be defined here
        int len;

        PyDeque() = default;
        void printHelloWorld();
        static void _register(VM *vm, PyObject *mod, PyObject *type);

        void _gc_mark() const;  // needed for container types
    };

    void add_module_mycollections(VM *vm);
} // namespace pkpy