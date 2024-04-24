---
icon: dot
label: 'Access Attributes'
order: 80
---

### Direct access

Some python objects have an instance dict, a.k.a, `__dict__` in cpython.
You can use `obj->attr()` to manipulate the instance dict of an object.

```cpp
VM* vm = new VM();

// get the `builtin` module
PyObject* builtins = vm->builtins;
// get `dict` type
PyObject* dict = builtins->attr("dict");
// set `pi = 3.14`
builtins->attr().set("pi", py_var(vm, 3.14));
```

However, you cannot call `attr` on an object which does not have an instance dict.
For example, the `int` object.

```cpp
// create a `int` object
PyObject* obj = py_var(vm, 1);
// THIS IS WRONG!! WILL LEAD TO A SEGFAULT!!
PyObject* add = obj->attr("__add__");
```

To determine whether an object has instance dict or not, you can use this snippet.

```cpp
// 1. call `is_tagged` to check the object supports `->` operator
// 2. call `is_attr_valid` to check the existence of instance dict
PyObject* obj = py_var(vm, 1);
bool ok = !is_tagged(obj) && obj->is_attr_valid();  // false
```

### General access

As you can see, direct access does not take care of derived attributes or methods.
In most cases, what you need is `getattr` and `setattr`.
These two methods handle all possible cases.

#### `PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true)`

This method is equivalent to `getattr` in python.
If the attribute is not found, it will return `nullptr`
or throw an `AttributeError` depending on the value of `throw_err`.

```cpp
// create a `int` object
PyObject* obj = py_var(vm, 1);

// get its `__add__` method, which is a `bound_method` object
PyObject* add = vm->getattr(obj, "__add__");

// call it (equivalent to `1 + 2`)
PyObject* ret = vm->call(add, py_var(vm, 2););

// get the result
int result = py_cast<int>(vm, ret);
std::cout << result << std::endl; // 3
```

#### `void setattr(PyObject*, StrName, PyObject*)`

This method is equivalent to `setattr` in python.
It raises `TypeError` if the object does not support attribute assignment.
