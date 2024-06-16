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

typedef struct PyVar{
    pkpy_Type type;
    bool is_ptr;
    int extra;
    union {
        int64_t _i64;
        double _f64;
        bool _bool;
        PyObject* _obj;
        void* _ptr;
        // Vec2
    };
} PyVar;


#define PyVar__as(T, self)  _Generic((T), \
    int64_t: self->_i64, \
    double: self->_f64, \
    bool: self->_bool, \
    PyObject*: self->_obj, \
    void*: self->_ptr, \
)

static_assert(sizeof(PyVar) == 16, "sizeof(PyVar) != 16");

extern const pkpy_Type tp_object, tp_type;
extern const pkpy_Type tp_int, tp_float, tp_bool, tp_str;
extern const pkpy_Type tp_list, tp_tuple;
extern const pkpy_Type tp_slice, tp_range, tp_module;
extern const pkpy_Type tp_function, tp_native_func, tp_bound_method;
extern const pkpy_Type tp_super, tp_exception, tp_bytes, tp_mappingproxy;
extern const pkpy_Type tp_dict, tp_property, tp_star_wrapper;
extern const pkpy_Type tp_staticmethod, tp_classmethod;
extern const pkpy_Type tp_none_type, tp_not_implemented_type;
extern const pkpy_Type tp_ellipsis;
extern const pkpy_Type tp_op_call, tp_op_yield;

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
bool pkpy_Var__eq__(void *vm, PyVar a, PyVar b);
int64_t pkpy_Var__hash__(void *vm, PyVar a);

extern PyVar pkpy_True, pkpy_False, pkpy_None;
extern PyVar pkpy_NotImplemented, pkpy_Ellipsis;
extern PyVar pkpy_NULL, pkpy_OP_CALL, pkpy_OP_YIELD;

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