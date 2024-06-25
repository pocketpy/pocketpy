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

typedef struct py_Str py_Str;

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
void py_newint(py_Ref, int64_t);
void py_newfloat(py_Ref, double);
void py_newbool(py_Ref, bool);
void py_newstr(py_Ref, const char*);
void py_newstrn(py_Ref, const char*, int);
// void py_newfstr(py_Ref, const char*, ...);
void py_newbytes(py_Ref, const uint8_t*, int);
void py_newnone(py_Ref);
void py_newnull(py_Ref);

void py_newtuple(py_Ref, int);

// new style decl-based function
void py_newfunction(py_Ref, py_CFunction, const char* sig, BindType bt);
void py_newfunction2(py_Ref, py_CFunction, const char* sig, BindType bt, const char* docstring, const py_Ref userdata);
// old style argc-based function
void py_newnativefunc(py_Ref, py_CFunction, int argc, BindType bt);
void py_newnativefunc2(py_Ref, py_CFunction, int argc, BindType bt, const char* docstring, const py_Ref userdata);


py_Ref py_newmodule(const char* name, const char* package);
py_Ref py_getmodule(const char* name);
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

/// Equivalent to `self.name = val`.
/// Returns 0 | err
int py_setattr(py_Ref self, py_Name name, const py_Ref val);

/// Equivalent to `self.name`.
/// Returns 0 | err
int py_getattr(const py_Ref self, py_Name name, py_Ref out);

/// Returns a reference to the i-th object from the top of the stack.
/// i should be negative, e.g. (-1) means TOS.
py_Ref py_stack(int i);
/// Returns a reference to the i-th register.
py_Ref py_reg(int i);
/// Returns a reference to the i-th system register.
py_Ref py_sysreg(int i);
/// Prepares a push and returns an uninitialized reference.
py_Ref py_push();


/// Pushes the object to the stack.
void py_pushref(const py_Ref src);
/// Pops the object from the stack.
void py_copyref(const py_Ref src, py_Ref dst);


/* tuple */

// unchecked functions, if self is not a tuple, the behavior is undefined
py_Ref py_tuple__getitem(const py_Ref self, int i);
void py_tuple__setitem(py_Ref self, int i, const py_Ref val);
int py_tuple__len(const py_Ref self);
bool py_tuple__contains(const py_Ref self, const py_Ref val);

// unchecked functions, if self is not a list, the behavior is undefined
py_Ref py_list__getitem(const py_Ref self, int i);
void py_list__setitem(py_Ref self, int i, const py_Ref val);
void py_list__delitem(py_Ref self, int i);
int py_list__len(const py_Ref self);
bool py_list__contains(const py_Ref self, const py_Ref val);
void py_list__append(py_Ref self, const py_Ref val);
void py_list__extend(py_Ref self, const py_Ref begin, const py_Ref end);
void py_list__clear(py_Ref self);
void py_list__insert(py_Ref self, int i, const py_Ref val);

// unchecked functions, if self is not a dict, the behavior is undefined
int py_dict__len(const py_Ref self);
bool py_dict__contains(const py_Ref self, const py_Ref key);
py_Ref py_dict__getitem(const py_Ref self, const py_Ref key);
void py_dict__setitem(py_Ref self, const py_Ref key, const py_Ref val);
void py_dict__delitem(py_Ref self, const py_Ref key);
void py_dict__clear(py_Ref self);

int py_str(const py_Ref, py_Str* out);
int py_repr(const py_Ref, py_Str* out);

int py_toint(py_Ref, int64_t* out);
int py_tofloat(py_Ref, double* out);
int py_tostr(py_Ref, py_Str** out);
int py_tobool(py_Ref, bool* out);

bool py_istype(const py_Ref, Type);
bool py_isinstance(const py_Ref obj, Type type);
bool py_issubclass(Type derived, Type base);

#ifdef __cplusplus
}
#endif
