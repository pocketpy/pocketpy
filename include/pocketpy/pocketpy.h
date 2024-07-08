#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "pocketpy/common/config.h"
#include "pocketpy/common/export.h"

/************* Public Types *************/
typedef struct py_TValue py_TValue;
typedef uint16_t py_Name;
typedef int16_t py_Type;

typedef int64_t py_i64;
typedef double py_f64;

/* string_view */
typedef struct c11_sv{
    const char* data;
    int size;
} c11_sv;

/// Generic reference.
typedef py_TValue* py_Ref;
/// An object reference which has the same lifespan as the object.
typedef py_TValue* py_ObjectRef;
/// A global reference which has the same lifespan as the VM.
typedef py_TValue* py_GlobalRef;
/// A specific location in the stack.
typedef py_TValue* py_StackRef;
/// A temporary reference which has a short or unknown lifespan.
typedef py_TValue* py_TmpRef;

/// Native function signature.
/// @param argc number of arguments.
/// @param argv array of arguments. Use `py_arg(i)` macro to get the i-th argument.
/// @return true if the function is successful.
typedef bool (*py_CFunction)(int argc, py_StackRef argv);

enum BindType {
    BindType_FUNCTION,
    BindType_STATICMETHOD,
    BindType_CLASSMETHOD,
};

enum CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, JSON_MODE, CELL_MODE };

/************* Global VMs *************/
void py_initialize();
void py_finalize();

/// Run a simple source string. Do not change the stack.
bool py_exec(const char* source);
/// Eval a simple expression.
/// The result will be set to `py_retval()`.
bool py_eval(const char* source);
bool py_exec2(const char* source, const char* filename, enum CompileMode mode);

/************* Values Creation *************/
void py_newint(py_Ref, py_i64);
void py_newfloat(py_Ref, py_f64);
void py_newbool(py_Ref, bool);
void py_newstr(py_Ref, const char*);
void py_newstrn(py_Ref, const char*, int);
unsigned char* py_newbytes(py_Ref, int);
void py_newnone(py_Ref);
void py_newnotimplemented(py_Ref out);
void py_newellipsis(py_Ref out);
void py_newnil(py_Ref);

/// Create a tuple with n UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newtuple(py_Ref, int n);
/// Create a list.
void py_newlist(py_Ref);
/// Create a list with n UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newlistn(py_Ref, int n);

py_Name py_name(const char*);
const char* py_name2str(py_Name);
py_Name py_namev(c11_sv name);
c11_sv py_name2sv(py_Name);

bool py_ismagicname(py_Name);

// opaque types
void py_newdict(py_Ref);
void py_newset(py_Ref);
void py_newslice(py_Ref);
// old style argc-based function
void py_newnativefunc(py_Ref out, py_CFunction);

/// Create a new object.
/// @param out output reference.
/// @param type type of the object.
/// @param slots number of slots. Use -1 to create a `__dict__`.
/// @param udsize size of your userdata. You can use `py_touserdata()` to get the pointer to it.
void* py_newobject(py_Ref out, py_Type type, int slots, int udsize);
/************* Type Cast *************/
py_i64 py_toint(const py_Ref);
py_f64 py_tofloat(const py_Ref);
bool py_castfloat(const py_Ref, py_f64* out);
bool py_tobool(const py_Ref);
py_Type py_totype(const py_Ref);
const char* py_tostr(const py_Ref);
const char* py_tostrn(const py_Ref, int* size);
c11_sv py_tosv(const py_Ref);
unsigned char* py_tobytes(const py_Ref, int* size);

void* py_touserdata(const py_Ref);

#define py_isint(self) py_istype(self, tp_int)
#define py_isfloat(self) py_istype(self, tp_float)
#define py_isbool(self) py_istype(self, tp_bool)
#define py_isstr(self) py_istype(self, tp_str)

bool py_istype(const py_Ref, py_Type);
bool py_isinstance(const py_Ref obj, py_Type type);
bool py_issubclass(py_Type derived, py_Type base);

/************* References *************/
#define PY_CHECK_ARGC(n)                                                                           \
    if(argc != n) return TypeError("expected %d arguments, got %d", n, argc)

#define PY_CHECK_ARG_TYPE(i, type)                                                                 \
    if(!py_checktype(py_arg(i), type)) return false

#define py_offset(p, i) ((py_Ref)((char*)p + ((i) << 4)))
#define py_arg(i) py_offset(argv, i)

py_GlobalRef py_tpmagic(py_Type type, py_Name name);
#define py_bindmagic(type, __magic__, f) py_newnativefunc(py_tpmagic((type), __magic__), (f))

// new style decl-based bindings
void py_bind(py_Ref obj, const char* sig, py_CFunction f);
py_ObjectRef py_bind2(py_Ref obj,
              const char* sig,
              py_CFunction f,
              enum BindType bt,
              const char* docstring,
              int slots);

py_ObjectRef py_bind3(py_Ref obj,
              py_CFunction f,
              c11_sv name,
              c11_sv* args,
              int argc,
              c11_sv starred_arg,
              c11_sv* kwargs,
              int kwargc,
              py_Ref kwdefaults,  // a tuple contains default values
              c11_sv starred_kwarg,
              enum BindType bt,
              const char* docstring,
              int slots);

// old style argc-based bindings
void py_bindmethod(py_Type type, const char* name, py_CFunction f);
void py_bindmethod2(py_Type type, const char* name, py_CFunction f, enum BindType bt);
void py_bindnativefunc(py_Ref obj, const char* name, py_CFunction f);

/// Get the reference to the i-th register.
/// All registers are located in a contiguous memory.
py_GlobalRef py_reg(int i);

/// Get the reference of the object's `__dict__`.
/// The object must have a `__dict__`.
/// Returns a reference to the value or NULL if not found.
py_ObjectRef py_getdict(const py_Ref self, py_Name name);
void py_setdict(py_Ref self, py_Name name, const py_Ref val);

/// Get the reference of the i-th slot of the object.
/// The object must have slots and `i` must be in range.
py_ObjectRef py_getslot(const py_Ref self, int i);
void py_setslot(py_Ref self, int i, const py_Ref val);

py_TmpRef py_getupvalue(py_StackRef argv);
void py_setupvalue(py_StackRef argv, const py_Ref val);

/// Gets the attribute of the object.
/// 1: success, 0: not found, -1: error
int py_getattr(const py_Ref self, py_Name name, py_Ref out);
/// Sets the attribute of the object.
bool py_setattr(py_Ref self, py_Name name, const py_Ref val);
/// Deletes the attribute of the object.
bool py_delattr(py_Ref self, py_Name name);
/// Gets the unbound method of the object.
bool py_getunboundmethod(py_Ref self, py_Name name, py_Ref out, py_Ref out_self);

bool py_getitem(const py_Ref self, const py_Ref key, py_Ref out);
bool py_setitem(py_Ref self, const py_Ref key, const py_Ref val);
bool py_delitem(py_Ref self, const py_Ref key);

/// Perform a binary operation on the stack.
/// It assumes `lhs` and `rhs` are already pushed to the stack.
/// The result will be set to `py_retval()`.
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
/// Return a reference to the i-th object from the top of the stack.
/// i should be negative, e.g. (-1) means TOS.
py_StackRef py_peek(int i);
/// Push the object to the stack.
void py_push(const py_Ref src);
/// Push a nil object to the stack.
void py_pushnil();
/// Pop an object from the stack.
void py_pop();
/// Shrink the stack by n.
void py_shrink(int n);
/// Get a temporary variable from the stack and returns the reference to it.
py_StackRef py_pushtmp();
/// Free n temporary variable.
#define py_poptmp(n) py_shrink(n)

#define py_gettop() py_peek(-1)
#define py_getsecond() py_peek(-2)
#define py_settop(v) py_assign(py_peek(-1), v)
#define py_setsecond(v) py_assign(py_peek(-2), v)
#define py_duptop() py_push(py_peek(-1))
#define py_dupsecond() py_push(py_peek(-2))
/************* Modules *************/
py_TmpRef py_newmodule(const char* name, const char* package);
py_TmpRef py_getmodule(const char* name);

/// Import a module.
/// The result will be set to `py_retval()`.
bool py_import(const char* name);

/************* Errors *************/
bool py_exception(const char* name, const char* fmt, ...);
/// Print the last error to the console.
void py_printexc();
/// Format the last error to a string.
void py_formatexc(char* out);

#define KeyError(q) py_exception("KeyError", "%q", (q))
#define NameError(n) py_exception("NameError", "name '%n' is not defined", (n))
#define TypeError(...) py_exception("TypeError", __VA_ARGS__)
#define ValueError(...) py_exception("ValueError", __VA_ARGS__)
#define IndexError(...) py_exception("IndexError", __VA_ARGS__)
#define NotImplementedError() py_exception("NotImplementedError", "")
#define AttributeError(self, n)                                                                    \
    py_exception("AttributeError", "'%t' object has no attribute '%n'", (self)->type, (n))
#define UnboundLocalError(n)                                                                       \
    py_exception("UnboundLocalError", "local variable '%n' referenced before assignment", (n))

bool StopIteration();

/************* Operators *************/
/// Equivalent to `bool(val)`.
/// Returns 1 if `val` is truthy, otherwise 0.
/// Returns -1 if an error occurred.
int py_bool(const py_Ref val);

int py_eq(const py_Ref, const py_Ref);
int py_ne(const py_Ref, const py_Ref);
int py_le(const py_Ref, const py_Ref);
int py_lt(const py_Ref, const py_Ref);
int py_ge(const py_Ref, const py_Ref);
int py_gt(const py_Ref, const py_Ref);

bool py_hash(const py_Ref, py_i64* out);

/// Get the iterator of the object.
bool py_iter(const py_Ref);
/// Get the next element from the iterator.
/// 1: success, 0: StopIteration, -1: error
int py_next(const py_Ref);

/// Python equivalent to `lhs is rhs`.
bool py_isidentical(const py_Ref, const py_Ref);

/// A stack operation that calls a function.
/// It assumes `argc + kwargc` arguments are already pushed to the stack.
/// The result will be set to `py_retval()`.
/// The stack size will be reduced by `argc + kwargc`.
bool py_vectorcall(uint16_t argc, uint16_t kwargc);
/// Call a function.
/// It prepares the stack and then performs a `vectorcall(argc, 0, false)`.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
bool py_call(py_Ref f, int argc, py_Ref argv);
/// Call a non-magic method.
/// It prepares the stack and then performs a `vectorcall(argc+1, 0, false)`.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
bool py_callmethod(py_Ref self, py_Name, int argc, py_Ref argv);
/// Call a magic method using a continuous buffer.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
bool py_callmagic(py_Name name, int argc, py_Ref argv);

bool py_str(py_Ref val);

#define py_repr(val) py_callmagic(__repr__, 1, val)
#define py_len(val) py_callmagic(__len__, 1, val)

/// The return value of the most recent call.
py_GlobalRef py_retval();

#define py_isnil(self) ((self)->type == 0)
#define py_isnone(self) ((self)->type == tp_none_type)

/* tuple */

// unchecked functions, if self is not a tuple, the behavior is undefined
py_ObjectRef py_tuple__getitem(const py_Ref self, int i);
void py_tuple__setitem(py_Ref self, int i, const py_Ref val);
int py_tuple__len(const py_Ref self);

// unchecked functions, if self is not a list, the behavior is undefined
py_ObjectRef py_list__getitem(const py_Ref self, int i);
py_ObjectRef py_list__data(const py_Ref self);
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
/// Return the reference or NULL if not found.
py_GlobalRef py_tpfindmagic(py_Type, py_Name name);

/// Search the name from the given type to the base type.
/// Return the reference or NULL if not found.
py_GlobalRef py_tpfindname(py_Type, py_Name name);

/// Get the type object of the given type.
py_GlobalRef py_tpobject(py_Type type);

/// Get the type name.
const char* py_tpname(py_Type type);

/// Call a type to create a new instance.
bool py_tpcall(py_Type type, int argc, py_Ref argv);

/// Check if the object is an instance of the given type.
bool py_checktype(const py_Ref self, py_Type type);

#define py_checkint(self) py_checktype(self, tp_int)
#define py_checkfloat(self) py_checktype(self, tp_float)
#define py_checkbool(self) py_checktype(self, tp_bool)
#define py_checkstr(self) py_checktype(self, tp_str)

int py_replinput(char* buf);

/// Python favored string formatting.
/// %d: int
/// %i: py_i64 (int64_t)
/// %f: py_f64 (double)
/// %s: const char*
/// %q: c11_sv
/// %v: c11_sv
/// %c: char
/// %p: void*
/// %t: py_Type
/// %n: py_Name

enum py_MagicNames {
    py_MagicNames__NULL,  // 0 is reserved

#define MAGIC_METHOD(x) x,
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD
};

enum py_PredefinedTypes {
    tp_object = 1,
    tp_type,  // py_Type
    tp_int,
    tp_float,
    tp_bool,
    tp_str,
    tp_str_iterator,
    tp_list,   // c11_vector
    tp_tuple,  // N slots
    tp_array_iterator,
    tp_slice,  // 3 slots (start, stop, step)
    tp_range,
    tp_range_iterator,
    tp_module,
    tp_function,
    tp_nativefunc,
    tp_bound_method,
    tp_super,  // 1 slot + py_Type
    tp_exception,
    tp_bytes,
    tp_mappingproxy,
    tp_dict,
    tp_property,  // 2 slots (getter + setter)
    tp_star_wrapper,
    tp_staticmethod,  // 1 slot
    tp_classmethod,   // 1 slot
    tp_none_type,
    tp_not_implemented_type,
    tp_ellipsis,
    tp_syntax_error,
    tp_stop_iteration,
};

#ifdef __cplusplus
}
#endif

/*
Some notes:

## Macros
1. Function macros are partial functions. They can be used as normal expressions. Use the same
naming convention as functions.
2. Snippet macros are `do {...} while(0)` blocks. They cannot be used as expressions. Use
`UPPER_CASE` naming convention.
3. Constant macros are used for global constants. Use `UPPER_CASE` or k-prefix naming convention.
*/