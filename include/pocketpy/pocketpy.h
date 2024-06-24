#pragma once

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject PyObject;
typedef struct PyVar PyVar;
typedef struct pk_VM pk_VM;
typedef struct py_Error py_Error;

typedef enum BindType {
    BindType_FUNCTION,
    BindType_STATICMETHOD,
    BindType_CLASSMETHOD,
} BindType;

typedef int (*py_CFunction)(const PyVar*, int);

typedef uint16_t StrName;
typedef int16_t Type;

extern pk_VM* pk_current_vm;

void py_initialize();
// void py_switch_vm(const char* name);
void py_finalize();

py_Error* py_exec_simple(const char*);
py_Error* py_eval_simple(const char*, PyVar*);

/* py_error */
void py_Error__print(const py_Error*);
void py_Error__delete(py_Error*);

int py_eq(const PyVar*, const PyVar*);
int py_le(const PyVar*, const PyVar*);
int py_hash(const PyVar*, int64_t* out);

/* py_var */
void py_new_int(PyVar*, int64_t);
void py_new_float(PyVar*, double);
void py_new_bool(PyVar*, bool);
void py_new_str(PyVar*, const char*);
void py_new_strn(PyVar*, const char*, int);
void py_new_fstr(PyVar*, const char*, ...);
void py_new_bytes(PyVar*, const uint8_t*, int);
void py_new_none(PyVar*);
void py_new_null(PyVar*);

// new style decl-based function
void py_new_function(PyVar*, py_CFunction, const char* sig, BindType bt);
void py_new_function2(PyVar*, py_CFunction, const char* sig, BindType bt, const char* docstring, const PyVar* userdata);
// old style argc-based function
void py_new_nativefunc(PyVar*, py_CFunction, int argc, BindType bt);
void py_new_nativefunc2(PyVar*, py_CFunction, int argc, BindType bt, const char* docstring, const PyVar* userdata);

int py_setattr(PyVar* self, StrName name, const PyVar* val);

PyVar py_new_module(const char* name);
PyVar pk_new_module(const char* name, const char* package);

// Type pk_new_type

#define py_isnull(self) ((self)->type == 0)

#ifdef __cplusplus
}
#endif
