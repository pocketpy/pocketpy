---
icon: dot
label: 'Bind native function'
order: 60
---

pkpy allows to wrap a function pointer as a python function or method that can be called in python code.
This function pointer has the following signature:

```cpp
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
```
+ The first argument is the pointer of `VM` instance.
+ The second argument is an array-like object indicates the arguments list. You can use `[]` operator to get the element.
+ The return value is a `PyObject*`, which should not be `nullptr`. If there is no return value, return `vm->None`.


## Bind a function or method

Use `vm->bind` to bind a function or method.

+ `PyObject* bind(PyObject*, const char* sig, NativeFuncC)`
+ `PyObject* bind(PyObject*, const char* sig, const char* docstring, NativeFuncC)`

```cpp

vm->bind(obj, "add(a: int, b: int) -> int", [](VM* vm, ArgsView args){
    int a = CAST(int, args[0]);
    int b = CAST(int, args[1]);
    return VAR(a + b);
});

// or you can provide a docstring
vm->bind(obj,
    "add(a: int, b: int) -> int",
    "add two integers", [](VM* vm, ArgsView args){
    int a = CAST(int, args[0]);
    int b = CAST(int, args[1]);
    return VAR(a + b);
});
```

### Bind a magic method

For some magic methods, we provide specialized binding function.
They do not take universal function pointer as argument.
You need to provide the detailed `Type` object and the corresponding function pointer.

```cpp
PyObject* __add__(PyObject* lhs, PyObject* rhs){
    int a = CAST(int, lhs);
    int b = CAST(int, rhs);
    return VAR(a + b);
}

Type type = vm->tp_int;
vm->bind__add__(type, __add__);
```

This specialized binding function has optimizations and result in better performance when calling from python code.

For example, `vm->bind__add__` is preferred over `vm->bind_method<1>(type, "__add__", ...)`.

### Bind a property

a property is a python's `property` that attached to a type instance with a getter and an optional setter. It is a data descriptor. A property redirects attribute access to specific functions.

You can use `@property` to create python property or use `vm->property` to create native property.

Use `vm->bind_property()`, the new style property binding function.

```cpp
struct Point {
  PY_CLASS(Point, test, Point);

  int x;
  int y;

  Point(int x, int y) : x(x), y(y) {}

  static void _register(VM *vm, auto mod, auto type) {
    vm->bind_constructor<3>(type, [](VM *vm, auto args) {
      auto x = CAST(i64, args[1]);
      auto y = CAST(i64, args[2]);
      return VAR_T(Point, x, y);
    });

    // getter and setter of property `x`
    vm->bind_property(type, "x: int",
      [](VM* vm, ArgsView args){
          Point& self = CAST(Point&, args[0]);
          return VAR(self.x);
      },
      [](VM* vm, ArgsView args){
          Point& self = CAST(Point&, args[0]);
          self.x = CAST(int, args[1]);
          return vm->None;
      });
  }
};
```

### Old style binding

You may see somewhere in the code that `vm->bind_method<>` or `vm->bind_func<>` is used.
They are old style binding functions and are deprecated.
You should use `vm->bind` instead.