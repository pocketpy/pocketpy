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
    uint8_t flags;
    int flags_ex;
    union {
        int64_t _i64;
        double _f64;
        bool _bool;
        PyObject* _obj;
        void* _ptr;
        // Vec2
    };
} PyVar;

static_assert(sizeof(PyVar) == 16, "sizeof(PyVar) != 16");

PK_INLINE bool PyVar__is_null(const PyVar* self) { return self->type == 0; }
PK_INLINE int64_t PyVar__hash(const PyVar* self) { return self->flags_ex + self->_i64; }

PK_INLINE bool PyVar__less(const PyVar* self, const PyVar* other){
    return memcmp(self, other, sizeof(PyVar)) < 0;
}
PK_INLINE bool PyVar__equal(const PyVar* self, const PyVar* other){
    return memcmp(self, other, sizeof(PyVar)) == 0;
}

PK_INLINE void PyVar__ctor(PyVar* self, pkpy_Type type, PyObject* obj){
    self->type = type;
    self->is_ptr = true;
    self->flags = 0;
    self->flags_ex = 0;
    self->_obj = obj;
}

void PyVar__ctor2(PyVar* self, PyObject* existing);

#define pkpy_Var__is_null(self) ((self)->type == 0)
#define pkpy_Var__set_null(self) do { (self)->type = 0; } while(0)
bool pkpy_Var__eq__(void *vm, PyVar a, PyVar b);
int64_t pkpy_Var__hash__(void *vm, PyVar a);

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

extern const PyVar pkpy_True, pkpy_False, pkpy_None;
extern const PyVar pkpy_NotImplemented, pkpy_Ellipsis;
extern const PyVar pkpy_NULL;

#ifdef __cplusplus
}
#endif