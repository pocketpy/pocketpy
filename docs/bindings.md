---
icon: cpu
title: Write Bindings
order: 18
---

In order to use a C/C++ library in python, you need to write bindings for it.

pkpy uses an universal signature to wrap a function pointer as a python function or method that can be called in python code, i.e `NativeFuncC`.

```cpp
typedef PyVar (*NativeFuncC)(VM*, ArgsView);
```
+ The first argument is the pointer of `VM` instance.
+ The second argument is an array-like object indicates the arguments list. You can use `[]` operator to get the element and call `size()` to get the length of the array.
+ The return value is a `PyVar`, which should not be `nullptr`. If there is no return value, return `vm->None`.

## Bind a function or method

Use `vm->bind` to bind a function or method.

+ `PyVar bind(PyVar, const char* sig, NativeFuncC)`
+ `PyVar bind(PyVar, const char* sig, const char* docstring, NativeFuncC)`

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

### How to capture something

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
2. Captured values are unsafe, especially for `PyVar` as they could leak by accident.

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

The 2nd way is to use `vm->bind`'s last parameter `userdata`, you can store an `pkpy::any` object.
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

## Bind a class or struct

Assume you have a struct `Point` declared as follows.

```cpp
struct Point{
    int x;
    int y;
}
```

You can create a `test` module and use `vm->register_user_class<>` to bind the class to the test module.

```cpp
int main(){
    VM* vm = new VM();
    PyVar mod = vm->new_module("test");
    vm->register_user_class<Point>(mod, "Point",
        [](VM* vm, PyVar mod, PyVar type){
            // wrap field x
            vm->bind_field(type, "x", &Point::x);
            // wrap field y
            vm->bind_field(type, "y", &Point::y);

            // __init__ method
            vm->bind(type, "__init__(self, x, y)", [](VM* vm, ArgsView args){
                Point& self = _py_cast<Point&>(vm, args[0]);
                self.x = py_cast<int>(vm, args[1]);
                self.y = py_cast<int>(vm, args[2]);
                return vm->None;
            });
        });

    // use the Point class
    vm->exec("import test");
    vm->exec("a = test.Point(1, 2)");
    vm->exec("print(a.x)");         // 1
    vm->exec("print(a.y)");         // 2

    delete vm;
    return 0;
}
```

### Handle gc for container types

If your custom type stores `PyVar` in its fields, you need to handle gc for them.

```cpp
struct Container{
    PyVar a;
    std::vector<PyVar> b;
    // ...
}
```

Add a magic method `_gc_mark() const` to your custom type.

```cpp
struct Container{
    PyVar a;
    std::vector<PyVar> b;
    // ...

    void _gc_mark() const{
        // mark a
        if(a) PK_OBJ_MARK(a);

        // mark elements in b
        for(PyVar obj : b){
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

## Others

For some magic methods, we provide specialized binding function.
They do not take universal function pointer as argument.
You need to provide the detailed `Type` object and the corresponding function pointer.

```cpp
PyVar f_add(VM* vm, PyVar lhs, PyVar rhs){
    int a = py_cast<int>(vm, lhs);
    int b = py_cast<int>(vm, rhs);
    return py_var(vm, a + b);
}

vm->bind__add__(vm->tp_int, f_add);
```

This specialized binding function has optimizations and result in better performance when calling from python code.

For example, `vm->bind__add__` is preferred over `vm->bind_func(type, "__add__", 2, f_add)`.


## Further reading

See [random.cpp](https://github.com/pocketpy/pocketpy/blob/main/src/random.cpp) for an example used by `random` module.

See [collections.cpp](https://github.com/pocketpy/pocketpy/blob/main/src/collections.cpp) for a modern implementation of `collections.deque`.

## Reuse Lua bindings

pkpy provides a lua bridge to reuse lua bindings.
It allows you to run lua code and call lua functions in python
by embedding a lua virtual machine.

Add `lua_bridge.hpp` and `lua_bridge.cpp` in [3rd/lua_bridge](https://github.com/pocketpy/pocketpy/tree/main/3rd/lua_bridge) to your project.
Make sure `lua.h`, `lualib.h` and `lauxlib.h` are in your include path
because `lua_bridge.hpp` needs them.

The lua bridge is based on lua 5.1.5 for maximum compatibility.
lua 5.2 or higher should also work.

### Setup

Use `initialize_lua_bridge(VM*, lua_State*)` to initialize the lua bridge.
This creates a new module `lua` in your python virtual machine.

You can use `lua.dostring` to execute lua code and get the result.
And use `lua.Table()` to create a lua table.
A `lua.Table` instance in python is a dict-like object which provides a bunch of
magic methods to access the underlying lua table.

```python
class Table:
    def keys(self) -> list:
        """Return a list of keys in the table."""

    def values(self) -> list:
        """Return a list of values in the table."""

    def items(self) -> list[tuple]:
        """Return a list of (key, value) pairs in the table."""

    def __len__(self) -> int:
        """Return the length of the table."""

    def __contains__(self, key) -> bool:
        """Return True if the table contains the key."""

    def __getitem__(self, key): ...
    def __setitem__(self, key, value): ...
    def __delitem__(self, key): ...
    def __getattr__(self, key): ...
    def __setattr__(self, key, value): ...
    def __delattr__(self, key): ...
```

Only basic types can be passed between python and lua.
The following table shows the type mapping.
If you pass an unsupported type, an exception will be raised.

| Python type   | Lua type  | Allow create in Python? | Reference? |
| -----------   | --------  | ---------------------- |  --------- |
| `None`        | `nil`     | YES                    |  NO        |
| `bool`        | `boolean` | YES                    |  NO        |
| `int`         | `number`  | YES                    |  NO        |
| `float`       | `number`  | YES                    |  NO        |
| `str`         | `string`  | YES                    |  NO        |
| `tuple`       | `table`   | YES                    |  NO        |
| `list`        | `table`   | YES                    |  NO        |
| `dict`        | `table`   | YES                    |  NO        |
| `lua.Table`   | `table`   | YES                    |  YES       |
| `lua.Function`| `function`| NO                     |  YES       |

### Example
```cpp
#include "lua_bridge.hpp"

using namespace pkpy;

int main(){
    VM* vm = new VM();

    // create lua state
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // initialize lua bridge
    initialize_lua_bridge(vm, L);

    // dostring to get _G
    vm->exec("import lua");
    vm->exec("g = lua.dostring('return _G')");

    // create a table
    vm->exec("t = lua.Table()");
    vm->exec("t.a = 1");
    vm->exec("t.b = 2");

    // call lua function
    vm->exec("g.print(t.a + t.b)");     // 3
    
    return 0;
}
```