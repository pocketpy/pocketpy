#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pocketpy/config.h"
#include "pocketpy/export.h"
#include "pocketpy/vmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/************* Public Types *************/
/// A helper struct for `py_Name`.
typedef struct py_OpaqueName py_OpaqueName;
/// A pointer that represents a python identifier. For fast name resolution.
typedef py_OpaqueName* py_Name;
/// A opaque type that represents a python object. You cannot access its members directly.
typedef struct py_TValue py_TValue;
/// An integer that represents a python type. `0` is invalid.
typedef int16_t py_Type;
/// A 64-bit integer type. Corresponds to `int` in python.
typedef int64_t py_i64;
/// A 64-bit floating-point type. Corresponds to `float` in python.
typedef double py_f64;
/// A generic destructor function.
typedef void (*py_Dtor)(void*);

#ifdef PK_IS_PUBLIC_INCLUDE
typedef struct py_TValue {
    py_Type type;
    bool is_ptr;
    int extra;

    union {
        int64_t _i64;
        char _chars[16];
    };
} py_TValue;
#endif

/// A string view type. It is helpful for passing strings which are not null-terminated.
typedef struct c11_sv {
    const char* data;
    int size;
} c11_sv;

#define PY_RAISE
#define PY_RETURN

/// A generic reference to a python object.
typedef py_TValue* py_Ref;
/// A reference which has the same lifespan as the python object.
typedef py_TValue* py_ObjectRef;
/// A global reference which has the same lifespan as the VM.
typedef py_TValue* py_GlobalRef;
/// A specific location in the value stack of the VM.
typedef py_TValue* py_StackRef;
/// An item reference to a container object. It invalidates when the container is modified.
typedef py_TValue* py_ItemRef;
/// An output reference for returning a value. Only use this for function arguments.
typedef py_TValue* py_OutRef;

typedef struct py_Frame py_Frame;

// An enum for tracing events.
enum py_TraceEvent {
    TRACE_EVENT_LINE,
    TRACE_EVENT_PUSH,
    TRACE_EVENT_POP,
};

typedef void (*py_TraceFunc)(py_Frame* frame, enum py_TraceEvent);

/// A struct contains the callbacks of the VM.
typedef struct py_Callbacks {
    /// Used by `__import__` to load a source module.
    char* (*importfile)(const char*);
    /// Called before `importfile` to lazy-import a C module.
    py_GlobalRef (*lazyimport)(const char*);
    /// Used by `print` to output a string.
    void (*print)(const char*);
    /// Flush the output buffer of `print`.
    void (*flush)();
    /// Used by `input` to get a character.
    int (*getchr)();
    /// Used by `gc.collect()` to mark extra objects for garbage collection.
    void (*gc_mark)(void (*f)(py_Ref val, void* ctx), void* ctx);
} py_Callbacks;

/// Native function signature.
/// @param argc number of arguments.
/// @param argv array of arguments. Use `py_arg(i)` macro to get the i-th argument.
/// @return `true` if the function is successful or `false` if an exception is raised.
typedef bool (*py_CFunction)(int argc, py_StackRef argv) PY_RAISE PY_RETURN;

/// Python compiler modes.
/// + `EXEC_MODE`: for statements.
/// + `EVAL_MODE`: for expressions.
/// + `SINGLE_MODE`: for REPL or jupyter notebook execution.
/// + `RELOAD_MODE`: for reloading a module without allocating new types if possible.
enum py_CompileMode { EXEC_MODE, EVAL_MODE, SINGLE_MODE, RELOAD_MODE };

/************* Global Setup *************/

/// Initialize pocketpy and the default VM.
PK_API void py_initialize();
/// Finalize pocketpy and free all VMs. This opearation is irreversible.
/// After this call, you cannot use any function from this header anymore.
PK_API void py_finalize();
/// Get the current VM index.
PK_API int py_currentvm();
/// Switch to a VM.
/// @param index index of the VM ranging from 0 to 16 (exclusive). `0` is the default VM.
PK_API void py_switchvm(int index);
/// Reset the current VM.
PK_API void py_resetvm();
/// Reset All VMs.
PK_API void py_resetallvm();
/// Get the current VM context. This is used for user-defined data.
PK_API void* py_getvmctx();
/// Set the current VM context. This is used for user-defined data.
PK_API void py_setvmctx(void* ctx);
/// Setup the callbacks for the current VM.
PK_API py_Callbacks* py_callbacks();

/// Set `sys.argv`. Used for storing command-line arguments.
PK_API void py_sys_setargv(int argc, char** argv);
/// Set the trace function for the current VM.
PK_API void py_sys_settrace(py_TraceFunc func, bool reset);
/// Invoke the garbage collector.
PK_API int py_gc_collect();

/// Wrapper for `PK_MALLOC(size)`.
PK_API void* py_malloc(size_t size);
/// Wrapper for `PK_REALLOC(ptr, size)`.
PK_API void* py_realloc(void* ptr, size_t size);
/// Wrapper for `PK_FREE(ptr)`.
PK_API void py_free(void* ptr);

/// A shorthand for `True`.
PK_API py_GlobalRef py_True();
/// A shorthand for `False`.
PK_API py_GlobalRef py_False();
/// A shorthand for `None`.
PK_API py_GlobalRef py_None();
/// A shorthand for `nil`. `nil` is not a valid python object.
PK_API py_GlobalRef py_NIL();

/************* Frame Ops *************/

/// Get the current source location of the frame.
PK_API const char* py_Frame_sourceloc(py_Frame* frame, int* lineno);
/// Python equivalent to `globals()` with respect to the given frame.
PK_API void py_Frame_newglobals(py_Frame* frame, py_OutRef out);
/// Python equivalent to `locals()` with respect to the given frame.
PK_API void py_Frame_newlocals(py_Frame* frame, py_OutRef out);
/// Get the function object of the frame.
/// Returns `NULL` if not available.
PK_API py_StackRef py_Frame_function(py_Frame* frame);

/************* Code Execution *************/

/// Compile a source string into a code object.
/// Use python's `exec()` or `eval()` to execute it.
PK_API bool py_compile(const char* source,
                       const char* filename,
                       enum py_CompileMode mode,
                       bool is_dynamic) PY_RAISE PY_RETURN;
/// Run a source string.
/// @param source source string.
/// @param filename filename (for error messages).
/// @param mode compile mode. Use `EXEC_MODE` for statements `EVAL_MODE` for expressions.
/// @param module target module. Use NULL for the main module.
/// @return `true` if the execution is successful or `false` if an exception is raised.
PK_API bool py_exec(const char* source,
                    const char* filename,
                    enum py_CompileMode mode,
                    py_Ref module) PY_RAISE PY_RETURN;
/// Evaluate a source string. Equivalent to `py_exec(source, "<string>", EVAL_MODE, module)`.
PK_API bool py_eval(const char* source, py_Ref module) PY_RAISE PY_RETURN;
/// Run a source string with smart interpretation.
/// Example:
/// `py_newstr(py_r0(), "abc");`
/// `py_newint(py_r1(), 123);`
/// `py_smartexec("print(_0, _1)", NULL, py_r0(), py_r1());`
/// `// "abc 123" will be printed`.
PK_API bool py_smartexec(const char* source, py_Ref module, ...) PY_RAISE PY_RETURN;
/// Evaluate a source string with smart interpretation.
/// Example:
/// `py_newstr(py_r0(), "abc");`
/// `py_smarteval("len(_)", NULL, py_r0());`
/// `int res = py_toint(py_retval());`
/// `// res will be 3`.
PK_API bool py_smarteval(const char* source, py_Ref module, ...) PY_RAISE PY_RETURN;

/************* Value Creation *************/

/// Create an `int` object.
PK_API void py_newint(py_OutRef, py_i64);
/// Create a trivial value object.
PK_API void py_newtrivial(py_OutRef out, py_Type type, void* data, int size);
/// Create a `float` object.
PK_API void py_newfloat(py_OutRef, py_f64);
/// Create a `bool` object.
PK_API void py_newbool(py_OutRef, bool);
/// Create a `str` object from a null-terminated string (utf-8).
PK_API void py_newstr(py_OutRef, const char*);
/// Create a `str` object with `n` UNINITIALIZED bytes plus `'\0'`.
PK_API char* py_newstrn(py_OutRef, int);
/// Create a `str` object from a `c11_sv`.
PK_API void py_newstrv(py_OutRef, c11_sv);
/// Create a formatted `str` object.
PK_API void py_newfstr(py_OutRef, const char*, ...);
/// Create a `bytes` object with `n` UNINITIALIZED bytes.
PK_API unsigned char* py_newbytes(py_OutRef, int n);
/// Create a `None` object.
PK_API void py_newnone(py_OutRef);
/// Create a `NotImplemented` object.
PK_API void py_newnotimplemented(py_OutRef);
/// Create a `...` object.
PK_API void py_newellipsis(py_OutRef);
/// Create a `nil` object. `nil` is an invalid representation of an object.
/// Don't use it unless you know what you are doing.
PK_API void py_newnil(py_OutRef);
/// Create a `nativefunc` object.
PK_API void py_newnativefunc(py_OutRef, py_CFunction);
/// Create a `function` object.
PK_API py_Name py_newfunction(py_OutRef out,
                              const char* sig,
                              py_CFunction f,
                              const char* docstring,
                              int slots);
/// Create a `boundmethod` object.
PK_API void py_newboundmethod(py_OutRef out, py_Ref self, py_Ref func);
/// Create a new object.
/// @param out output reference.
/// @param type type of the object.
/// @param slots number of slots. Use `-1` to create a `__dict__`.
/// @param udsize size of your userdata.
/// @return pointer to the userdata.
PK_API void* py_newobject(py_OutRef out, py_Type type, int slots, int udsize);

/************* Name Conversion *************/

/// Convert a null-terminated string to a name.
PK_API py_Name py_name(const char*);
/// Convert a name to a null-terminated string.
PK_API const char* py_name2str(py_Name);
/// Convert a name to a python `str` object with cache.
PK_API py_GlobalRef py_name2ref(py_Name);
/// Convert a `c11_sv` to a name.
PK_API py_Name py_namev(c11_sv);
/// Convert a name to a `c11_sv`.
PK_API c11_sv py_name2sv(py_Name);

/************* Bindings *************/

/// Bind a function to the object via "decl-based" style.
/// @param obj the target object.
/// @param sig signature of the function. e.g. `add(x, y)`.
/// @param f function to bind.
PK_API void py_bind(py_Ref obj, const char* sig, py_CFunction f);
/// Bind a method to type via "argc-based" style.
/// @param type the target type.
/// @param name name of the method.
/// @param f function to bind.
PK_API void py_bindmethod(py_Type type, const char* name, py_CFunction f);
/// Bind a static method to type via "argc-based" style.
/// @param type the target type.
/// @param name name of the method.
/// @param f function to bind.
PK_API void py_bindstaticmethod(py_Type type, const char* name, py_CFunction f);
/// Bind a function to the object via "argc-based" style.
/// @param obj the target object.
/// @param name name of the function.
/// @param f function to bind.
PK_API void py_bindfunc(py_Ref obj, const char* name, py_CFunction f);
/// Bind a property to type.
/// @param type the target type.
/// @param name name of the property.
/// @param getter getter function.
/// @param setter setter function. Use `NULL` if not needed.
PK_API void
    py_bindproperty(py_Type type, const char* name, py_CFunction getter, py_CFunction setter);
/// Bind a magic method to type.
PK_API void py_bindmagic(py_Type type, py_Name name, py_CFunction f);
/// Bind a compile-time function via "decl-based" style.
PK_API void py_macrobind(const char* sig, py_CFunction f);
/// Get a compile-time function by name.
PK_API py_ItemRef py_macroget(py_Name name);

/************* Value Cast *************/

/// Convert an `int` object in python to `int64_t`.
PK_API py_i64 py_toint(py_Ref);
/// Get the address of the trivial value object (16 bytes).
PK_API void* py_totrivial(py_Ref);
/// Convert a `float` object in python to `double`.
PK_API py_f64 py_tofloat(py_Ref);
/// Cast a `int` or `float` object in python to `double`.
/// If successful, return true and set the value to `out`.
/// Otherwise, return false and raise `TypeError`.
PK_API bool py_castfloat(py_Ref, py_f64* out) PY_RAISE;
/// 32-bit version of `py_castfloat`.
PK_API bool py_castfloat32(py_Ref, float* out) PY_RAISE;
/// Cast a `int` object in python to `int64_t`.
PK_API bool py_castint(py_Ref, py_i64* out) PY_RAISE;
/// Convert a `bool` object in python to `bool`.
PK_API bool py_tobool(py_Ref);
/// Convert a `type` object in python to `py_Type`.
PK_API py_Type py_totype(py_Ref);
/// Convert a user-defined object to its userdata.
PK_API void* py_touserdata(py_Ref);
/// Convert a `str` object in python to null-terminated string.
PK_API const char* py_tostr(py_Ref);
/// Convert a `str` object in python to char array.
PK_API const char* py_tostrn(py_Ref, int* size);
/// Convert a `str` object in python to `c11_sv`.
PK_API c11_sv py_tosv(py_Ref);
/// Convert a `bytes` object in python to char array.
PK_API unsigned char* py_tobytes(py_Ref, int* size);
/// Resize a `bytes` object. It can only be resized down.
PK_API void py_bytes_resize(py_Ref, int size);

/************* Type System *************/

/// Create a new type.
/// @param name name of the type.
/// @param base base type.
/// @param module module where the type is defined. Use `NULL` for built-in types.
/// @param dtor destructor function. Use `NULL` if not needed.
PK_API py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, py_Dtor dtor);
/// Check if the object is exactly the given type.
PK_API bool py_istype(py_Ref, py_Type);
/// Get the type of the object.
PK_API py_Type py_typeof(py_Ref self);
/// Check if the object is an instance of the given type.
PK_API bool py_isinstance(py_Ref obj, py_Type type);
/// Check if the derived type is a subclass of the base type.
PK_API bool py_issubclass(py_Type derived, py_Type base);
/// Get type by module and name. e.g. `py_gettype("time", py_name("struct_time"))`.
/// Return `0` if not found.
PK_API py_Type py_gettype(const char* module, py_Name name);
/// Check if the object is an instance of the given type exactly.
/// Raise `TypeError` if the check fails.
PK_API bool py_checktype(py_Ref self, py_Type type) PY_RAISE;
/// Check if the object is an instance of the given type or its subclass.
/// Raise `TypeError` if the check fails.
PK_API bool py_checkinstance(py_Ref self, py_Type type) PY_RAISE;
/// Get the magic method from the given type only.
/// Return `nil` if not found.
PK_API PK_DEPRECATED py_GlobalRef py_tpgetmagic(py_Type type, py_Name name);
/// Search the magic method from the given type to the base type.
/// Return `NULL` if not found.
PK_API py_GlobalRef py_tpfindmagic(py_Type, py_Name name);
/// Search the name from the given type to the base type.
/// Return `NULL` if not found.
PK_API py_ItemRef py_tpfindname(py_Type, py_Name name);
/// Get the base type of the given type.
PK_API py_Type py_tpbase(py_Type type);
/// Get the type object of the given type.
PK_API py_GlobalRef py_tpobject(py_Type type);
/// Get the type name.
PK_API const char* py_tpname(py_Type type);
/// Disable the type for subclassing.
PK_API void py_tpsetfinal(py_Type type);
/// Set attribute hooks for the given type.
PK_API void py_tphookattributes(py_Type type,
                                bool (*getattribute)(py_Ref self, py_Name name) PY_RAISE PY_RETURN,
                                bool (*setattribute)(py_Ref self, py_Name name, py_Ref val)
                                    PY_RAISE PY_RETURN,
                                bool (*delattribute)(py_Ref self, py_Name name) PY_RAISE,
                                bool (*getunboundmethod)(py_Ref self, py_Name name) PY_RETURN);

#define py_isint(self) py_istype(self, tp_int)
#define py_isfloat(self) py_istype(self, tp_float)
#define py_isbool(self) py_istype(self, tp_bool)
#define py_isstr(self) py_istype(self, tp_str)
#define py_islist(self) py_istype(self, tp_list)
#define py_istuple(self) py_istype(self, tp_tuple)
#define py_isdict(self) py_istype(self, tp_dict)
#define py_isnil(self) py_istype(self, 0)
#define py_isnone(self) py_istype(self, tp_NoneType)

#define py_checkint(self) py_checktype(self, tp_int)
#define py_checkfloat(self) py_checktype(self, tp_float)
#define py_checkbool(self) py_checktype(self, tp_bool)
#define py_checkstr(self) py_checktype(self, tp_str)

/************* Inspection *************/

/// Get the current `function` object on the stack.
/// Return `NULL` if not available.
/// NOTE: This function should be placed at the beginning of your decl-based bindings.
PK_API py_StackRef py_inspect_currentfunction();
/// Get the current `module` object where the code is executed.
/// Return `NULL` if not available.
PK_API py_GlobalRef py_inspect_currentmodule();
/// Get the current frame object.
/// Return `NULL` if not available.
PK_API py_Frame* py_inspect_currentframe();
/// Python equivalent to `globals()`.
PK_API void py_newglobals(py_OutRef);
/// Python equivalent to `locals()`.
PK_API void py_newlocals(py_OutRef);

/************* Dict & Slots *************/

/// Get the i-th register.
/// All registers are located in a contiguous memory.
PK_API py_GlobalRef py_getreg(int i);
/// Set the i-th register.
PK_API void py_setreg(int i, py_Ref val);
/// Get the last return value.
/// Please note that `py_retval()` cannot be used as input argument.
PK_API py_GlobalRef py_retval();

#define py_r0() py_getreg(0)
#define py_r1() py_getreg(1)
#define py_r2() py_getreg(2)
#define py_r3() py_getreg(3)
#define py_r4() py_getreg(4)
#define py_r5() py_getreg(5)
#define py_r6() py_getreg(6)
#define py_r7() py_getreg(7)

/// Get an item from the object's `__dict__`.
/// Return `NULL` if not found.
PK_API py_ItemRef py_getdict(py_Ref self, py_Name name);
/// Set an item to the object's `__dict__`.
PK_API void py_setdict(py_Ref self, py_Name name, py_Ref val);
/// Delete an item from the object's `__dict__`.
/// Return `true` if the deletion is successful.
PK_API bool py_deldict(py_Ref self, py_Name name);
/// Prepare an insertion to the object's `__dict__`.
PK_API py_ItemRef py_emplacedict(py_Ref self, py_Name name);
/// Apply a function to all items in the object's `__dict__`.
/// Return `true` if the function is successful for all items.
/// NOTE: Be careful if `f` modifies the object's `__dict__`.
PK_API bool
    py_applydict(py_Ref self, bool (*f)(py_Name name, py_Ref val, void* ctx), void* ctx) PY_RAISE;
/// Clear the object's `__dict__`. This function is dangerous.
PK_API void py_cleardict(py_Ref self);
/// Get the i-th slot of the object.
/// The object must have slots and `i` must be in valid range.
PK_API py_ObjectRef py_getslot(py_Ref self, int i);
/// Set the i-th slot of the object.
PK_API void py_setslot(py_Ref self, int i, py_Ref val);
/// Get variable in the `builtins` module.
PK_API py_ItemRef py_getbuiltin(py_Name name);
/// Get variable in the `__main__` module.
PK_API py_ItemRef py_getglobal(py_Name name);
/// Set variable in the `__main__` module.
PK_API void py_setglobal(py_Name name, py_Ref val);

/************* Stack Ops *************/

/// Get the i-th object from the top of the stack.
/// `i` should be negative, e.g. (-1) means TOS.
PK_API py_StackRef py_peek(int i);
/// Push the object to the stack.
PK_API void py_push(py_Ref src);
/// Push a `nil` object to the stack.
PK_API void py_pushnil();
/// Push a `None` object to the stack.
PK_API void py_pushnone();
/// Push a `py_Name` to the stack. This is used for keyword arguments.
PK_API void py_pushname(py_Name name);
/// Pop an object from the stack.
PK_API void py_pop();
/// Shrink the stack by n.
PK_API void py_shrink(int n);
/// Get a temporary variable from the stack.
PK_API py_StackRef py_pushtmp();
/// Get the unbound method of the object.
/// Assume the object is located at the top of the stack.
/// If return true:  `[self] -> [unbound, self]`.
/// If return false: `[self] -> [self]` (no change).
PK_API bool py_pushmethod(py_Name name);
/// Evaluate an expression and push the result to the stack.
/// This function is used for testing.
PK_API bool py_pusheval(const char* expr, py_GlobalRef module) PY_RAISE;
/// Call a callable object via pocketpy's calling convention.
/// You need to prepare the stack using the following format:
/// `callable, self/nil, arg1, arg2, ..., k1, v1, k2, v2, ...`.
/// `argc` is the number of positional arguments excluding `self`.
/// `kwargc` is the number of keyword arguments.
/// The result will be set to `py_retval()`.
/// The stack size will be reduced by `2 + argc + kwargc * 2`.
PK_API bool py_vectorcall(uint16_t argc, uint16_t kwargc) PY_RAISE PY_RETURN;
/// Call a function.
/// It prepares the stack and then performs a `vectorcall(argc, 0, false)`.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged if successful.
PK_API bool py_call(py_Ref f, int argc, py_Ref argv) PY_RAISE PY_RETURN;
/// Call a type to create a new instance.
PK_API bool py_tpcall(py_Type type, int argc, py_Ref argv) PY_RAISE PY_RETURN;

#ifndef NDEBUG
/// Call a `py_CFunction` in a safe way.
/// This function does extra checks to help you debug `py_CFunction`.
PK_API bool py_callcfunc(py_CFunction f, int argc, py_Ref argv) PY_RAISE PY_RETURN;
#else
#define py_callcfunc(f, argc, argv) (f((argc), (argv)))
#endif

#define PY_CHECK_ARGC(n)                                                                           \
    if(argc != n) return TypeError("expected %d arguments, got %d", n, argc)

#define PY_CHECK_ARG_TYPE(i, type)                                                                 \
    if(!py_checktype(py_arg(i), type)) return false

#define py_offset(p, i) ((p) + (i))
#define py_arg(i) (&argv[i])
#define py_assign(dst, src) *(dst) = *(src)

/// Perform a binary operation.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
PK_API bool py_binaryop(py_Ref lhs, py_Ref rhs, py_Name op, py_Name rop) PY_RAISE PY_RETURN;

/************* Python Ops *************/

/// lhs + rhs
PK_API bool py_binaryadd(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs - rhs
PK_API bool py_binarysub(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs * rhs
PK_API bool py_binarymul(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs / rhs
PK_API bool py_binarytruediv(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs // rhs
PK_API bool py_binaryfloordiv(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs % rhs
PK_API bool py_binarymod(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs ** rhs
PK_API bool py_binarypow(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs << rhs
PK_API bool py_binarylshift(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs >> rhs
PK_API bool py_binaryrshift(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs & rhs
PK_API bool py_binaryand(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs | rhs
PK_API bool py_binaryor(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs ^ rhs
PK_API bool py_binaryxor(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs @ rhs
PK_API bool py_binarymatmul(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs == rhs
PK_API bool py_eq(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs != rhs
PK_API bool py_ne(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs < rhs
PK_API bool py_lt(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs <= rhs
PK_API bool py_le(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs > rhs
PK_API bool py_gt(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;
/// lhs >= rhs
PK_API bool py_ge(py_Ref lhs, py_Ref rhs) PY_RAISE PY_RETURN;

/// Python equivalent to `lhs is rhs`.
PK_API bool py_isidentical(py_Ref, py_Ref);
/// Python equivalent to `bool(val)`.
/// 1: true, 0: false, -1: error
PK_API int py_bool(py_Ref val) PY_RAISE;
/// Compare two objects.
/// 1: lhs == rhs, 0: lhs != rhs, -1: error
PK_API int py_equal(py_Ref lhs, py_Ref rhs) PY_RAISE;
/// Compare two objects.
/// 1: lhs < rhs, 0: lhs >= rhs, -1: error
PK_API int py_less(py_Ref lhs, py_Ref rhs) PY_RAISE;
/// Python equivalent to `callable(val)`.
PK_API bool py_callable(py_Ref val);
/// Get the hash value of the object.
PK_API bool py_hash(py_Ref, py_i64* out) PY_RAISE;
/// Get the iterator of the object.
PK_API bool py_iter(py_Ref) PY_RAISE PY_RETURN;
/// Get the next element from the iterator.
/// 1: success, 0: StopIteration, -1: error
PK_API int py_next(py_Ref) PY_RAISE PY_RETURN;
/// Python equivalent to `str(val)`.
PK_API bool py_str(py_Ref val) PY_RAISE PY_RETURN;
/// Python equivalent to `repr(val)`.
PK_API bool py_repr(py_Ref val) PY_RAISE PY_RETURN;
/// Python equivalent to `len(val)`.
PK_API bool py_len(py_Ref val) PY_RAISE PY_RETURN;

/// Python equivalent to `getattr(self, name)`.
PK_API bool py_getattr(py_Ref self, py_Name name) PY_RAISE PY_RETURN;
/// Python equivalent to `setattr(self, name, val)`.
PK_API bool py_setattr(py_Ref self, py_Name name, py_Ref val) PY_RAISE;
/// Python equivalent to `delattr(self, name)`.
PK_API bool py_delattr(py_Ref self, py_Name name) PY_RAISE;
/// Python equivalent to `self[key]`.
PK_API bool py_getitem(py_Ref self, py_Ref key) PY_RAISE PY_RETURN;
/// Python equivalent to `self[key] = val`.
PK_API bool py_setitem(py_Ref self, py_Ref key, py_Ref val) PY_RAISE;
/// Python equivalent to `del self[key]`.
PK_API bool py_delitem(py_Ref self, py_Ref key) PY_RAISE;

/************* Module System *************/

/// Get a module by path.
PK_API py_GlobalRef py_getmodule(const char* path);
/// Create a new module.
PK_API py_GlobalRef py_newmodule(const char* path);
/// Reload an existing module.
PK_API bool py_importlib_reload(py_Ref module) PY_RAISE PY_RETURN;
/// Import a module.
/// The result will be set to `py_retval()`.
/// -1: error, 0: not found, 1: success
PK_API int py_import(const char* path) PY_RAISE PY_RETURN;

/************* PyException *************/

/// Check if there is an unhandled exception.
PK_API bool py_checkexc();
/// Check if the unhandled exception is an instance of the given type.
/// If match, the exception will be stored in `py_retval()`.
PK_API bool py_matchexc(py_Type type) PY_RETURN;
/// Clear the unhandled exception.
/// @param p0 the unwinding point. Use `NULL` if not needed.
PK_API void py_clearexc(py_StackRef p0);
/// Print the unhandled exception.
PK_API void py_printexc();
/// Format the unhandled exception and return a null-terminated string.
/// The returned string should be freed by the caller.
PK_API char* py_formatexc();
/// Raise an exception by type and message. Always return false.
PK_API bool py_exception(py_Type type, const char* fmt, ...) PY_RAISE;
/// Raise an exception object. Always return false.
PK_API bool py_raise(py_Ref) PY_RAISE;

#define NameError(n) py_exception(tp_NameError, "name '%n' is not defined", (n))
#define TypeError(...) py_exception(tp_TypeError, __VA_ARGS__)
#define RuntimeError(...) py_exception(tp_RuntimeError, __VA_ARGS__)
#define TimeoutError(...) py_exception(tp_TimeoutError, __VA_ARGS__)
#define OSError(...) py_exception(tp_OSError, __VA_ARGS__)
#define ValueError(...) py_exception(tp_ValueError, __VA_ARGS__)
#define IndexError(...) py_exception(tp_IndexError, __VA_ARGS__)
#define ImportError(...) py_exception(tp_ImportError, __VA_ARGS__)
#define ZeroDivisionError(...) py_exception(tp_ZeroDivisionError, __VA_ARGS__)
#define AttributeError(self, n)                                                                    \
    py_exception(tp_AttributeError, "'%t' object has no attribute '%n'", (self)->type, (n))
#define UnboundLocalError(n)                                                                       \
    py_exception(tp_UnboundLocalError,                                                             \
                 "cannot access local variable '%n' where it is not associated with a value",      \
                 (n))

PK_API bool KeyError(py_Ref key) PY_RAISE;
PK_API bool StopIteration() PY_RAISE;

/************* Debugger *************/

#if PK_ENABLE_OS
PK_API void py_debugger_waitforattach(const char* hostname, unsigned short port);
PK_API int py_debugger_status();
PK_API void py_debugger_exceptionbreakpoint(py_Ref exc);
PK_API void py_debugger_exit(int code);
#else
#define py_debugger_waitforattach(hostname, port)
#define py_debugger_status() 0
#define py_debugger_exceptionbreakpoint(exc)
#define py_debugger_exit(code)
#endif

/************* PyTuple *************/

/// Create a `tuple` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
PK_API py_ObjectRef py_newtuple(py_OutRef, int n);
PK_API py_ObjectRef py_tuple_data(py_Ref self);
PK_API py_ObjectRef py_tuple_getitem(py_Ref self, int i);
PK_API void py_tuple_setitem(py_Ref self, int i, py_Ref val);
PK_API int py_tuple_len(py_Ref self);

/************* PyList *************/

/// Create an empty `list`.
PK_API void py_newlist(py_OutRef);
/// Create a `list` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
PK_API void py_newlistn(py_OutRef, int n);
PK_API py_ItemRef py_list_data(py_Ref self);
PK_API py_ItemRef py_list_getitem(py_Ref self, int i);
PK_API void py_list_setitem(py_Ref self, int i, py_Ref val);
PK_API void py_list_delitem(py_Ref self, int i);
PK_API int py_list_len(py_Ref self);
PK_API void py_list_swap(py_Ref self, int i, int j);
PK_API void py_list_append(py_Ref self, py_Ref val);
PK_API py_ItemRef py_list_emplace(py_Ref self);
PK_API void py_list_clear(py_Ref self);
PK_API void py_list_insert(py_Ref self, int i, py_Ref val);

/************* PyDict *************/

/// Create an empty `dict`.
PK_API void py_newdict(py_OutRef);
/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem(py_Ref self, py_Ref key) PY_RAISE PY_RETURN;
/// true: success, false: error
PK_API bool py_dict_setitem(py_Ref self, py_Ref key, py_Ref val) PY_RAISE;
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem(py_Ref self, py_Ref key) PY_RAISE;

/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem_by_str(py_Ref self, const char* key) PY_RAISE PY_RETURN;
/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem_by_int(py_Ref self, py_i64 key) PY_RAISE PY_RETURN;
/// true: success, false: error
PK_API bool py_dict_setitem_by_str(py_Ref self, const char* key, py_Ref val) PY_RAISE;
/// true: success, false: error
PK_API bool py_dict_setitem_by_int(py_Ref self, py_i64 key, py_Ref val) PY_RAISE;
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem_by_str(py_Ref self, const char* key) PY_RAISE;
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem_by_int(py_Ref self, py_i64 key) PY_RAISE;

/// true: success, false: error
PK_API bool
    py_dict_apply(py_Ref self, bool (*f)(py_Ref key, py_Ref val, void* ctx), void* ctx) PY_RAISE;
/// noexcept
PK_API int py_dict_len(py_Ref self);

/************* PySlice *************/

/// Create an UNINITIALIZED `slice` object.
/// You should use `py_setslot()` to set `start`, `stop`, and `step`.
PK_API py_ObjectRef py_newslice(py_OutRef);
/// Create a `slice` object from 3 integers.
PK_API void py_newsliceint(py_OutRef out, py_i64 start, py_i64 stop, py_i64 step);

/************* random module *************/
PK_API void py_newRandom(py_OutRef out);
PK_API void py_Random_seed(py_Ref self, py_i64 seed);
PK_API py_f64 py_Random_random(py_Ref self);
PK_API py_f64 py_Random_uniform(py_Ref self, py_f64 a, py_f64 b);
PK_API py_i64 py_Random_randint(py_Ref self, py_i64 a, py_i64 b);

/************* array2d module *************/
PK_API void py_newarray2d(py_OutRef out, int width, int height);
PK_API int py_array2d_getwidth(py_Ref self);
PK_API int py_array2d_getheight(py_Ref self);
PK_API py_ObjectRef py_array2d_getitem(py_Ref self, int x, int y);
PK_API void py_array2d_setitem(py_Ref self, int x, int y, py_Ref val);

/************* vmath module *************/
PK_API void py_newvec2(py_OutRef out, c11_vec2);
PK_API void py_newvec3(py_OutRef out, c11_vec3);
PK_API void py_newvec2i(py_OutRef out, c11_vec2i);
PK_API void py_newvec3i(py_OutRef out, c11_vec3i);
PK_API void py_newcolor32(py_OutRef out, c11_color32);
PK_API c11_mat3x3* py_newmat3x3(py_OutRef out);
PK_API c11_vec2 py_tovec2(py_Ref self);
PK_API c11_vec3 py_tovec3(py_Ref self);
PK_API c11_vec2i py_tovec2i(py_Ref self);
PK_API c11_vec3i py_tovec3i(py_Ref self);
PK_API c11_mat3x3* py_tomat3x3(py_Ref self);
PK_API c11_color32 py_tocolor32(py_Ref self);

/************* json module *************/
/// Python equivalent to `json.dumps(val)`.
PK_API bool py_json_dumps(py_Ref val, int indent) PY_RAISE PY_RETURN;
/// Python equivalent to `json.loads(val)`.
PK_API bool py_json_loads(const char* source) PY_RAISE PY_RETURN;

/************* pickle module *************/
/// Python equivalent to `pickle.dumps(val)`.
PK_API bool py_pickle_dumps(py_Ref val) PY_RAISE PY_RETURN;
/// Python equivalent to `pickle.loads(val)`.
PK_API bool py_pickle_loads(const unsigned char* data, int size) PY_RAISE PY_RETURN;

/************* pkpy module *************/
/// Begin the watchdog with `timeout` in milliseconds.
/// `PK_ENABLE_WATCHDOG` must be defined to `1` to use this feature.
/// You need to call `py_watchdog_end()` later.
/// If `timeout` is reached, `TimeoutError` will be raised.
PK_API void py_watchdog_begin(py_i64 timeout);
/// Reset the watchdog.
PK_API void py_watchdog_end();

PK_API void py_profiler_begin();
PK_API void py_profiler_end();
PK_API void py_profiler_reset();
PK_API char* py_profiler_report();

/************* Others *************/

/// An utility function to read a line from stdin for REPL.
PK_API int py_replinput(char* buf, int max_size);

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

enum py_PredefinedType {
    tp_nil = 0,
    tp_object = 1,
    tp_type,  // py_Type
    tp_int,
    tp_float,
    tp_bool,
    tp_str,
    tp_str_iterator,
    tp_list,            // c11_vector
    tp_tuple,           // N slots
    tp_list_iterator,   // 1 slot
    tp_tuple_iterator,  // 1 slot
    tp_slice,           // 3 slots (start, stop, step)
    tp_range,
    tp_range_iterator,
    tp_module,
    tp_function,
    tp_nativefunc,
    tp_boundmethod,  // 2 slots (self, func)
    tp_super,        // 1 slot + py_Type
    tp_BaseException,
    tp_Exception,
    tp_bytes,
    tp_namedict,
    tp_locals,
    tp_code,
    tp_dict,
    tp_dict_iterator,  // 1 slot
    tp_property,       // 2 slots (getter + setter)
    tp_star_wrapper,   // 1 slot + int level
    tp_staticmethod,   // 1 slot
    tp_classmethod,    // 1 slot
    tp_NoneType,
    tp_NotImplementedType,
    tp_ellipsis,
    tp_generator,
    /* builtin exceptions */
    tp_SystemExit,
    tp_KeyboardInterrupt,
    tp_StopIteration,
    tp_SyntaxError,
    tp_RecursionError,
    tp_OSError,
    tp_NotImplementedError,
    tp_TypeError,
    tp_IndexError,
    tp_ValueError,
    tp_RuntimeError,
    tp_TimeoutError,
    tp_ZeroDivisionError,
    tp_NameError,
    tp_UnboundLocalError,
    tp_AttributeError,
    tp_ImportError,
    tp_AssertionError,
    tp_KeyError,
    /* vmath */
    tp_vec2,
    tp_vec3,
    tp_vec2i,
    tp_vec3i,
    tp_mat3x3,
    tp_color32,
    /* array2d */
    tp_array2d_like,
    tp_array2d_like_iterator,
    tp_array2d,
    tp_array2d_view,
    tp_chunked_array2d,
};

#ifdef __cplusplus
}
#endif
