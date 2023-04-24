---
icon: dot
label: 'Interop with PyObject'
order: 90
---

In pkpy, any python object is represented by a `PyObject*`.
There are 3 macros for you to do convert.

+  `VAR(...)`,
create a `PyObject*` from a C type
+ `CAST(T, ...)`,
cast a `PyObject*` to a C type
+ `_CAST(T, ...)`,
cast a `PyObject*` to a C type, without type check

```cpp
PyObject* x = VAR(12);		// cast a C int to PyObject*
int y = CAST(int, x);		// cast a PyObject* to C int

PyObject* i = VAR("abc");
std::cout << CAST(Str, i);	// abc
```

### Types

| python type  | C type           | note                   |
| ------------ | ---------------- | ---------------------- |
| `int`        | `i64`            | 62 bits integer        |
| `float`      | `f64`            | 62 bits floating point |
| `str`        | `pkpy::Str`      |                        |
| `bool`       | `bool`           |                        |
| `list`       | `pkpy::List`     |                        |
| `tuple`      | `pkpy::Tuple`    |                        |
| `function`   | `pkpy::Function` |                        |
| ...          | ...              | ...                    |

### Type check

+ `is_type(PyObject* obj, Type type)`
+ `is_non_tagged_type(PyObject* obj, Type type)`
+ `VM::check_type(PyObject* obj, Type type)`