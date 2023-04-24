---
icon: code
label: 'Overview'
order: 95
---

pkpy's C++ interfaces are organized in an object-oriented way.
All classes are located in `pkpy` namespace.

The most important class is the `VM` class. A `VM` instance is a python virtual machine which holds all necessary runtime states, including callstacks, modules, variables, etc.

You need to use the C++ `new` operator to create a `VM` instance.

```cpp
VM* vm = new VM();
```

The constructor can take 2 extra parameters.

#### `VM(bool use_stdio=true, bool enable_os=true)`

+ `use_stdio`, if `true`, the `print()` function outputs string to `stdout`. Error messages will be send to `stderr`; If `false`, they will be sent to an internal buffer. In the latter case, you need to read them via `read_output` manually.
+ `enable_os`, whether to enable OS-related features or not. This setting controls the availability of some priviledged modules such os `io` and `os` as well as builtin function `open`.

