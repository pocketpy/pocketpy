### py_initialize 
```c
/// Initialize pocketpy and the default VM.
PK_API void py_initialize();
```

### py_finalize 
```c
/// Finalize pocketpy and free all VMs. This opearation is irreversible.
/// After this call, you cannot use any function from this header anymore.
PK_API void py_finalize();
```

### py_currentvm 
```c
/// Get the current VM index.
PK_API int py_currentvm();
```

### py_switchvm 
```c
/// Switch to a VM.
/// @param index index of the VM ranging from 0 to 16 (exclusive). `0` is the default VM.
PK_API void py_switchvm(int index);
```

### py_resetvm 
```c
/// Reset the current VM.
PK_API void py_resetvm();
```

### py_resetallvm 
```c
/// Reset All VMs.
PK_API void py_resetallvm();
```

### py_getvmctx 
```c
/// Get the current VM context. This is used for user-defined data.
PK_API void* py_getvmctx();
```

### py_setvmctx 
```c
/// Set the current VM context. This is used for user-defined data.
PK_API void py_setvmctx(void* ctx);
```

### py_callbacks 
```c
/// Setup the callbacks for the current VM.
PK_API py_Callbacks* py_callbacks();
```

### py_appcallbacks 
```c
/// Setup the application callbacks
PK_API py_AppCallbacks* py_appcallbacks();
```

### py_sys_setargv 
```c
/// Set `sys.argv`. Used for storing command-line arguments.
PK_API void py_sys_setargv(int argc, char** argv);
```

### py_sys_settrace 
```c
/// Set the trace function for the current VM.
PK_API void py_sys_settrace(py_TraceFunc func, bool reset);
```

### py_gc_collect 
```c
/// Invoke the garbage collector.
PK_API int py_gc_collect();
```

### py_malloc 
```c
/// Wrapper for `PK_MALLOC(size)`.
PK_API void* py_malloc(size_t size);
```

### py_realloc 
```c
/// Wrapper for `PK_REALLOC(ptr, size)`.
PK_API void* py_realloc(void* ptr, size_t size);
```

### py_free 
```c
/// Wrapper for `PK_FREE(ptr)`.
PK_API void py_free(void* ptr);
```

### py_True 
```c
/// A shorthand for `True`.
PK_API py_GlobalRef py_True();
```

### py_False 
```c
/// A shorthand for `False`.
PK_API py_GlobalRef py_False();
```

### py_None 
```c
/// A shorthand for `None`.
PK_API py_GlobalRef py_None();
```

### py_NIL 
```c
/// A shorthand for `nil`. `nil` is not a valid python object.
PK_API py_GlobalRef py_NIL();
```

### py_Frame_newglobals 
```c
/// Python equivalent to `globals()` with respect to the given frame.
PK_API void py_Frame_newglobals(py_Frame* frame, py_OutRef out);
```

### py_Frame_newlocals 
```c
/// Python equivalent to `locals()` with respect to the given frame.
PK_API void py_Frame_newlocals(py_Frame* frame, py_OutRef out);
```

### py_Frame_function 
```c
/// Get the function object of the frame.
/// Returns `NULL` if not available.
PK_API py_StackRef py_Frame_function(py_Frame* frame);
```

### py_compile [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Compile a source string into a code object.
/// Use python's `exec()` or `eval()` to execute it.
PK_API bool py_compile(const char* source,
                       const char* filename,
                       enum py_CompileMode mode,
                       bool is_dynamic);
```

### py_compilefile [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Compile a `.py` file into a `.pyc` file.
PK_API bool py_compilefile(const char* src_path,
                           const char* dst_path);
```

### py_execo [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Run a compiled code object.
PK_API bool py_execo(const void* data, int size, const char* filename, py_Ref module);
```

### py_exec [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Run a source string.
/// @param source source string.
/// @param filename filename (for error messages).
/// @param mode compile mode. Use `EXEC_MODE` for statements `EVAL_MODE` for expressions.
/// @param module target module. Use NULL for the main module.
/// @return `true` if the execution is successful or `false` if an exception is raised.
PK_API bool py_exec(const char* source,
                    const char* filename,
                    enum py_CompileMode mode,
                    py_Ref module);
```

### py_eval [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Evaluate a source string. Equivalent to `py_exec(source, "<string>", EVAL_MODE, module)`.
PK_API bool py_eval(const char* source, py_Ref module);
```

### py_smartexec [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Run a source string with smart interpretation.
/// Example:
/// `py_newstr(py_r0(), "abc");`
/// `py_newint(py_r1(), 123);`
/// `py_smartexec("print(_0, _1)", NULL, py_r0(), py_r1());`
/// `// "abc 123" will be printed`.
PK_API bool py_smartexec(const char* source, py_Ref module, ...);
```

### py_smarteval [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Evaluate a source string with smart interpretation.
/// Example:
/// `py_newstr(py_r0(), "abc");`
/// `py_smarteval("len(_)", NULL, py_r0());`
/// `int res = py_toint(py_retval());`
/// `// res will be 3`.
PK_API bool py_smarteval(const char* source, py_Ref module, ...);
```

### py_newint 
```c
/// Create an `int` object.
PK_API void py_newint(py_OutRef, py_i64);
```

### py_newtrivial 
```c
/// Create a trivial value object.
PK_API void py_newtrivial(py_OutRef out, py_Type type, void* data, int size);
```

### py_newfloat 
```c
/// Create a `float` object.
PK_API void py_newfloat(py_OutRef, py_f64);
```

### py_newbool 
```c
/// Create a `bool` object.
PK_API void py_newbool(py_OutRef, bool);
```

### py_newstr 
```c
/// Create a `str` object from a null-terminated string (utf-8).
PK_API void py_newstr(py_OutRef, const char*);
```

### py_newstrn 
```c
/// Create a `str` object with `n` UNINITIALIZED bytes plus `'\0'`.
PK_API char* py_newstrn(py_OutRef, int);
```

### py_newstrv 
```c
/// Create a `str` object from a `c11_sv`.
PK_API void py_newstrv(py_OutRef, c11_sv);
```

### py_newfstr 
```c
/// Create a formatted `str` object.
PK_API void py_newfstr(py_OutRef, const char*, ...);
```

### py_newnone 
```c
/// Create a `None` object.
PK_API void py_newnone(py_OutRef);
```

### py_newnotimplemented 
```c
/// Create a `NotImplemented` object.
PK_API void py_newnotimplemented(py_OutRef);
```

### py_newellipsis 
```c
/// Create a `...` object.
PK_API void py_newellipsis(py_OutRef);
```

### py_newnil 
```c
/// Create a `nil` object. `nil` is an invalid representation of an object.
/// Don't use it unless you know what you are doing.
PK_API void py_newnil(py_OutRef);
```

### py_newnativefunc 
```c
/// Create a `nativefunc` object.
PK_API void py_newnativefunc(py_OutRef, py_CFunction);
```

### py_newfunction 
```c
/// Create a `function` object.
PK_API py_Name py_newfunction(py_OutRef out,
                              const char* sig,
                              py_CFunction f,
                              const char* docstring,
                              int slots);
```

### py_newboundmethod 
```c
/// Create a `boundmethod` object.
PK_API void py_newboundmethod(py_OutRef out, py_Ref self, py_Ref func);
```

### py_newobject 
```c
/// Create a new object.
/// @param out output reference.
/// @param type type of the object.
/// @param slots number of slots. Use `-1` to create a `__dict__`.
/// @param udsize size of your userdata.
/// @return pointer to the userdata.
PK_API void* py_newobject(py_OutRef out, py_Type type, int slots, int udsize);
```

### py_name 
```c
/// Convert a null-terminated string to a name.
PK_API py_Name py_name(const char*);
```

### py_name2ref 
```c
/// Convert a name to a python `str` object with cache.
PK_API py_GlobalRef py_name2ref(py_Name);
```

### py_namev 
```c
/// Convert a `c11_sv` to a name.
PK_API py_Name py_namev(c11_sv);
```

### py_name2sv 
```c
/// Convert a name to a `c11_sv`.
PK_API c11_sv py_name2sv(py_Name);
```

### py_bind 
```c
/// Bind a function to the object via "decl-based" style.
/// @param obj the target object.
/// @param sig signature of the function. e.g. `add(x, y)`.
/// @param f function to bind.
PK_API void py_bind(py_Ref obj, const char* sig, py_CFunction f);
```

### py_bindmethod 
```c
/// Bind a method to type via "argc-based" style.
/// @param type the target type.
/// @param name name of the method.
/// @param f function to bind.
PK_API void py_bindmethod(py_Type type, const char* name, py_CFunction f);
```

### py_bindstaticmethod 
```c
/// Bind a static method to type via "argc-based" style.
/// @param type the target type.
/// @param name name of the method.
/// @param f function to bind.
PK_API void py_bindstaticmethod(py_Type type, const char* name, py_CFunction f);
```

### py_bindfunc 
```c
/// Bind a function to the object via "argc-based" style.
/// @param obj the target object.
/// @param name name of the function.
/// @param f function to bind.
PK_API void py_bindfunc(py_Ref obj, const char* name, py_CFunction f);
```

### py_bindproperty 
```c
/// Bind a property to type.
/// @param type the target type.
/// @param name name of the property.
/// @param getter getter function.
/// @param setter setter function. Use `NULL` if not needed.
PK_API void py_bindproperty(py_Type type, const char* name, py_CFunction getter, py_CFunction setter);
```

### py_bindmagic 
```c
/// Bind a magic method to type.
PK_API void py_bindmagic(py_Type type, py_Name name, py_CFunction f);
```

### py_toint 
```c
/// Convert an `int` object in python to `int64_t`.
PK_API py_i64 py_toint(py_Ref);
```

### py_totrivial 
```c
/// Get the address of the trivial value object (16 bytes).
PK_API void* py_totrivial(py_Ref);
```

### py_tofloat 
```c
/// Convert a `float` object in python to `double`.
PK_API py_f64 py_tofloat(py_Ref);
```

### py_castfloat [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Cast a `int` or `float` object in python to `double`.
/// If successful, return true and set the value to `out`.
/// Otherwise, return false and raise `TypeError`.
PK_API bool py_castfloat(py_Ref, py_f64* out);
```

### py_castfloat32 [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// 32-bit version of `py_castfloat`.
PK_API bool py_castfloat32(py_Ref, float* out);
```

### py_castint [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Cast a `int` object in python to `int64_t`.
PK_API bool py_castint(py_Ref, py_i64* out);
```

### py_tobool 
```c
/// Convert a `bool` object in python to `bool`.
PK_API bool py_tobool(py_Ref);
```

### py_totype 
```c
/// Convert a `type` object in python to `py_Type`.
PK_API py_Type py_totype(py_Ref);
```

### py_touserdata 
```c
/// Convert a user-defined object to its userdata.
PK_API void* py_touserdata(py_Ref);
```

### py_tosv 
```c
/// Convert a `str` object in python to `c11_sv`.
PK_API c11_sv py_tosv(py_Ref);
```

### py_bytes_resize 
```c
/// Resize a `bytes` object. It can only be resized down.
PK_API void py_bytes_resize(py_Ref, int size);
```

### py_newtype 
```c
/// Create a new type.
/// @param name name of the type.
/// @param base base type.
/// @param module module where the type is defined. Use `NULL` for built-in types.
/// @param dtor destructor function. Use `NULL` if not needed.
PK_API py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, py_Dtor dtor);
```

### py_istype 
```c
/// Check if the object is exactly the given type.
PK_API bool py_istype(py_Ref, py_Type);
```

### py_typeof 
```c
/// Get the type of the object.
PK_API py_Type py_typeof(py_Ref self);
```

### py_isinstance 
```c
/// Check if the object is an instance of the given type.
PK_API bool py_isinstance(py_Ref obj, py_Type type);
```

### py_issubclass 
```c
/// Check if the derived type is a subclass of the base type.
PK_API bool py_issubclass(py_Type derived, py_Type base);
```

### py_gettype 
```c
/// Get type by module and name. e.g. `py_gettype("time", py_name("struct_time"))`.
/// Return `0` if not found.
PK_API py_Type py_gettype(const char* module, py_Name name);
```

### py_checktype [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Check if the object is an instance of the given type exactly.
/// Raise `TypeError` if the check fails.
PK_API bool py_checktype(py_Ref self, py_Type type);
```

### py_checkinstance [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Check if the object is an instance of the given type or its subclass.
/// Raise `TypeError` if the check fails.
PK_API bool py_checkinstance(py_Ref self, py_Type type);
```

### py_tpfindmagic 
```c
/// Search the magic method from the given type to the base type.
/// Return `NULL` if not found.
PK_API py_GlobalRef py_tpfindmagic(py_Type, py_Name name);
```

### py_tpfindname 
```c
/// Search the name from the given type to the base type.
/// Return `NULL` if not found.
PK_API py_ItemRef py_tpfindname(py_Type, py_Name name);
```

### py_tpbase 
```c
/// Get the base type of the given type.
PK_API py_Type py_tpbase(py_Type type);
```

### py_tpobject 
```c
/// Get the type object of the given type.
PK_API py_GlobalRef py_tpobject(py_Type type);
```

### py_tpsetfinal 
```c
/// Disable the type for subclassing.
PK_API void py_tpsetfinal(py_Type type);
```

### py_tphookattributes 
```c
/// Set attribute hooks for the given type.
PK_API void py_tphookattributes(py_Type type,
                                bool (*getattribute)(py_Ref self, py_Name name) PY_RAISE PY_RETURN,
                                bool (*setattribute)(py_Ref self, py_Name name, py_Ref val)
                                    PY_RAISE PY_RETURN,
                                bool (*delattribute)(py_Ref self, py_Name name) PY_RAISE,
                                bool (*getunboundmethod)(py_Ref self, py_Name name) PY_RETURN);
```

### py_inspect_currentfunction 
```c
/// Get the current `Callable` object on the stack of the most recent vectorcall.
/// Return `NULL` if not available.
/// NOTE: This function should be placed at the beginning of your bindings or you will get wrong result.
PK_API py_StackRef py_inspect_currentfunction();
```

### py_inspect_currentmodule 
```c
/// Get the current `module` object where the code is executed.
/// Return `NULL` if not available.
PK_API py_GlobalRef py_inspect_currentmodule();
```

### py_inspect_currentframe 
```c
/// Get the current frame object.
/// Return `NULL` if not available.
PK_API py_Frame* py_inspect_currentframe();
```

### py_newglobals 
```c
/// Python equivalent to `globals()`.
PK_API void py_newglobals(py_OutRef);
```

### py_newlocals 
```c
/// Python equivalent to `locals()`.
PK_API void py_newlocals(py_OutRef);
```

### py_getreg 
```c
/// Get the i-th register.
/// All registers are located in a contiguous memory.
PK_API py_GlobalRef py_getreg(int i);
```

### py_setreg 
```c
/// Set the i-th register.
PK_API void py_setreg(int i, py_Ref val);
```

### py_retval 
```c
/// Get the last return value.
/// Please note that `py_retval()` cannot be used as input argument.
PK_API py_GlobalRef py_retval();
```

### py_getdict 
```c
/// Get an item from the object's `__dict__`.
/// Return `NULL` if not found.
PK_API py_ItemRef py_getdict(py_Ref self, py_Name name);
```

### py_setdict 
```c
/// Set an item to the object's `__dict__`.
PK_API void py_setdict(py_Ref self, py_Name name, py_Ref val);
```

### py_deldict 
```c
/// Delete an item from the object's `__dict__`.
/// Return `true` if the deletion is successful.
PK_API bool py_deldict(py_Ref self, py_Name name);
```

### py_emplacedict 
```c
/// Prepare an insertion to the object's `__dict__`.
PK_API py_ItemRef py_emplacedict(py_Ref self, py_Name name);
```

### py_applydict [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Apply a function to all items in the object's `__dict__`.
/// Return `true` if the function is successful for all items.
/// NOTE: Be careful if `f` modifies the object's `__dict__`.
PK_API bool py_applydict(py_Ref self, bool (*f)(py_Name name, py_Ref val, void* ctx), void* ctx);
```

### py_cleardict 
```c
/// Clear the object's `__dict__`. This function is dangerous.
PK_API void py_cleardict(py_Ref self);
```

### py_getslot 
```c
/// Get the i-th slot of the object.
/// The object must have slots and `i` must be in valid range.
PK_API py_ObjectRef py_getslot(py_Ref self, int i);
```

### py_setslot 
```c
/// Set the i-th slot of the object.
PK_API void py_setslot(py_Ref self, int i, py_Ref val);
```

### py_getbuiltin 
```c
/// Get variable in the `builtins` module.
PK_API py_ItemRef py_getbuiltin(py_Name name);
```

### py_getglobal 
```c
/// Get variable in the `__main__` module.
PK_API py_ItemRef py_getglobal(py_Name name);
```

### py_setglobal 
```c
/// Set variable in the `__main__` module.
PK_API void py_setglobal(py_Name name, py_Ref val);
```

### py_peek 
```c
/// Get the i-th object from the top of the stack.
/// `i` should be negative, e.g. (-1) means TOS.
PK_API py_StackRef py_peek(int i);
```

### py_push 
```c
/// Push the object to the stack.
PK_API void py_push(py_Ref src);
```

### py_pushnil 
```c
/// Push a `nil` object to the stack.
PK_API void py_pushnil();
```

### py_pushnone 
```c
/// Push a `None` object to the stack.
PK_API void py_pushnone();
```

### py_pushname 
```c
/// Push a `py_Name` to the stack. This is used for keyword arguments.
PK_API void py_pushname(py_Name name);
```

### py_pop 
```c
/// Pop an object from the stack.
PK_API void py_pop();
```

### py_shrink 
```c
/// Shrink the stack by n.
PK_API void py_shrink(int n);
```

### py_pushtmp 
```c
/// Get a temporary variable from the stack.
PK_API py_StackRef py_pushtmp();
```

### py_pushmethod 
```c
/// Get the unbound method of the object.
/// Assume the object is located at the top of the stack.
/// If return true:  `[self] -> [unbound, self]`.
/// If return false: `[self] -> [self]` (no change).
PK_API bool py_pushmethod(py_Name name);
```

### py_pusheval [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Evaluate an expression and push the result to the stack.
/// This function is used for testing.
PK_API bool py_pusheval(const char* expr, py_GlobalRef module);
```

### py_vectorcall [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Call a callable object via pocketpy's calling convention.
/// You need to prepare the stack using the following format:
/// `callable, self/nil, arg1, arg2, ..., k1, v1, k2, v2, ...`.
/// `argc` is the number of positional arguments excluding `self`.
/// `kwargc` is the number of keyword arguments.
/// The result will be set to `py_retval()`.
/// The stack size will be reduced by `2 + argc + kwargc * 2`.
PK_API bool py_vectorcall(uint16_t argc, uint16_t kwargc);
```

### py_call [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Call a function.
/// It prepares the stack and then performs a `vectorcall(argc, 0, false)`.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged if successful.
PK_API bool py_call(py_Ref f, int argc, py_Ref argv);
```

### py_tpcall [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Call a type to create a new instance.
PK_API bool py_tpcall(py_Type type, int argc, py_Ref argv);
```

### py_callcfunc [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Call a `py_CFunction` in a safe way.
/// This function does extra checks to help you debug `py_CFunction`.
PK_API bool py_callcfunc(py_CFunction f, int argc, py_Ref argv);
```

### py_binaryop [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Perform a binary operation.
/// The result will be set to `py_retval()`.
/// The stack remains unchanged after the operation.
PK_API bool py_binaryop(py_Ref lhs, py_Ref rhs, py_Name op, py_Name rop);
```

### py_binaryadd [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs + rhs
PK_API bool py_binaryadd(py_Ref lhs, py_Ref rhs);
```

### py_binarysub [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs - rhs
PK_API bool py_binarysub(py_Ref lhs, py_Ref rhs);
```

### py_binarymul [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs * rhs
PK_API bool py_binarymul(py_Ref lhs, py_Ref rhs);
```

### py_binarytruediv [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs / rhs
PK_API bool py_binarytruediv(py_Ref lhs, py_Ref rhs);
```

### py_binaryfloordiv [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs // rhs
PK_API bool py_binaryfloordiv(py_Ref lhs, py_Ref rhs);
```

### py_binarymod [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs % rhs
PK_API bool py_binarymod(py_Ref lhs, py_Ref rhs);
```

### py_binarypow [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs ** rhs
PK_API bool py_binarypow(py_Ref lhs, py_Ref rhs);
```

### py_binarylshift [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs << rhs
PK_API bool py_binarylshift(py_Ref lhs, py_Ref rhs);
```

### py_binaryrshift [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs >> rhs
PK_API bool py_binaryrshift(py_Ref lhs, py_Ref rhs);
```

### py_binaryand [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs & rhs
PK_API bool py_binaryand(py_Ref lhs, py_Ref rhs);
```

### py_binaryor [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs | rhs
PK_API bool py_binaryor(py_Ref lhs, py_Ref rhs);
```

### py_binaryxor [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs ^ rhs
PK_API bool py_binaryxor(py_Ref lhs, py_Ref rhs);
```

### py_binarymatmul [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs @ rhs
PK_API bool py_binarymatmul(py_Ref lhs, py_Ref rhs);
```

### py_eq [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs == rhs
PK_API bool py_eq(py_Ref lhs, py_Ref rhs);
```

### py_ne [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs != rhs
PK_API bool py_ne(py_Ref lhs, py_Ref rhs);
```

### py_lt [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs < rhs
PK_API bool py_lt(py_Ref lhs, py_Ref rhs);
```

### py_le [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs <= rhs
PK_API bool py_le(py_Ref lhs, py_Ref rhs);
```

### py_gt [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs > rhs
PK_API bool py_gt(py_Ref lhs, py_Ref rhs);
```

### py_ge [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// lhs >= rhs
PK_API bool py_ge(py_Ref lhs, py_Ref rhs);
```

### py_isidentical 
```c
/// Python equivalent to `lhs is rhs`.
PK_API bool py_isidentical(py_Ref, py_Ref);
```

### py_bool [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Python equivalent to `bool(val)`.
/// 1: true, 0: false, -1: error
PK_API int py_bool(py_Ref val);
```

### py_equal [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Compare two objects.
/// 1: lhs == rhs, 0: lhs != rhs, -1: error
PK_API int py_equal(py_Ref lhs, py_Ref rhs);
```

### py_less [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Compare two objects.
/// 1: lhs < rhs, 0: lhs >= rhs, -1: error
PK_API int py_less(py_Ref lhs, py_Ref rhs);
```

### py_callable 
```c
/// Python equivalent to `callable(val)`.
PK_API bool py_callable(py_Ref val);
```

### py_hash [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Get the hash value of the object.
PK_API bool py_hash(py_Ref, py_i64* out);
```

### py_iter [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Get the iterator of the object.
PK_API bool py_iter(py_Ref);
```

### py_next [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Get the next element from the iterator.
/// 1: success, 0: StopIteration, -1: error
PK_API int py_next(py_Ref);
```

### py_str [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `str(val)`.
PK_API bool py_str(py_Ref val);
```

### py_repr [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `repr(val)`.
PK_API bool py_repr(py_Ref val);
```

### py_len [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `len(val)`.
PK_API bool py_len(py_Ref val);
```

### py_getattr [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `getattr(self, name)`.
PK_API bool py_getattr(py_Ref self, py_Name name);
```

### py_setattr [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Python equivalent to `setattr(self, name, val)`.
PK_API bool py_setattr(py_Ref self, py_Name name, py_Ref val);
```

### py_delattr [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Python equivalent to `delattr(self, name)`.
PK_API bool py_delattr(py_Ref self, py_Name name);
```

### py_getitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `self[key]`.
PK_API bool py_getitem(py_Ref self, py_Ref key);
```

### py_setitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Python equivalent to `self[key] = val`.
PK_API bool py_setitem(py_Ref self, py_Ref key, py_Ref val);
```

### py_delitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Python equivalent to `del self[key]`.
PK_API bool py_delitem(py_Ref self, py_Ref key);
```

### py_getmodule 
```c
/// Get a module by path.
PK_API py_GlobalRef py_getmodule(const char* path);
```

### py_newmodule 
```c
/// Create a new module.
PK_API py_GlobalRef py_newmodule(const char* path);
```

### py_importlib_reload [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Reload an existing module.
PK_API bool py_importlib_reload(py_Ref module);
```

### py_import [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Import a module.
/// The result will be set to `py_retval()`.
/// -1: error, 0: not found, 1: success
PK_API int py_import(const char* path);
```

### py_checkexc 
```c
/// Check if there is an unhandled exception.
PK_API bool py_checkexc();
```

### py_matchexc [!badge text="return"](../introduction/#py_return-macro)
```c
/// Check if the unhandled exception is an instance of the given type.
/// If match, the exception will be stored in `py_retval()`.
PK_API bool py_matchexc(py_Type type);
```

### py_clearexc 
```c
/// Clear the unhandled exception.
/// @param p0 the unwinding point. Use `NULL` if not needed.
PK_API void py_clearexc(py_StackRef p0);
```

### py_printexc 
```c
/// Print the unhandled exception.
PK_API void py_printexc();
```

### py_formatexc 
```c
/// Format the unhandled exception and return a null-terminated string.
/// The returned string should be freed by the caller.
PK_API char* py_formatexc();
```

### py_exception [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Raise an exception by type and message. Always return false.
PK_API bool py_exception(py_Type type, const char* fmt, ...);
```

### py_raise [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// Raise an exception object. Always return false.
PK_API bool py_raise(py_Ref);
```

### KeyError [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c

PK_API bool KeyError(py_Ref key);
```

### StopIteration [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c

PK_API bool StopIteration();
```

### py_debugger_waitforattach 
```c

PK_API void py_debugger_waitforattach(const char* hostname, unsigned short port);
```

### py_debugger_status 
```c

PK_API int py_debugger_status();
```

### py_debugger_exceptionbreakpoint 
```c

PK_API void py_debugger_exceptionbreakpoint(py_Ref exc);
```

### py_debugger_exit 
```c

PK_API void py_debugger_exit(int code);
```

### py_newtuple 
```c
/// Create a `tuple` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
PK_API py_ObjectRef py_newtuple(py_OutRef, int n);
```

### py_tuple_data 
```c

PK_API py_ObjectRef py_tuple_data(py_Ref self);
```

### py_tuple_getitem 
```c

PK_API py_ObjectRef py_tuple_getitem(py_Ref self, int i);
```

### py_tuple_setitem 
```c

PK_API void py_tuple_setitem(py_Ref self, int i, py_Ref val);
```

### py_tuple_len 
```c

PK_API int py_tuple_len(py_Ref self);
```

### py_newlist 
```c
/// Create an empty `list`.
PK_API void py_newlist(py_OutRef);
```

### py_newlistn 
```c
/// Create a `list` with `n` UNINITIALIZED elements.
/// You should initialize all elements before using it.
PK_API void py_newlistn(py_OutRef, int n);
```

### py_list_data 
```c

PK_API py_ItemRef py_list_data(py_Ref self);
```

### py_list_getitem 
```c

PK_API py_ItemRef py_list_getitem(py_Ref self, int i);
```

### py_list_setitem 
```c

PK_API void py_list_setitem(py_Ref self, int i, py_Ref val);
```

### py_list_delitem 
```c

PK_API void py_list_delitem(py_Ref self, int i);
```

### py_list_len 
```c

PK_API int py_list_len(py_Ref self);
```

### py_list_swap 
```c

PK_API void py_list_swap(py_Ref self, int i, int j);
```

### py_list_append 
```c

PK_API void py_list_append(py_Ref self, py_Ref val);
```

### py_list_emplace 
```c

PK_API py_ItemRef py_list_emplace(py_Ref self);
```

### py_list_clear 
```c

PK_API void py_list_clear(py_Ref self);
```

### py_list_insert 
```c

PK_API void py_list_insert(py_Ref self, int i, py_Ref val);
```

### py_newdict 
```c
/// Create an empty `dict`.
PK_API void py_newdict(py_OutRef);
```

### py_dict_getitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem(py_Ref self, py_Ref key);
```

### py_dict_setitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// true: success, false: error
PK_API bool py_dict_setitem(py_Ref self, py_Ref key, py_Ref val);
```

### py_dict_delitem [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem(py_Ref self, py_Ref key);
```

### py_dict_getitem_by_str [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem_by_str(py_Ref self, const char* key);
```

### py_dict_getitem_by_int [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// -1: error, 0: not found, 1: found
PK_API int py_dict_getitem_by_int(py_Ref self, py_i64 key);
```

### py_dict_setitem_by_str [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// true: success, false: error
PK_API bool py_dict_setitem_by_str(py_Ref self, const char* key, py_Ref val);
```

### py_dict_setitem_by_int [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// true: success, false: error
PK_API bool py_dict_setitem_by_int(py_Ref self, py_i64 key, py_Ref val);
```

### py_dict_delitem_by_str [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem_by_str(py_Ref self, const char* key);
```

### py_dict_delitem_by_int [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// -1: error, 0: not found, 1: found (and deleted)
PK_API int py_dict_delitem_by_int(py_Ref self, py_i64 key);
```

### py_dict_apply [!badge text="raise" variant="danger"](../introduction/#py_raise-macro)
```c
/// true: success, false: error
PK_API bool py_dict_apply(py_Ref self, bool (*f)(py_Ref key, py_Ref val, void* ctx), void* ctx);
```

### py_dict_len 
```c
/// noexcept
PK_API int py_dict_len(py_Ref self);
```

### py_newslice 
```c
/// Create an UNINITIALIZED `slice` object.
/// You should use `py_setslot()` to set `start`, `stop`, and `step`.
PK_API py_ObjectRef py_newslice(py_OutRef);
```

### py_newsliceint 
```c
/// Create a `slice` object from 3 integers.
PK_API void py_newsliceint(py_OutRef out, py_i64 start, py_i64 stop, py_i64 step);
```

### py_newRandom 
```c

PK_API void py_newRandom(py_OutRef out);
```

### py_Random_seed 
```c

PK_API void py_Random_seed(py_Ref self, py_i64 seed);
```

### py_Random_random 
```c

PK_API py_f64 py_Random_random(py_Ref self);
```

### py_Random_uniform 
```c

PK_API py_f64 py_Random_uniform(py_Ref self, py_f64 a, py_f64 b);
```

### py_Random_randint 
```c

PK_API py_i64 py_Random_randint(py_Ref self, py_i64 a, py_i64 b);
```

### py_newarray2d 
```c

PK_API void py_newarray2d(py_OutRef out, int width, int height);
```

### py_array2d_getwidth 
```c

PK_API int py_array2d_getwidth(py_Ref self);
```

### py_array2d_getheight 
```c

PK_API int py_array2d_getheight(py_Ref self);
```

### py_array2d_getitem 
```c

PK_API py_ObjectRef py_array2d_getitem(py_Ref self, int x, int y);
```

### py_array2d_setitem 
```c

PK_API void py_array2d_setitem(py_Ref self, int x, int y, py_Ref val);
```

### py_newvec2 
```c

PK_API void py_newvec2(py_OutRef out, c11_vec2);
```

### py_newvec3 
```c

PK_API void py_newvec3(py_OutRef out, c11_vec3);
```

### py_newvec2i 
```c

PK_API void py_newvec2i(py_OutRef out, c11_vec2i);
```

### py_newvec3i 
```c

PK_API void py_newvec3i(py_OutRef out, c11_vec3i);
```

### py_newvec4i 
```c

PK_API void py_newvec4i(py_OutRef out, c11_vec4i);
```

### py_newcolor32 
```c

PK_API void py_newcolor32(py_OutRef out, c11_color32);
```

### py_newmat3x3 
```c

PK_API c11_mat3x3* py_newmat3x3(py_OutRef out);
```

### py_tovec2 
```c

PK_API c11_vec2 py_tovec2(py_Ref self);
```

### py_tovec3 
```c

PK_API c11_vec3 py_tovec3(py_Ref self);
```

### py_tovec2i 
```c

PK_API c11_vec2i py_tovec2i(py_Ref self);
```

### py_tovec3i 
```c

PK_API c11_vec3i py_tovec3i(py_Ref self);
```

### py_tovec4i 
```c

PK_API c11_vec4i py_tovec4i(py_Ref self);
```

### py_tomat3x3 
```c

PK_API c11_mat3x3* py_tomat3x3(py_Ref self);
```

### py_tocolor32 
```c

PK_API c11_color32 py_tocolor32(py_Ref self);
```

### py_json_dumps [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `json.dumps(val)`.
PK_API bool py_json_dumps(py_Ref val, int indent);
```

### py_json_loads [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `json.loads(val)`.
PK_API bool py_json_loads(const char* source);
```

### py_pickle_dumps [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `pickle.dumps(val)`.
PK_API bool py_pickle_dumps(py_Ref val);
```

### py_pickle_loads [!badge text="raise" variant="danger"](../introduction/#py_raise-macro) [!badge text="return"](../introduction/#py_return-macro)
```c
/// Python equivalent to `pickle.loads(val)`.
PK_API bool py_pickle_loads(const unsigned char* data, int size);
```

### py_watchdog_begin 
```c
/// Begin the watchdog with `timeout` in milliseconds.
/// `PK_ENABLE_WATCHDOG` must be defined to `1` to use this feature.
/// You need to call `py_watchdog_end()` later.
/// If `timeout` is reached, `TimeoutError` will be raised.
PK_API void py_watchdog_begin(py_i64 timeout);
```

### py_watchdog_end 
```c
/// Reset the watchdog.
PK_API void py_watchdog_end();
```

### py_profiler_begin 
```c

PK_API void py_profiler_begin();
```

### py_profiler_end 
```c

PK_API void py_profiler_end();
```

### py_profiler_reset 
```c

PK_API void py_profiler_reset();
```

### py_profiler_report 
```c

PK_API char* py_profiler_report();
```

### py_replinput 
```c
/// An utility function to read a line from stdin for REPL.
PK_API int py_replinput(char* buf, int max_size);
```
