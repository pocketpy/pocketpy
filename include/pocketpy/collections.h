#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"
#include "iter.h"
#include "cffi.h"

namespace pkpy
{
    // STARTING HERE
    struct PyDeque
    {
        PY_CLASS(PyDeque, mycollections, deque);
        
        PyDeque(VM *vm, PyObject *iterable, PyObject* maxlen);

        // PyDeque members
        std::deque<PyObject *> dequeItems;
        int maxlen=-1; // -1 means unbounded
        bool bounded=false; // if true, maxlen is not -1


        // PyDeque methods: add, remove, insert, etc.
        void appendLeft(PyObject *item); // add to the left
        void append(PyObject *item); // add to the right
        PyObject *popLeft(); // remove from the left
        PyObject *pop(); // remove from the right
        bool insert(int index, PyObject *item); // insert at index
        bool remove(VM *vm, PyObject *item); // remove first occurence of item
        
        
        void rotate(int n); // rotate n steps to the right
        void reverse();// reverse the deque
        void clear(); // clear the deque
        

        int count(VM *vm, PyObject *obj); // count the number of occurences of obj
        int findIndex(VM *vm, PyObject *obj, int startPos, int endPos); // find the index of obj in range starting from startPos and ending at endPos, default range is entire deque

        PyObject* getItem(int index); // get item at index
        bool setItem(int index, PyObject* item); // set item at index
        bool eraseItem(int index);// erase item at index

        std::stringstream getRepr(VM *vm); // get the string representation of the deque

        int fixIndex(int index); // for internal use only, returns -1 if index is out of range, handles negative indices
        // Special methods
        static void _register(VM *vm, PyObject *mod, PyObject *type); // register the type
        void _gc_mark() const; // needed for container types, mark all objects in the deque for gc
    };

    void add_module_mycollections(VM *vm);
} // namespace pkpy