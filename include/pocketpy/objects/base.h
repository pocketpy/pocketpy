#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "assert.h"
#include "string.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject PyObject;
typedef struct pk_VM pk_VM;
extern pk_VM* pk_current_vm;

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
        // Vec2
    };
} py_TValue;

// 16 bytes to make py_arg() macro work
static_assert(sizeof(py_CFunction) <= 8, "sizeof(py_CFunction) > 8");
static_assert(sizeof(py_TValue) == 16, "sizeof(py_TValue) != 16");

#ifdef __cplusplus
}
#endif
