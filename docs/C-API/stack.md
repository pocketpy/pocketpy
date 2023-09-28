---
title: Stack Manipulation
icon: dot
order: 8
---

### Basic manipulation

+ `bool pkpy_dup(pkpy_vm*, int)`

    Duplicate the value at the given index.

+ `bool pkpy_pop(pkpy_vm*, int)`

    Pop `n` values from the stack.

+ `bool pkpy_pop_top(pkpy_vm*)`
    
    Pop the top value from the stack.

+ `bool pkpy_dup_top(pkpy_vm*)`

    Duplicate the top value on the stack.

+ `bool pkpy_rot_two(pkpy_vm*)`

    Swap the top two values on the stack.

+ `int pkpy_stack_size(pkpy_vm*)`

    Get the element count of the stack.


### Basic push, check and convert

+ `pkpy_push_xxx` pushes a value onto the stack.
+ `pkpy_is_xxx` checks if the value at the given index is of the given type.
+ `pkpy_to_xxx` converts the value at the given index to the given type.

Stack index is 0-based instead of 1-based. And it can be negative, which means the index is counted from the top of the stack.

```c
// int
PK_EXPORT bool pkpy_push_int(pkpy_vm*, int);
PK_EXPORT bool pkpy_is_int(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_int(pkpy_vm*, int i, int* out);

// float
PK_EXPORT bool pkpy_push_float(pkpy_vm*, double);
PK_EXPORT bool pkpy_is_float(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_float(pkpy_vm*, int i, double* out);

// bool
PK_EXPORT bool pkpy_push_bool(pkpy_vm*, bool);
PK_EXPORT bool pkpy_is_bool(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_bool(pkpy_vm*, int i, bool* out);

// string
PK_EXPORT bool pkpy_push_string(pkpy_vm*, pkpy_CString);
PK_EXPORT bool pkpy_is_string(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_string(pkpy_vm*, int i, pkpy_CString* out);

// void_p
PK_EXPORT bool pkpy_push_voidp(pkpy_vm*, void*);
PK_EXPORT bool pkpy_is_voidp(pkpy_vm*, int i);
PK_EXPORT bool pkpy_to_voidp(pkpy_vm*, int i, void** out);

// none
PK_EXPORT bool pkpy_push_none(pkpy_vm*);
PK_EXPORT bool pkpy_is_none(pkpy_vm*, int i);
```

### Special push

+ `pkpy_push_null(pkpy_vm*)`

    Push a `PY_NULL` onto the stack. It is used for `pkpy_vectorcall`.

+ `pkpy_push_function(pkpy_vm*, const char* sig, pkpy_CFunction f)`

    Push a function onto the stack. `sig` is the function signature, e.g. `add(a: int, b: int) -> int`. `f` is the function pointer.

+ `pkpy_push_module(pkpy_vm*, const char* name)`

    Push a new module onto the stack. `name` is the module name. This is not `import`. It creates a new module object.

### Variable access

+ `bool pkpy_getattr(pkpy_vm*, pkpy_CName name)`

    Push `b.<name>` onto the stack. Return false if the attribute is not found.

    ```
    [b] -> [b.<name>]
    ```

+ `bool pkpy_setattr(pkpy_vm*, pkpy_CName name)`

    Set `b.<name>` to the value at the top of the stack.
    First push the value, then push `b`.

    ```
    [value, b] -> []
    ```

+ `bool pkpy_getglobal(pkpy_vm*, pkpy_CName name)`

    Push a global/builtin variable onto the stack. Return false if the variable is not found.

    ```
    [] -> [value]
    ```

+ `bool pkpy_setglobal(pkpy_vm*, pkpy_CName name)`

    Set a global variable to the value at the top of the stack.

    ```
    [value] -> []
    ```

+ `bool pkpy_eval(pkpy_vm*, const char* source)`

    Evaluate a string and push the result onto the stack.

    ```
    [] -> [result]
    ```

+ `bool pkpy_unpack_sequence(pkpy_vm*, int size)`

    Unpack a sequence at the top of the stack. `size` is the element count of the sequence.

    ```
    [a] -> [a[0], a[1], ..., a[size - 1]]
    ```

+ `bool pkpy_get_unbound_method(pkpy_vm*, pkpy_CName name)`

    It is used for method call.
    Get an unbound method from the object at the top of the stack. `name` is the method name.
    Also push the object as self.

    ```
    [obj] -> [obj.<name> self]
    ```
+ `bool pkpy_py_repr(pkpy_vm*)`

    Get the repr of the value at the top of the stack.

    ```
    [value] -> [repr(value)]
    ```