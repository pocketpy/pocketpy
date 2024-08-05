#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "pocketpy/common/config.h"
#include "pocketpy/common/export.h"

#ifdef __cplusplus
extern "C" {
#endif

/************* Public Types *************/
typedef struct py_TValue py_TValue;
typedef uint16_t py_Name;
typedef int16_t py_Type;

typedef int64_t py_i64;
typedef double py_f64;

#define PY_RAISE  // mark a function that can raise an exception

typedef struct c11_sv {
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
typedef bool (*py_CFunction)(int argc, py_StackRef argv) PY_RAISE;

enum py_CompileMode { EXEC_MODE, EVAL_MODE, REPL_MODE, CELL_MODE };

extern py_GlobalRef py_True;
extern py_GlobalRef py_False;
extern py_GlobalRef py_None;
extern py_GlobalRef py_NIL;

/************* Global Setup *************/

/// Initialize pocketpy and the default VM.
void py_initialize();
/// Finalize pocketpy.
void py_finalize();
/// Get the current VM index.
int py_currentvm();
/// Switch to a VM.
/// @param index index of the VM ranging from 0 to 16 (exclusive). `0` is the default VM.
void py_switchvm(int index);

/// Run a source string.
/// @param source source string.
/// @param filename filename (for error messages).
/// @param mode compile mode. Use `EXEC_MODE` for statements `EVAL_MODE` for expressions.
/// @param module target module. Use NULL for the main module.
/// @return true if the execution is successful.
bool py_exec(const char* source,
             const char* filename,
             enum py_CompileMode mode,
             py_Ref module) PY_RAISE;

/************* Values Creation *************/

/// Create an `int` object.
void py_newint(py_Ref, py_i64);
/// Create a `float` object.
void py_newfloat(py_Ref, py_f64);
/// Create a `bool` object.
void py_newbool(py_Ref, bool);
/// Create a `str` object from a null-terminated string (utf-8).
void py_newstr(py_Ref, const char*);
/// Create a `str` object from a char array (utf-8).
void py_newstrn(py_Ref, const char*, int);
/// Create a `bytes` object with `n` UNINITIALIZED bytes.
unsigned char* py_newbytes(py_Ref, int n);
/// Create a `None` object.
void py_newnone(py_Ref);
/// Create a `NotImplemented` object.
void py_newnotimplemented(py_Ref out);
/// Create a `...` object.
void py_newellipsis(py_Ref out);
/// Create a `nil` object. `nil` is an invalid representation of an object.
/// Don't use it unless you know what you are doing.
void py_newnil(py_Ref);
/// Create a `tuple` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newtuple(py_Ref, int n);
/// Create an empty `list`.
void py_newlist(py_Ref);
/// Create a `list` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
void py_newlistn(py_Ref, int n);
/// Create an empty `dict`.
void py_newdict(py_Ref);
/// Create an UNINITIALIZED `slice` object.
/// You should use `py_setslot()` to set `start`, `stop`, and `step`.
void py_newslice(py_Ref);
/// Create a `nativefunc` object.
void py_newnativefunc(py_Ref out, py_CFunction);
/// Create a `function` object.
py_Name
    py_newfunction(py_Ref out, const char* sig, py_CFunction f, const char* docstring, int slots);
/// Create a `boundmethod` object.
void py_newboundmethod(py_Ref out, py_Ref self, py_Ref func);

/************* Name Convertions *************/

/// Convert a null-terminated string to a name.
py_Name py_name(const char*);
/// Convert a name to a null-terminated string.
const char* py_name2str(py_Name);
/// Convert a name to a `c11_sv`.
py_Name py_namev(c11_sv name);
/// Convert a `c11_sv` to a name.
c11_sv py_name2sv(py_Name);

#define py_ismagicname(name) (name <= __missing__)

/************* Meta Operations *************/

/// Create a new type.
/// @param name name of the type.
/// @param base base type.
/// @param module module where the type is defined. Use `NULL` for built-in types.
/// @param dtor destructor function. Use `NULL` if not needed.
py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, void (*dtor)(void*));

/// Create a new object.
/// @param out output reference.
/// @param type type of the object.
/// @param slots number of slots. Use -1 to create a `__dict__`.
/// @param udsize size of your userdata.
/// @return pointer to the userdata.
void* py_newobject(py_Ref out, py_Type type, int slots, int udsize);

/************* Type Cast *************/

/// Convert an `int` object in python to `int64_t`.
py_i64 py_toint(py_Ref);
/// Convert a `float` object in python to `double`.
py_f64 py_tofloat(py_Ref);
/// Cast a `int` or `float` object in python to `double`.
/// If successful, returns true and set the value to `out`.
/// Otherwise, return false and raise `TypeError`.
bool py_castfloat(py_Ref, py_f64* out) PY_RAISE;
/// Convert a `bool` object in python to `bool`.
bool py_tobool(py_Ref);
/// Convert a `type` object in python to `py_Type`.
py_Type py_totype(py_Ref);
/// Convert a `str` object in python to null-terminated string.
const char* py_tostr(py_Ref);
/// Convert a `str` object in python to char array.
const char* py_tostrn(py_Ref, int* size);
/// Convert a `str` object in python to `c11_sv`.
c11_sv py_tosv(py_Ref);
/// Convert a `bytes` object in python to char array.
unsigned char* py_tobytes(py_Ref, int* size);
/// Convert a user-defined object to its userdata.
void* py_touserdata(py_Ref);

#define py_isint(self) py_istype(self, tp_int)
#define py_isfloat(self) py_istype(self, tp_float)
#define py_isbool(self) py_istype(self, tp_bool)
#define py_isstr(self) py_istype(self, tp_str)
#define py_islist(self) py_istype(self, tp_list)
#define py_istuple(self) py_istype(self, tp_tuple)
#define py_isdict(self) py_istype(self, tp_dict)

#define py_isnil(self) py_istype(self, 0)
#define py_isnone(self) py_istype(self, tp_NoneType)

/// Get the type of the object.
py_Type py_typeof(py_Ref self);
/// Check if the object is exactly the given type.
bool py_istype(py_Ref, py_Type);
/// Check if the object is an instance of the given type.
bool py_isinstance(py_Ref obj, py_Type type);
/// Check if the derived type is a subclass of the base type.
bool py_issubclass(py_Type derived, py_Type base);

/// Search the magic method from the given type to the base type.
/// Return `NULL` if not found.
py_GlobalRef py_tpfindmagic(py_Type, py_Name name);
/// Search the name from the given type to the base type.
/// Return `NULL` if not found.
py_GlobalRef py_tpfindname(py_Type, py_Name name);
/// Get the type object of the given type.
py_GlobalRef py_tpobject(py_Type type);
/// Get the type name.
const char* py_tpname(py_Type type);
/// Call a type to create a new instance.
bool py_tpcall(py_Type type, int argc, py_Ref argv) PY_RAISE;
/// Get the magic method from the given type only.
/// The returned reference is always valid. However, its value may be `nil`.
py_GlobalRef py_tpmagic(py_Type type, py_Name name);

/// Check if the object is an instance of the given type.
/// Raise `TypeError` if the check fails.
bool py_checktype(py_Ref self, py_Type type) PY_RAISE;

#define py_checkint(self) py_checktype(self, tp_int)
#define py_checkfloat(self) py_checktype(self, tp_float)
#define py_checkbool(self) py_checktype(self, tp_bool)
#define py_checkstr(self) py_checktype(self, tp_str)

/************* References *************/
/// Get the i-th register.
/// All registers are located in a contiguous memory.
py_GlobalRef py_getreg(int i);
/// Set the i-th register.
void py_setreg(int i, py_Ref val);

/// Equivalent to `*dst = *src`.
void py_assign(py_Ref dst, py_Ref src);
/// Get the last return value.
py_GlobalRef py_retval();

/// Get an item from the object's `__dict__`.
/// Return `NULL` if not found.
py_ObjectRef py_getdict(py_Ref self, py_Name name);
/// Set an item to the object's `__dict__`.
void py_setdict(py_Ref self, py_Name name, py_Ref val);
/// Delete an item from the object's `__dict__`.
/// Return `true` if the deletion is successful.
bool py_deldict(py_Ref self, py_Name name);
/// Prepare an insertion to the object's `__dict__`.
py_ObjectRef py_emplacedict(py_Ref self, py_Name name);

/// Get the i-th slot of the object.
/// The object must have slots and `i` must be in valid range.
py_ObjectRef py_getslot(py_Ref self, int i);
/// Set the i-th slot of the object.
void py_setslot(py_Ref self, int i, py_Ref val);

/************* Inspection *************/

/// Get the current `function` object from the stack.
/// Return `NULL` if not available.
py_StackRef py_inspect_currentfunction();

/************* Bindings *************/

/// Bind a function to the object via "decl-based" style.
/// @param obj the target object.
/// @param sig signature of the function. e.g. `add(x, y)`.
/// @param f function to bind.
void py_bind(py_Ref obj, const char* sig, py_CFunction f);
/// Bind a method to type via "argc-based" style.
/// @param type the target type.
/// @param name name of the method.
/// @param f function to bind.
void py_bindmethod(py_Type type, const char* name, py_CFunction f);
/// Bind a function to the object via "argc-based" style.
/// @param obj the target object.
/// @param name name of the function.
/// @param f function to bind.
void py_bindfunc(py_Ref obj, const char* name, py_CFunction f);
/// Bind a property to type.
/// @param type the target type.
/// @param name name of the property.
/// @param getter getter function.
/// @param setter setter function. Use `NULL` if not needed.
void py_bindproperty(py_Type type, const char* name, py_CFunction getter, py_CFunction setter);

#define py_bindmagic(type, __magic__, f) py_newnativefunc(py_tpmagic((type), __magic__), (f))

#define PY_CHECK_ARGC(n)                                                                           \
    if(argc != n) return TypeError("expected %d arguments, got %d", n, argc)

#define PY_CHECK_ARG_TYPE(i, type)                                                                 \
    if(!py_checktype(py_arg(i), type)) return false

#define py_offset(p, i) ((py_Ref)((char*)p + ((i) << 4)))
#define py_arg(i) py_offset(argv, i)
/************* Python Equivalents *************/
/// Python equivalent to `getattr(self, name)`.
bool py_getattr(py_Ref self, py_Name name) PY_RAISE;
/// Python equivalent to `setattr(self, name, val)`.
bool py_setattr(py_Ref self, py_Name name, py_Ref val) PY_RAISE;
/// Python equivalent to `delattr(self, name)`.
bool py_delattr(py_Ref self, py_Name name) PY_RAISE;
/// Python equivalent to `self[key]`.
bool py_getitem(py_Ref self, py_Ref key) PY_RAISE;
/// Python equivalent to `self[key] = val`.
bool py_setitem(py_Ref self, py_Ref key, py_Ref val) PY_RAISE;
/// Python equivalent to `del self[key]`.
bool py_delitem(py_Ref self, py_Ref key) PY_RAISE;

/// Perform a binary operation on the stack.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
bool py_binaryop(py_Ref lhs, py_Ref rhs, py_Name op, py_Name rop) PY_RAISE;

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

/************* Stack Operations *************/

/// Get the i-th object from the top of the stack.
/// `i` should be negative, e.g. (-1) means TOS.
py_StackRef py_peek(int i);
/// Push the object to the stack.
void py_push(py_Ref src);
/// Push a `nil` object to the stack.
void py_pushnil();
/// Pop an object from the stack.
void py_pop();
/// Shrink the stack by n.
void py_shrink(int n);
/// Get a temporary variable from the stack.
py_StackRef py_pushtmp();
/// Get the unbound method of the object.
/// Assume the object is located at the top of the stack.
/// If return true:  `[self] -> [unbound, self]`.
/// If return false: `[self] -> [self]` (no change).
bool py_pushmethod(py_Name name);
/// Call a callable object.
/// Assume `argc + kwargc` arguments are already pushed to the stack.
/// The result will be set to `py_retval()`.
/// The stack size will be reduced by `argc + kwargc`.
bool py_vectorcall(uint16_t argc, uint16_t kwargc) PY_RAISE;

/************* Modules *************/

/// Create a new module.
py_TmpRef py_newmodule(const char* path);
/// Get a module by path.
py_TmpRef py_getmodule(const char* path);

/// Import a module.
/// The result will be set to `py_retval()`.
/// -1: error, 0: not found, 1: success
int py_import(const char* path) PY_RAISE;

/************* Errors *************/

/// Raise an exception by type and message. Always return false.
bool py_exception(py_Type type, const char* fmt, ...) PY_RAISE;
/// Raise an expection object. Always return false.
bool py_raise(py_Ref) PY_RAISE;
/// Print the current exception.
void py_printexc();
/// Format the current exception and return a null-terminated string.
/// The result should be freed by the caller.
char* py_formatexc();
/// Check if an exception is raised.
bool py_checkexc();
/// Check if the exception is an instance of the given type.
bool py_matchexc(py_Type type);
/// Clear the current exception.
void py_clearexc(py_StackRef p0);

#define NameError(n) py_exception(tp_NameError, "name '%n' is not defined", (n))
#define TypeError(...) py_exception(tp_TypeError, __VA_ARGS__)
#define RuntimeError(...) py_exception(tp_RuntimeError, __VA_ARGS__)
#define ValueError(...) py_exception(tp_ValueError, __VA_ARGS__)
#define IndexError(...) py_exception(tp_IndexError, __VA_ARGS__)
#define ImportError(...) py_exception(tp_ImportError, __VA_ARGS__)
#define NotImplementedError() py_exception(tp_NotImplementedError, "")
#define AttributeError(self, n)                                                                    \
    py_exception(tp_AttributeError, "'%t' object has no attribute '%n'", (self)->type, (n))
#define UnboundLocalError(n)                                                                       \
    py_exception(tp_UnboundLocalError, "local variable '%n' referenced before assignment", (n))

bool StopIteration();
bool KeyError(py_Ref key) PY_RAISE;

/************* Operators *************/

/// Equivalent to `bool(val)`.
/// Returns 1 if `val` is truthy, otherwise 0.
/// Returns -1 if an error occurred.
int py_bool(py_Ref val) PY_RAISE;

/// Compare two objects.
/// 1: lhs == rhs, 0: lhs != rhs, -1: error
int py_equal(py_Ref lhs, py_Ref rhs) PY_RAISE;
/// Compare two objects.
/// 1: lhs < rhs, 0: lhs >= rhs, -1: error
int py_less(py_Ref lhs, py_Ref rhs) PY_RAISE;

#define py_eq(lhs, rhs) py_binaryop(lhs, rhs, __eq__, __eq__)
#define py_ne(lhs, rhs) py_binaryop(lhs, rhs, __ne__, __ne__)
#define py_lt(lhs, rhs) py_binaryop(lhs, rhs, __lt__, __gt__)
#define py_le(lhs, rhs) py_binaryop(lhs, rhs, __le__, __ge__)
#define py_gt(lhs, rhs) py_binaryop(lhs, rhs, __gt__, __lt__)
#define py_ge(lhs, rhs) py_binaryop(lhs, rhs, __ge__, __le__)

/// Get the hash value of the object.
bool py_hash(py_Ref, py_i64* out) PY_RAISE;
/// Get the iterator of the object.
bool py_iter(py_Ref) PY_RAISE;
/// Get the next element from the iterator.
/// 1: success, 0: StopIteration, -1: error
int py_next(py_Ref) PY_RAISE;
/// Python equivalent to `lhs is rhs`.
bool py_isidentical(py_Ref, py_Ref);
/// Call a function.
/// It prepares the stack and then performs a `vectorcall(argc, 0, false)`.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
bool py_call(py_Ref f, int argc, py_Ref argv) PY_RAISE;

/// Python equivalent to `str(val)`.
bool py_str(py_Ref val) PY_RAISE;
/// Python equivalent to `repr(val)`.
bool py_repr(py_Ref val) PY_RAISE;
/// Python equivalent to `len(val)`.
bool py_len(py_Ref val) PY_RAISE;

/************* Unchecked Functions *************/

py_ObjectRef py_tuple_data(py_Ref self);
py_ObjectRef py_tuple_getitem(py_Ref self, int i);
void py_tuple_setitem(py_Ref self, int i, py_Ref val);
int py_tuple_len(py_Ref self);

py_TmpRef py_list_data(py_Ref self);
py_TmpRef py_list_getitem(py_Ref self, int i);
void py_list_setitem(py_Ref self, int i, py_Ref val);
void py_list_delitem(py_Ref self, int i);
int py_list_len(py_Ref self);
void py_list_append(py_Ref self, py_Ref val);
void py_list_clear(py_Ref self);
void py_list_insert(py_Ref self, int i, py_Ref val);
void py_list_reverse(py_Ref self);

py_TmpRef py_dict_getitem(py_Ref self, py_Ref key) PY_RAISE;
void py_dict_setitem(py_Ref self, py_Ref key, py_Ref val) PY_RAISE;
void py_dict_delitem(py_Ref self, py_Ref key) PY_RAISE;
bool py_dict_contains(py_Ref self, py_Ref key) PY_RAISE;
int py_dict_len(py_Ref self);
bool py_dict_apply(py_Ref self, bool (*f)(py_Ref key, py_Ref val, void* ctx), void* ctx) PY_RAISE;

/************* Others *************/

/// An utility function to read a line from stdin for REPL.
int py_replinput(char* buf, int max_size);

/// Python favored string formatting. (just put here, not for users)
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
    tp_boundmethod,
    tp_super,          // 1 slot + py_Type
    tp_BaseException,  // 2 slots (arg + inner exc)
    tp_Exception,
    tp_bytes,
    tp_mappingproxy,
    tp_dict,
    tp_dict_items,    // 1 slot
    tp_property,      // 2 slots (getter + setter)
    tp_star_wrapper,  // 1 slot + int level
    tp_staticmethod,  // 1 slot
    tp_classmethod,   // 1 slot
    tp_NoneType,
    tp_NotImplementedType,
    tp_ellipsis,
    tp_SyntaxError,
    tp_StopIteration,
    /* builtin exceptions */
    tp_StackOverflowError,
    tp_IOError,
    tp_OSError,
    tp_NotImplementedError,
    tp_TypeError,
    tp_IndexError,
    tp_ValueError,
    tp_RuntimeError,
    tp_ZeroDivisionError,
    tp_NameError,
    tp_UnboundLocalError,
    tp_AttributeError,
    tp_ImportError,
    tp_AssertionError,
    tp_KeyError,
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