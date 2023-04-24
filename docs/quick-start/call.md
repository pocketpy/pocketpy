---
icon: dot
label: 'Call Python function'
order: 70
---

pkpy uses a variant of the [Vectorcall](https://peps.python.org/pep-0590/) protocol (PEP 590).

There are 2 methods for calling a python function.

+ `PyObject* VM::call(PyObject* obj, ...)`
+ `PyObject* VM::call_method(PyObject* obj, StrName name, ...)`

For example, to create a `dict` object,

```cpp
PyObject* tp = vm->builtins->attr("dict");
PyObject* obj = vm->call(tp);	// this is a `dict`
```

And set a key-value pair,

```cpp
vm->call_method(obj, "__setitem__", VAR("a"), VAR(5));
PyObject* ret = vm->call(obj, "__getitem__", VAR("a"));
std::cout << CAST(int, ret) << std::endl; // 5
```