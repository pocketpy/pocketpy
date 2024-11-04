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

## Steps

Say you have a function `add` that takes two integers and returns their sum.
```c
int add(int a, int b) {
    return a + b;
}
```

Here is how you can write the binding for it:
```c
// 1. Define a wrapper function with the signature `py_CFunction`.
bool py_add(int argc, py_Ref argv) {
    // 2. Check the number of arguments.
    PY_CHECK_ARGC(2);
    // 3. Check the type of arguments.
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    // 4. Convert the arguments into C types.
    int _0 = py_toint(py_arg(0));
    int _1 = py_toint(py_arg(1));
    // 5. Call the original function.
    int res = add(_0, _1);
    // 6. Set the return value.
    py_newint(py_retval(), res);
    // 7. Return `true`.
    return true;
}
```

Once you have the wrapper function, you can bind it to a python module via `py_bindfunc`.
```c
py_GlobalRef mod = py_getmodule("__main__");
py_bindfunc(mod, "add", py_add);
```

Alternatively, you can use `py_bind` with a signature, which allows you to specify some default values.
```c
py_GlobalRef mod = py_getmodule("__main__");
py_bind(mod, "add(a, b=1)", py_add);
```

See also:
+ [`py_bind`](/c-api/functions/#py_bind)
+ [`py_bindmethod`](/c-api/functions/#py_bindmethod)
+ [`py_bindfunc`](/c-api/functions/#py_bindfunc)
+ [`py_bindproperty`](/c-api/functions/#py_bindproperty)
+ [`py_newmodule`](/c-api/functions/#py_newmodule)
+ [`py_newtype`](/c-api/functions/#py_newtype)