#pragma once

#include "pocketpy/pocketpy.h"

typedef struct PyObject PyObject;
typedef struct VM VM;
extern VM* pk_current_vm;

typedef struct py_TValue {
    py_Type type;
    bool is_ptr;
    int extra;

    union {
        int64_t _i64;
        double _f64;
        bool _bool;
        py_CFunction _cfunc;
        PyObject* _obj;
        c11_vec2 _vec2;
        c11_vec2i _vec2i;
    };
} py_TValue;
