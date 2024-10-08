---
title: Introduction
icon: dot
order: 10
---

All public functions in the C API are prefixed with `py_` in [pocketpy.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/pocketpy.h).


## Overview

pocketpy works with opaque references. `py_Ref` is used to reference objects in the virtual machine. It is your responsibility to ensure a reference is valid before using it. See following reference types:

```c
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
/// An output reference for returning a value.
typedef py_TValue* py_OutRef;
```

You can store python objects into "stack" or "register".
We provide 8 registers and you can get references to them by `py_reg()`.
Also, `py_retval()` is a special register that is used to store the return value of a `py_CFunction`.
Registers are shared so they could be overwritten easily.
If you want to store python objects across function calls, you should store them into the stack via `py_push()` and `py_pop()`.

## Data Types

You can do conversions between C types and python objects using the following functions:

| C type              | Python type | C to Python     | Python to C                      |
| ------------------- | ----------- | --------------- | -------------------------------- |
| char,short,int,long | int         | `py_newint()`   | `py_toint()`                     |
| float,double        | float       | `py_newfloat()` | `py_tofloat()`, `py_castfloat()` |
| bool                | bool        | `py_newbool()`  | `py_tobool()`                    |
| const char*         | str         | `py_newstr()`   | `py_tostr()`                     |
| void*,intptr_t      | int         | `py_newint()`   | `(void*)py_toint()`              |

---

### `PY_RAISE` macro

Mark a function that can raise an exception on failure.

+ If the function returns `bool`, then `false` means an exception is raised.
+ If the function returns `int`, then `-1` means an exception is raised.

### `PY_RETURN` macro

Mark a function that can store a value in `py_retval()` on success.