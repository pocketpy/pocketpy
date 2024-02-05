---
icon: cpu
title: Write Bindings
order: 18
---

In order to use a C/C++ library in python, you need to write bindings for it.

## Manual bindings

pkpy uses an universal signature to wrap a function pointer as a python function or method that can be called in python code, i.e `NativeFuncC`.

```cpp
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
```
+ The first argument is the pointer of `VM` instance.
+ The second argument is an array-like object indicates the arguments list. You can use `[]` operator to get the element and call `size()` to get the length of the array.
+ The return value is a `PyObject*`, which should not be `nullptr`. If there is no return value, return `vm->None`.

### Bind a function or method

Use `vm->bind` to bind a function or method.

+ `PyObject* bind(PyObject*, const char* sig, NativeFuncC)`
+ `PyObject* bind(PyObject*, const char* sig, const char* docstring, NativeFuncC)`

```cpp

vm->bind(obj, "add(a: int, b: int) -> int", [](VM* vm, ArgsView args){
    int a = py_cast<int>(vm, args[0]);
    int b = py_cast<int>(vm, args[1]);
    return py_var(vm, a + b);
});

// or you can provide a docstring
vm->bind(obj,
    "add(a: int, b: int) -> int",
    "add two integers", [](VM* vm, ArgsView args){
    int a = py_cast<int>(vm, args[0]);
    int b = py_cast<int>(vm, args[1]);
    return py_var(vm, a + b);
});
```

#### How to capture something

By default, the lambda being bound is a C function pointer,
you cannot capture anything! The following example does not compile.

```cpp
int x = 1;
vm->bind(obj, "f() -> int", [x](VM* vm, ArgsView args){
    // error: cannot capture 'x'
    return py_var(vm, x);
});
```

I do not encourage you to capture something in a lambda being bound
because:
1. Captured lambda runs slower and causes "code-bloat".
2. Captured values are unsafe, especially for `PyObject*` as they could leak by accident.

However, there are 3 ways to capture something when you really need to.
The most safe and elegant way is to subclass `VM` and add a member variable.

```cpp
class YourVM : public VM{
public:
    int x;
    YourVM() : VM() {}
};

int main(){
    YourVM* vm = new YourVM();
    vm->x = 1;
    vm->bind(obj, "f() -> int", [](VM* _vm, ArgsView args){
        // do a static_cast and you can get any extra members of YourVM
        YourVM* vm = static_cast<YourVM*>(_vm);
        return py_var(vm, vm->x);
    });
    return 0;
}
```

The 2nd way is to use `vm->bind`'s last parameter `userdata`, you can store a POD type smaller than 8 bytes.
And use `lambda_get_userdata<T>(args.begin())` to get it inside the lambda body.

```cpp
int x = 1;
vm->bind(obj, "f() -> int", [](VM* vm, ArgsView args){
    // get the userdata
    int x = lambda_get_userdata<int>(args.begin());
    return py_var(vm, x);
}, x);  // capture x
```

The 3rd way is to change the macro `PK_ENABLE_STD_FUNCTION` in `config.h`:
```cpp
#define PK_ENABLE_STD_FUNCTION 0   // => 1
```

Then you can use standard capture list in lambda.

### Bind a struct

Assume you have a struct `Point` declared as follows.

```cpp
struct Point{
    int x;
    int y;
}
```

You can write a wrapper class `wrapped__Point`. Add `PY_CLASS` macro into your wrapper class and implement a static function `_register`.

Inside the `_register` function, do bind methods and properties.

```cpp
PY_CLASS(T, mod, name)

// T is the struct type in cpp
// mod is the module name in python
// name is the class name in python
```

### Example

```cpp
struct wrapped__Point{
    // special macro for wrapper class
    PY_CLASS(wrapped__Point, builtins, Point)
    //       ^T              ^module   ^name

    // wrapped value
    Point value;

    // special method _ returns a pointer of the wrapped value
    Point* _() { return &value; }

    // define default constructors
    wrapped__Point() = default;
    wrapped__Point(const wrapped__Point&) = default;

    // define wrapped constructor
    wrapped__Point(Point value){
        this->value = value;
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        // enable default constructor and struct-like methods
        // if you don't use this, you must bind a `__new__` method as constructor
        PY_STRUCT_LIKE(wrapped__Point)

        // wrap field x
        PY_FIELD(wrapped__Point, "x", _, x)
        // wrap field y
        PY_FIELD(wrapped__Point, "y", _, y)

        // __init__ method
        vm->bind(type, "__init__(self, x, y)", [](VM* vm, ArgsView args){
            wrapped__Point& self = _py_cast<wrapped__Point&>(vm, args[0]);
            self.value.x = py_cast<int>(vm, args[1]);
            self.value.y = py_cast<int>(vm, args[2]);
            return vm->None;
        });

        // other custom methods
        // ...
    }
}

int main(){
    VM* vm = new VM();
    // register the wrapper class somewhere
    wrapped__Point::register_class(vm, vm->builtins);

    // use the Point class
    vm->exec("a = Point(1, 2)");
    vm->exec("print(a.x)");         // 1
    vm->exec("print(a.y)");         // 2

    delete vm;
    return 0;
}
```

#### Handle gc for container types

If your custom type stores `PyObject*` in its fields, you need to handle gc for them.

```cpp
struct Container{
    PY_CLASS(Container, builtins, Container)

    PyObject* a;
    std::vector<PyObject*> b;
    // ...
}
```

Add a magic method `_gc_mark() const` to your custom type.

```cpp
struct Container{
    PY_CLASS(Container, builtins, Container)

    PyObject* a;
    std::vector<PyObject*> b;
    // ...

    void _gc_mark() const{
        // mark a
        if(a) PK_OBJ_MARK(a);

        // mark elements in b
        for(PyObject* obj : b){
            if(obj) PK_OBJ_MARK(obj);
        }
    }
}
```

For global objects, use the callback in `vm->heap`.
```cpp
void (*_gc_marker_ex)(VM*) = nullptr;
```
It will be invoked before a GC starts. So you can mark objects inside the callback to keep them alive.

### Others

You may see somewhere in the code that `vm->bind_method<>` or `vm->bind_func<>` is used.
They are old style binding functions and are deprecated.
It is recommended to use `vm->bind`.

For some magic methods, we provide specialized binding function.
They do not take universal function pointer as argument.
You need to provide the detailed `Type` object and the corresponding function pointer.

```cpp
PyObject* f_add(VM* vm, PyObject* lhs, PyObject* rhs){
    int a = py_cast<int>(vm, lhs);
    int b = py_cast<int>(vm, rhs);
    return py_var(vm, a + b);
}

vm->bind__add__(vm->tp_int, f_add);
```

This specialized binding function has optimizations and result in better performance when calling from python code.

For example, `vm->bind__add__` is preferred over `vm->bind_method<1>(type, "__add__", ...)`.


## Automatic bindings

pkpy supports automatic binding generation **only for C libraries**.
See [pkpy-bindings](https://github.com/blueloveTH/pkpy-bindings) for details.

It takes a C header file and generates a python module stub (`*.pyi`) and a C++ binding file (`*.cpp`).


## Further reading

See [random.cpp](https://github.com/pocketpy/pocketpy/blob/main/src/random.cpp) for an example used by `random` module.

See [collections.cpp](https://github.com/pocketpy/pocketpy/blob/main/src/collections.cpp) for a modern implementation of `collections.deque`.