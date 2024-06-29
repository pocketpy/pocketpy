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

typedef int16_t py_Type;
typedef struct PyObject PyObject;

typedef struct py_TValue{
    py_Type type;
    bool is_ptr;
    int extra;
    union {
        int64_t _i64;
        double _f64;
        PyObject* _obj;
        void* _ptr;
        // Vec2
    };
} py_TValue;

// 16 bytes to make py_arg() macro work
static_assert(sizeof(py_TValue) == 16, "sizeof(py_TValue) != 16");

/* predefined vars */
static const py_Type tp_object = {1}, tp_type = {2};
static const py_Type tp_int = {3}, tp_float = {4}, tp_bool = {5}, tp_str = {6};
static const py_Type tp_list = {7}, tp_tuple = {8};
static const py_Type tp_slice = {9}, tp_range = {10}, tp_module = {11};
static const py_Type tp_function = {12}, tp_nativefunc = {13}, tp_bound_method = {14};
static const py_Type tp_super = {15}, tp_exception = {16}, tp_bytes = {17}, tp_mappingproxy = {18};
static const py_Type tp_dict = {19}, tp_property = {20}, tp_star_wrapper = {21};
static const py_Type tp_staticmethod = {22}, tp_classmethod = {23};
static const py_Type tp_none_type = {24}, tp_not_implemented_type = {25};
static const py_Type tp_ellipsis = {26};
static const py_Type tp_op_call = {27}, tp_op_yield = {28};
static const py_Type tp_syntax_error = {29}, tp_stop_iteration = {30};

extern py_TValue PY_NULL, PY_OP_CALL, PY_OP_YIELD;

#ifdef __cplusplus
}
#endif

/*
SSO types:
1. int64_t
2. double
3. bool (dummy)
4. tuple (extra + void*)
5. string (extra + void* or buf)
*/