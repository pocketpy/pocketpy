#pragma once

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "pocketpy/common/utils.h"
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

// 16 bytes to make py_arg() macro work
static_assert(sizeof(py_CFunction) <= 8, "sizeof(py_CFunction) > 8");
static_assert(sizeof(py_TValue) == 16, "sizeof(py_TValue) != 16");
static_assert(offsetof(py_TValue, extra) == 4, "offsetof(py_TValue, extra) != 4");