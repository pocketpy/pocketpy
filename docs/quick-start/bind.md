---
icon: dot
label: 'Bind native function'
order: 60
---

In `VM` class, there are 4 methods to bind native function.

+ `VM::bind_func<ARGC>`
+ `VM::bind_builtin_func<ARGC>`
+ `VM::bind_method<ARGC>`
+ `VM::bind_static_method<ARGC>`

They are all template methods, the template argument is a `int` number, indicating the argument count. For variadic arguments, use `-1`. For methods, `ARGC` do not include `self`.

!!!

Native functions do not support keyword arguments.

!!!

pkpy uses a universal C function pointer for native functions:

```cpp
typedef PyObject* (*NativeFuncC)(VM*, ArgsView);
```

The first argument is the pointer of `VM` instance.

The second argument is a view of an array. You can use `[]` operator to get the element. If you have specified `ARGC` other than `-1`, the interpreter will ensure `args.size() == ARGC`. No need to do size check.

The return value is a `PyObject*`, which should not be `nullptr`. If there is no return value, return `vm->None`.

This is an example of binding the `input()` function to the `builtins` module.

```cpp
VM* vm = pkpy_new_vm();
vm->bind_builtin_func<0>("input", [](VM* vm, ArgsView args){
    static std::string line;
    std::getline(std::cin, line);
    return VAR(line);
});

//                        vvv function name
vm->bind_builtin_func<2>("add", [](VM* vm, ArgsView args){
//                    ^ argument count
	i64 lhs = CAST(i64, args[0]);
    i64 rhs = CAST(i64, args[1]);
    return VAR(lhs + rhs);
});
```