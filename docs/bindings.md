---
icon: cpu
title: Write Bindings
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
