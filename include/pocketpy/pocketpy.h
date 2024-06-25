#pragma once

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PyObject PyObject;
typedef struct PyVar PyVar;
typedef struct pk_VM pk_VM;
typedef uint16_t py_Name;
typedef int16_t Type;
typedef PyVar* py_Ref;
typedef int (*py_CFunction)(const py_Ref, int);

typedef struct py_Error{
    Type type;
} py_Error;

typedef enum BindType {
    BindType_FUNCTION,
    BindType_STATICMETHOD,
    BindType_CLASSMETHOD,
} BindType;



extern pk_VM* pk_current_vm;


void py_initialize();
// void py_switch_vm(const char* name);
void py_finalize();

int py_exec_simple(const char*);
int py_eval_simple(const char*, py_Ref);

/* py_error */
py_Error* py_getlasterror();
void py_Error__print(py_Error*);

int py_eq(const py_Ref, const py_Ref);
int py_le(const py_Ref, const py_Ref);
int py_hash(const py_Ref, int64_t* out);

/* py_var */
void py_new_int(py_Ref, int64_t);
void py_new_float(py_Ref, double);
void py_new_bool(py_Ref, bool);
void py_new_str(py_Ref, const char*);
void py_new_strn(py_Ref, const char*, int);
void py_new_fstr(py_Ref, const char*, ...);
void py_new_bytes(py_Ref, const uint8_t*, int);
void py_new_none(py_Ref);
void py_new_null(py_Ref);

void py_new_tuple(PyVar, int);

// new style decl-based function
void py_new_function(py_Ref, py_CFunction, const char* sig, BindType bt);
void py_new_function2(py_Ref, py_CFunction, const char* sig, BindType bt, const char* docstring, const py_Ref userdata);
// old style argc-based function
void py_new_nativefunc(py_Ref, py_CFunction, int argc, BindType bt);
void py_new_nativefunc2(py_Ref, py_CFunction, int argc, BindType bt, const char* docstring, const py_Ref userdata);


py_Ref py_new_module(const char* name);
const py_Ref py_import(const char* name);

#define py_isnull(self) ((self)->type == 0)


/// Sets the name of the object to the given value.
void py_setdict(py_Ref self, py_Name name, const py_Ref val);
/// Returns a reference to the name of the object.
py_Ref py_getdict(const py_Ref self, py_Name name);

/// Sets the i-th slot of the object to the given value.
void py_setslot(py_Ref self, int i, const py_Ref val);
/// Returns a reference to the i-th slot of the object.
py_Ref py_getslot(const py_Ref self, int i);

/// Sets the attribute of the object to the given value.
/// This is equivalent to `self.name = val`.
/// Returns 0 | err
int py_setattr(py_Ref self, py_Name name, const py_Ref val);

/// Gets the attribute of the object.
/// This is equivalent to `self.name`.
/// Returns 0 | err
int py_getattr(const py_Ref self, py_Name name, py_Ref out);

void py_pushref(const py_Ref src);
void py_copyref(const py_Ref src, py_Ref dst);

#ifdef __cplusplus
}
#endif
