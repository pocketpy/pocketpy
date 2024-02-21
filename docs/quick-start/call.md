---
icon: dot
label: 'Call Python Function'
order: 70
---

pkpy uses a variant of the [Vectorcall](https://peps.python.org/pep-0590/) protocol (PEP 590).

You can use `call` to invoke any python callable object,
including functions, methods, classes, etc.
For methods, `call_method` can be used.

+ `PyObject* call(PyObject* obj, ...)`
+ `PyObject* call_method(PyObject* obj, StrName name, ...)`

### Example

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

If you want to call with dynamic number of arguments,
you should use `vm->vectorcall`. This is a low-level, stack-based API.

1. First push the callable object to the stack.
2. Push the `self` object to the stack. If there is no `self`, push `PY_NULL`.
3. Push the arguments to the stack.
4. Call `vm->vectorcall` with the number of arguments.

```cpp
PyObject* f_sum = vm->builtins->attr("sum");

List args(N);   // a list of N arguments

vm->s_data.push_back(f_print);
vm->s_data.push_back(PY_NULL);  // self

for(PyObject* arg : args) {
    vm->s_data.push_back(arg);
}

PyObject* ret = vm->vectorcall(args.size());
```

