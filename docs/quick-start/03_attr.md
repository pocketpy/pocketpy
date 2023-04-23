---
icon: code
label: 'Attribute access'
order: 80
---

#### `PyObject* getattr(PyObject* obj, StrName name, bool throw_err=true)`

This method is equivalent to `getattr` in Python.
If the attribute is not found, it will return `nullptr`
or throw an `AttributeError` depending on the value of `throw_err`.

```cpp
// create a `int` object
PyObject* obj = VAR(1);

// get its `__add__` method, which is a `bound_method` object
PyObject* add = vm->getattr(obj, "__add__");

// call it (equivalent to `1 + 2`)
PyObject* ret = vm->call(add, VAR(2));

// get the result
int result = CAST(int, ret);
std::cout << result << std::endl; // 3
```

#### `void setattr(PyObject*, StrName, PyObject*)`

This method is equivalent to `setattr` in Python.
It raises `TypeError` if the object does not support attribute assignment.