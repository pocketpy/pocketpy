---
icon: star
title: Write bindings
order: 18
---

!!!
This document is working in progress.
!!!

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

## Bind a property

a property is a python's `property` that attached to a type instance with a getter and an optional setter. It is a data descriptor. A property redirects attribute access to specific functions.

Use `vm->bind_property()` to bind a getter and an optional setter to a property.

```cpp
struct Point {
  PY_CLASS(Point, test, Point);

  int x;
  int y;

  Point(int x, int y) : x(x), y(y) {}

  static void _register(VM *vm, auto mod, auto type) {
    vm->bind(type, "__new__(cls, x, y)", [](VM *vm, ArgsView args) {
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

## Others

You may see somewhere in the code that `vm->bind_method<>` or `vm->bind_func<>` is used.
They are old style binding functions and are deprecated.
You should use `vm->bind` instead.

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

Add `PY_CLASS` macro into your `struct` and implement a static function `_register`.
Inside the `_register` function, you can bind methods and properties to the class.

```cpp
PY_CLASS(T, mod, name)

// T is the struct type in cpp
// mod is the module name in python
// name is the class name in python
```

## Example

In this example, we will create a `linalg` module
and implement a `vec2` type with some methods.
And make them available in python just like this.

```python
from linalg import vec2

# construct a vec2
a = vec2(1.0, 2.0)
b = vec2(0.0, -1.0)

# add two vec2
print(a + b)    # vec2(1.0, 1.0)

# set x component
a.x = 8.0
print(a)        # vec2(8.0, 2.0)

# use dot method
print(a.dot(b)) # -2.0
```

### Implement `Vec2` struct in cpp

```cpp
struct Vec2{
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const Vec2& v) : x(v.x), y(v.y) {}
    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    float dot(const Vec2& v) const { return x * v.x + y * v.y; }
};
```

### Create `PyVec2` wrapper

```cpp
struct PyVec2: Vec2 {
    PY_CLASS(PyVec2, linalg, vec2)

    PyVec2() : Vec2() {}
    PyVec2(const Vec2& v) : Vec2(v) {}
    PyVec2(const PyVec2& v) : Vec2(v) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
            float x = CAST_F(args[1]);
            float y = CAST_F(args[2]);
            return VAR(Vec2(x, y));
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            PyVec2& self = _CAST(PyVec2&, obj);
            std::stringstream ss;
            ss << "vec2(" << self.x << ", " << self.y << ")";
            return VAR(ss.str());
        });

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* other){
            PyVec2& self = _CAST(PyVec2&, obj);
            PyVec2& other_ = CAST(PyVec2&, other);
            return VAR_T(PyVec2, self + other_);
        });

        vm->bind(type, "dot(self, other: vec2) -> float", [](VM* vm, ArgsView args){
            PyVec2& self = _CAST(PyVec2&, args[0]);
            PyVec2& other = CAST(PyVec2&, args[1]);
            return VAR(self.dot(other));
        });
    }
};
```

### Create `linalg` module

```cpp
void add_module_linalg(VM* vm){
    PyObject* linalg = vm->new_module("linalg");
    // register PyVec2
    PyVec2::register_class(vm, linalg);
}
```

### Further reading

See [linalg.h](https://github.com/blueloveTH/pocketpy/blob/main/src/linalg.h) for the complete implementation.