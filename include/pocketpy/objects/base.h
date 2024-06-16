#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "assert.h"
#include "string.h"

#include "pocketpy/common/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t pkpy_Type;

typedef struct PyObject PyObject;
typedef struct pkpy_VM pkpy_VM;

struct pkpy_G {
    pkpy_VM* vm;
} extern pkpy_g;

typedef struct PyVar{
    pkpy_Type type;
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
static const pkpy_Type tp_object = 1, tp_type = 2;
static const pkpy_Type tp_int = 3, tp_float = 4, tp_bool = 5, tp_str = 6;
static const pkpy_Type tp_list = 7, tp_tuple = 8;
static const pkpy_Type tp_slice = 9, tp_range = 10, tp_module = 11;
static const pkpy_Type tp_function = 12, tp_native_func = 13, tp_bound_method = 14;
static const pkpy_Type tp_super = 15, tp_exception = 16, tp_bytes = 17, tp_mappingproxy = 18;
static const pkpy_Type tp_dict = 19, tp_property = 20, tp_star_wrapper = 21;
static const pkpy_Type tp_staticmethod = 22, tp_classmethod = 23;
static const pkpy_Type tp_none_type = 24, tp_not_implemented_type = 25;
static const pkpy_Type tp_ellipsis = 26;
static const pkpy_Type tp_op_call = 27, tp_op_yield = 28;

PK_INLINE bool PyVar__is_null(const PyVar* self) { return self->type == 0; }
PK_INLINE int64_t PyVar__hash(const PyVar* self) { return self->extra + self->_i64; }

PK_INLINE void PyVar__ctor(PyVar* self, pkpy_Type type, PyObject* obj){
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

extern PyVar pkpy_NULL, pkpy_OP_CALL, pkpy_OP_YIELD;

bool py_eq(const PyVar*, const PyVar*);
bool py_le(const PyVar*, const PyVar*);
int64_t py_hash(const PyVar*);

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