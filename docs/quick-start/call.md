---
icon: dot
label: 'Call Python function'
order: 70
---

pkpy uses a variant of the [Vectorcall](https://peps.python.org/pep-0590/) protocol (PEP 590).

You can use `call` to invoke any python callable object,
including functions, methods, classes, etc.
For methods, `call_method` can be used.

+ `PyObject* call(PyObject* obj, ...)`
+ `PyObject* call_method(PyObject* obj, StrName name, ...)`

### Exmaple

Let's create a `dict` object and set a key-value pair,
which equals to the following python snippet.

```python
obj = {}        # declare a `dict`
obj["a"] = 5    # set a key-value pair
print(obj["a"]) # print the value
```

First, create an empty dict object,

```cpp
PyObject* tp = vm->builtins->attr("dict");
PyObject* obj = vm->call(tp);	// this is a `dict`
```

And set a key-value pair,

```cpp
PyObject* _0 = py_var(vm, "a");
PyObject* _1 = py_var(vm, 5);
vm->call_method(obj, "__setitem__", _0, _1);
```

And get the value,

```cpp
PyObject* ret = vm->call_method(obj, "__getitem__", _0);
std::cout << py_cast<int>(vm, i64);
```