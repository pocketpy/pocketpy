---
icon: cpu
title: Write C Bindings
order: 18
---

In order to use a C/C++ library in python, you need to write bindings for it.

pkpy uses an universal signature to wrap a C function pointer as a python function or method, i.e `py_CFunction`.

```c
typedef bool (*py_CFunction)(int argc, py_Ref argv);
```
+ `argc` is the number of arguments passed to the function.
+ `argv` is the pointer to the first argument.

If successful, the function should return `true` and set the return value in `py_retval()`. In case there is no return value, you should use `py_newnone(py_retval())`.
If an error occurs, the function should raise an exception and return `false`.

See also:
+ [`py_bind`](/c-api/functions/#py_bind)
+ [`py_bindmethod`](/c-api/functions/#py_bindmethod)
+ [`py_bindfunc`](/c-api/functions/#py_bindfunc)
+ [`py_bindproperty`](/c-api/functions/#py_bindproperty)
+ [`py_newmodule`](/c-api/functions/#py_newmodule)
+ [`py_newtype`](/c-api/functions/#py_newtype)