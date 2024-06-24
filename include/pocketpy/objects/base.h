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

typedef int16_t Type;

typedef struct PyVar{
    Type type;
    bool is_ptr;
    int extra;
    union {
        int64_t _i64;
        double _f64;
        PyObject* _obj;
        void* _ptr;
        // Vec2
    };
} PyVar;


#define PyVar__as(T, self)  _Generic((T), \
    int64_t: self->_i64, \
    double: self->_f64, \
    PyObject*: self->_obj, \
    void*: self->_ptr, \
)

static_assert(sizeof(PyVar) == 16, "sizeof(PyVar) != 16");

/* predefined vars */
static const Type tp_object = {1}, tp_type = {2};
static const Type tp_int = {3}, tp_float = {4}, tp_bool = {5}, tp_str = {6};
static const Type tp_list = {7}, tp_tuple = {8};
static const Type tp_slice = {9}, tp_range = {10}, tp_module = {11};
static const Type tp_function = {12}, tp_nativefunc = {13}, tp_bound_method = {14};
static const Type tp_super = {15}, tp_exception = {16}, tp_bytes = {17}, tp_mappingproxy = {18};
static const Type tp_dict = {19}, tp_property = {20}, tp_star_wrapper = {21};
static const Type tp_staticmethod = {22}, tp_classmethod = {23};
static const Type tp_none_type = {24}, tp_not_implemented_type = {25};
static const Type tp_ellipsis = {26};
static const Type tp_op_call = {27}, tp_op_yield = {28};
static const Type tp_syntax_error = {29}, tp_stop_iteration = {30};

PK_INLINE bool PyVar__is_null(const PyVar* self) { return self->type == 0; }
PK_INLINE int64_t PyVar__hash(const PyVar* self) { return self->extra + self->_i64; }

PK_INLINE void PyVar__ctor(PyVar* self, Type type, PyObject* obj){
    self->type = type;
    self->is_ptr = true;
    self->_obj = obj;
}

void PyVar__ctor3(PyVar* self, PyObject* existing);

PK_INLINE bool PyVar__IS_OP(const PyVar* a, const PyVar* b){
    return a->is_ptr && b->is_ptr && a->_obj == b->_obj;
}

#define pkpy_Var__is_null(self) ((self)->type == 0)
#define pkpy_Var__set_null(self) do { (self)->type = 0; } while(0)

extern PyVar PY_NULL, PY_OP_CALL, PY_OP_YIELD;

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