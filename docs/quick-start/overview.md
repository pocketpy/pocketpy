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