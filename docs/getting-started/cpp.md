---
icon: code
label: 'C++'
order: 10
---

For C++ developers, you can download the `pocketpy.h` on our GitHub release page.

https://github.com/blueloveTH/pocketpy/releases/latest

## Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM(true);
    
    // Hello world!
    vm->exec("print('Hello world!')", "main.py", EXEC_MODE);

    // Create a list
    vm->exec("a = [1, 2, 3]", "main.py", EXEC_MODE);

    // Eval the sum of the list
    PyObject* result = vm->exec("sum(a)", "<eval>", EVAL_MODE);
    std::cout << py_cast<i64>(vm, result);   // 6
    return 0;
}
```

## Interop with `PyObject*`

In PkPy, any python object is represented by a `PyObject*`.

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

#### Types

| `PyObject` type | C type           | note                   |
| ------------ | ---------------- | ---------------------- |
| `int`        | `i64`            | 62 bits integer |
| `float`      | `f64`            | 62 bits floating point |
| `str`        | `pkpy::Str`      |                        |
| `bool`       | `bool`           |                        |
| `list`       | `pkpy::List`     |                        |
| `tuple`      | `pkpy::Tuple`    |                        |
| `function`   | `pkpy::Function` |                        |
| ...          | ...              | ...                    |

## Bind a Native Function

In `VM` class, there are 4 methods to bind native function.

+ `VM::bind_func<ARGC>`
+ `VM::bind_builtin_func<ARGC>`
+ `VM::bind_method<ARGC>`
+ `VM::bind_static_method<ARGC>`

They are all template methods, the template argument is a `int` number, indicating the argument count. For variadic arguments, use `-1`. For methods, `ARGC` do not include `self`.

!!!

Native functions do not support keyword arguments.

!!!

PkPy uses a universal C function pointer for native functions:

```cpp
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
```

The first argument is the pointer of `VM` instance.

The second argument is a view of an array. You can use `[]` operator to get the element. If you have specified `ARGC` other than `-1`, the interpreter will ensure `args.size() == ARGC`. No need to do size check.

The return value is a `PyObject*`, which should not be `nullptr`. If there is no return value, return `vm->None`.

This is an example of binding the `input()` function to the `builtins` module.

```cpp
VM* vm = pkpy_new_vm(true);
vm->bind_builtin_func<0>("input", [](VM* vm, ArgsView args){
    static std::string line;
    std::getline(std::cin, line);
    return VAR(line);
});

//                        vvv function name
vm->bind_builtin_func<2>("add", [](VM* vm, ArgsView args){
    //                ^ argument count
	i64 lhs = CAST(i64, args[0]);
    i64 rhs = CAST(i64, args[1]);
    return VAR(lhs + rhs);
});
```

## Call a Python Function

Use these to call a python function.

+ `PyObject* VM::call(PyObject* obj, ...)`
+ `PyObject* VM::call_method(PyObject* obj, StrName name, ...)`

For example, to create a `dict` object,

```cpp
PyObject* tp = vm->builtins->attr("dict");
PyObject* obj = vm->call(tp);	// this is a `dict`
```

And set a key-value pair,

```cpp
vm->call_method(obj, "__setitem__", VAR("a"), VAR(5));
PyObject* ret = vm->call(obj, "__getitem__", VAR("a"));
std::cout << CAST(int, ret) << std::endl; // 5
```

## Attribute Access

There are 3 methods for this.

+ `PyObject* VM::getattr(PyObject*, StrName)`
+ `void VM::setattr(PyObject*, StrName, PyObject*)`
+ `PyObject* VM::get_unbound_method(PyObject*, StrName, PyObject**)`

## Wrapping a `struct` as `PyObject`

!!!

This feature is unstable.

!!!
