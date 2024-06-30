#pragma once

#include <stdint.h>
#include <stdbool.h>

/************* Public Types *************/
typedef struct py_TValue py_TValue;
typedef struct pk_VM pk_VM;
typedef uint16_t py_Name;
typedef int16_t py_Type;
typedef py_TValue* py_Ref;
typedef struct py_Str py_Str;

typedef struct py_Error {
    py_Type type;
} py_Error;

/// Native function signature.
/// @param argc number of arguments.
/// @param argv array of arguments. Use `py_arg(i)` macro to get the i-th argument.
/// @return true if the function is successful.
typedef bool (*py_CFunction)(int argc, py_TValue* argv);

typedef enum BindType {
    BindType_FUNCTION,
    BindType_STATICMETHOD,
    BindType_CLASSMETHOD,
} BindType;

extern pk_VM* pk_current_vm;

/************* Global VMs *************/
void py_initialize();
void py_finalize();

/// Run a simple source string. Do not change the stack.
int py_exec(const char*);
/// Eval a simple expression.
/// The result will be set to `vm->last_retval`.
int py_eval(const char*);

/************* Values Creation *************/
void py_newint(py_Ref, int64_t);
void py_newfloat(py_Ref, double);
void py_newbool(py_Ref, bool);
void py_newstr(py_Ref, const char*);
void py_newstrn(py_Ref, const char*, int);
void py_newStr_(py_Ref, py_Str);
// void py_newfstr(py_Ref, const char*, ...);
void py_newbytes(py_Ref, const unsigned char*, int);
void py_newnone(py_Ref);
void py_newnull(py_Ref);

/// Create a tuple with n UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newtuple(py_Ref, int n);
/// Create a list.
void py_newlist(py_Ref);
/// Create a list with n UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newlistn(py_Ref, int n);

// opaque types
void py_newdict(py_Ref);
void py_newset(py_Ref);
void py_newslice(py_Ref, const py_Ref start, const py_Ref stop, const py_Ref step);

// new style decl-based function
void py_newfunction(py_Ref out, py_CFunction, const char* sig);
void py_newfunction2(py_Ref out,
                     py_CFunction,
                     const char* sig,
                     BindType bt,
                     const char* docstring,
                     const py_Ref upvalue);
// old style argc-based function
void py_newnativefunc(py_Ref out, py_CFunction);

void py_newnotimplemented(py_Ref out);

/// Create a new object.
/// @param out output reference.
/// @param type type of the object.
/// @param slots number of slots. Use -1 to create a `__dict__`.
/// @param udsize size of your userdata. You can use `py_touserdata()` to get the pointer to it.
void py_newobject(py_Ref out, py_Type type, int slots, int udsize);
/************* Type Cast *************/
int64_t py_toint(const py_Ref);
double py_tofloat(const py_Ref);
bool py_castfloat(const py_Ref, double* out);
bool py_tobool(const py_Ref);
const char* py_tostr(const py_Ref);
const char* py_tostrn(const py_Ref, int* out);

void* py_touserdata(const py_Ref);

#define py_isint(self) py_istype(self, tp_int)
#define py_isfloat(self) py_istype(self, tp_float)
#define py_isbool(self) py_istype(self, tp_bool)
#define py_isstr(self) py_istype(self, tp_str)

bool py_istype(const py_Ref, py_Type);
// bool py_isinstance(const py_Ref obj, py_Type type);
// bool py_issubclass(py_Type derived, py_Type base);

/************* References *************/
#define TypeError(x) false
#define py_arg(i) (py_Ref)((char*)argv + ((i) << 4))
#define py_checkargc(n)                                                                            \
    if(argc != n) return TypeError()

py_Ref py_tpmagic(py_Type type, py_Name name);
#define py_bindmagic(type, __magic__, f) py_newnativefunc(py_tpmagic((type), __magic__), (f))

// new style decl-based bindings
py_Ref py_bind(py_Ref obj, const char* sig, py_CFunction f);
py_Ref py_bind2(py_Ref obj,
                const char* sig,
                py_CFunction f,
                BindType bt,
                const char* docstring,
                const py_Ref upvalue);
// old style argc-based bindings
void py_bindmethod(py_Type type, const char* name, py_CFunction f);
void py_bindmethod2(py_Type type, const char* name, py_CFunction f, BindType bt);
void py_bindnativefunc(py_Ref obj, const char* name, py_CFunction f);

py_Ref py_reg(int i);

py_Ref py_getdict(const py_Ref self, py_Name name);
void py_setdict(py_Ref self, py_Name name, const py_Ref val);

py_Ref py_getslot(const py_Ref self, int i);
void py_setslot(py_Ref self, int i, const py_Ref val);

py_Ref py_getupvalue(py_Ref self);
void py_setupvalue(py_Ref self, const py_Ref val);

/// Gets the attribute of the object.
bool py_getattr(const py_Ref self, py_Name name, py_Ref out);
/// Gets the unbound method of the object.
bool py_getunboundmethod(const py_Ref self,
                         py_Name name,
                         bool fallback,
                         py_Ref out,
                         py_Ref out_self);
/// Sets the attribute of the object.
bool py_setattr(py_Ref self, py_Name name, const py_Ref val);
/// Deletes the attribute of the object.
bool py_delattr(py_Ref self, py_Name name);

bool py_getitem(const py_Ref self, const py_Ref key, py_Ref out);
bool py_setitem(py_Ref self, const py_Ref key, const py_Ref val);
bool py_delitem(py_Ref self, const py_Ref key);

/// Perform a binary operation on the stack.
/// It assumes `lhs` and `rhs` are already pushed to the stack.
/// The result will be set to `vm->last_retval`.
bool py_binaryop(const py_Ref lhs, const py_Ref rhs, py_Name op, py_Name rop);

#define py_binaryadd(lhs, rhs) py_binaryop(lhs, rhs, __add__, __radd__)
#define py_binarysub(lhs, rhs) py_binaryop(lhs, rhs, __sub__, __rsub__)
#define py_binarymul(lhs, rhs) py_binaryop(lhs, rhs, __mul__, __rmul__)
#define py_binarytruediv(lhs, rhs) py_binaryop(lhs, rhs, __truediv__, __rtruediv__)
#define py_binaryfloordiv(lhs, rhs) py_binaryop(lhs, rhs, __floordiv__, __rfloordiv__)
#define py_binarymod(lhs, rhs) py_binaryop(lhs, rhs, __mod__, __rmod__)
#define py_binarypow(lhs, rhs) py_binaryop(lhs, rhs, __pow__, __rpow__)

#define py_binarylshift(lhs, rhs) py_binaryop(lhs, rhs, __lshift__, 0)
#define py_binaryrshift(lhs, rhs) py_binaryop(lhs, rhs, __rshift__, 0)
#define py_binaryand(lhs, rhs) py_binaryop(lhs, rhs, __and__, 0)
#define py_binaryor(lhs, rhs) py_binaryop(lhs, rhs, __or__, 0)
#define py_binaryxor(lhs, rhs) py_binaryop(lhs, rhs, __xor__, 0)
#define py_binarymatmul(lhs, rhs) py_binaryop(lhs, rhs, __matmul__, 0)

/// Equivalent to `*dst = *src`.
void py_assign(py_Ref dst, const py_Ref src);

/************* Stack Operations *************/
py_Ref py_gettop();
void py_settop(const py_Ref);
py_Ref py_getsecond();
void py_setsecond(const py_Ref);
void py_duptop();
void py_dupsecond();
/// Returns a reference to the i-th object from the top of the stack.
/// i should be negative, e.g. (-1) means TOS.
py_Ref py_peek(int i);
/// Pops an object from the stack.
void py_pop();
void py_shrink(int n);

/// Pushes the object to the stack.
void py_push(const py_Ref src);

/// Get a temporary variable from the stack and returns the reference to it.
py_Ref py_pushtmp();
/// Free n temporary variable.
void py_poptmp(int n);

/************* Modules *************/
py_Ref py_newmodule(const char* name, const char* package);
py_Ref py_getmodule(const char* name);

/// Import a module.
bool py_import(const char* name, py_Ref out);

/************* Errors *************/
py_Error* py_lasterror();
void py_Error__print(py_Error*);

/************* Operators *************/
bool py_bool(const py_Ref);

int py_eq(const py_Ref, const py_Ref);
int py_ne(const py_Ref, const py_Ref);
int py_le(const py_Ref, const py_Ref);
int py_lt(const py_Ref, const py_Ref);
int py_ge(const py_Ref, const py_Ref);
int py_gt(const py_Ref, const py_Ref);

bool py_hash(const py_Ref, int64_t* out);

/// Compare two objects without using magic methods.
bool py_isidentical(const py_Ref, const py_Ref);

/// A stack operation that calls a function.
/// It assumes `argc + kwargc` arguments are already pushed to the stack.
/// The result will be set to `vm->last_retval`.
/// The stack size will be reduced by `argc + kwargc`.
bool pk_vectorcall(int argc, int kwargc, bool op_call);
/// Call a function.
/// It prepares the stack and then performs a `vectorcall(argc, 0, false)`.
/// The result will be set to `vm->last_retval`.
/// The stack remains unchanged after the operation.
bool py_call(py_Ref f, int argc, py_Ref argv);
/// Call a non-magic method.
/// It prepares the stack and then performs a `vectorcall(argc+1, 0, false)`.
/// The result will be set to `vm->last_retval`.
/// The stack remains unchanged after the operation.
bool py_callmethod(py_Ref self, py_Name, int argc, py_Ref argv);
/// Call a magic method.
/// The result will be set to `vm->last_retval`.
/// The stack remains unchanged after the operation.
bool py_callmagic(py_Name name, int argc, py_Ref argv);

#define py_repr(self) py_callmagic(__repr__, 1, self)
#define py_str(self) py_callmagic(__str__, 1, self)

/// The return value of the most recent vectorcall.
py_Ref py_lastretval();

#define py_isnull(self) ((self)->type == 0)

/* tuple */

// unchecked functions, if self is not a tuple, the behavior is undefined
py_Ref py_tuple__getitem(const py_Ref self, int i);
void py_tuple__setitem(py_Ref self, int i, const py_Ref val);
int py_tuple__len(const py_Ref self);

// unchecked functions, if self is not a list, the behavior is undefined
py_Ref py_list__getitem(const py_Ref self, int i);
void py_list__setitem(py_Ref self, int i, const py_Ref val);
void py_list__delitem(py_Ref self, int i);
int py_list__len(const py_Ref self);
void py_list__append(py_Ref self, const py_Ref val);
void py_list__clear(py_Ref self);
void py_list__insert(py_Ref self, int i, const py_Ref val);

// internal functions
typedef struct pk_TypeInfo pk_TypeInfo;
pk_TypeInfo* pk_tpinfo(const py_Ref self);

/// Search the magic method from the given type to the base type.
/// Returns the reference or NULL if not found.
/// @lifespan: Permanent.
py_Ref py_tpfindmagic(py_Type, py_Name name);

/// Get the type object of the given type.
/// @lifespan: Permanent.
py_Ref py_tpobject(py_Type type);

#define MAGIC_METHOD(x) extern uint16_t x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD

#ifdef __cplusplus
}
#endif
