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

!!!
Native functions do not support keyword arguments.
!!!

### Bind a function

Assume you have a cpp function `bool equals(int a, int b)`.
```cpp
bool equals(int a, int b){
    return a == b;
}
```

You can bind it into `test.equals` by using `vm->bind_func<ARGC>`:

```cpp
PyObject* obj = vm->new_module("test");

//                     v [function name]
vm->bind_func<2>(obj, "equals", [](VM* vm, ArgsView args){
//            ^ argument count
    int a = CAST(int, args[0]);
    int b = CAST(int, args[1]);
    bool result = equals(a, b);
    return VAR(result);
});
```

+ The first argument is the target object to bind. It can be any python object with an instance dict, such as a module, a class, or an instance.
+ The second argument is the function name.
+ The third argument is the function pointer. We often use lambda expression to wrap it. A non-capturing lambda expression can be converted to a function pointer.

The template argument `ARGC` is the argument count of the function. If the function is variadic, use `-1` as the argument count.

The interpreter will ensure `args.size() == ARGC` and throws `TypeError` if not.
For variadic functions, you need to check `args.size()` manually.

If you want to bind a function into `builtins` module, use `vm->bind_builtin_func<ARGC>` instead.


### Bind a constructor

The constructor of a class is a special function that returns an instance of the class.
It corresponds to the `__new__` magic method in python (not `__init__`).

```cpp
vm->bind_constructor<3>(type, [](VM* vm, ArgsView args){
    float x = CAST_F(args[1]);
    float y = CAST_F(args[2]);
    return VAR(Vec2(x, y));
});
```

### Bind a method

The `vm->bind_method<ARGC>` usage is almost the same as `vm->bind_func<ARGC>`.
The only difference is that `ARGC` in `vm->bind_method<ARGC>` does not include the `self` argument.

```cpp
vm->bind_method<1>("int", "equals", [](VM* vm, ArgsView args){
    int self = CAST(int, args[0]);
    int other = CAST(int, args[1]);
    return VAR(self == other);
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
    type->attr().set("x", vm->property([](VM* vm, ArgsView args){
        Point& self = CAST(Point&, args[0]);
        return VAR(self.x);
    },
    [](VM* vm, ArgsView args){
        Point& self = CAST(Point&, args[0]);
        self.x = CAST(int, args[1]);
        return vm->None;
    }));
  }
};
```