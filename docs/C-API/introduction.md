---
title: Introduction
icon: dot
order: 10
---

### What C-API is for

The C-APIs are designed for these purposes:

1. Your target platform does not support C++17. You compile pkpy into a static library and use its exported C-APIs.
2. You want to write a native module that can be imported via `__import__` at runtime. By using C-APIs, the module is portable across different compilers without C++ ABI compatibility issues.

Our C-APIs take a lot of inspiration from the lua C-APIs.
Methods return a `bool` indicating if the operation succeeded or not.
Special thanks for [@koltenpearson](https://github.com/koltenpearson)'s contribution.

!!!
C-APIs are always stable and backward compatible.
!!!

### Basic functions

+ `pkpy_vm* pkpy_new_vm(bool enable_os)`

    Wraps `new VM(enable_os)` in C++.

+ `void pkpy_delete_vm(pkpy_vm*)`

    Wraps `delete vm` in C++.

+ `bool pkpy_exec(pkpy_vm*, const char* source)`

    Wraps `vm->exec`. Execute a string of source code.

+ `bool pkpy_exec_2(pkpy_vm*, const char* source, const char* filename, int mode, const char* module)`

    Wraps `vm->exec_2`. Execute a string of source code with more options.

+ `void pkpy_set_main_argv(pkpy_vm*, int argc, char** argv)`

    Wraps `vm->set_main_argv`. Set the `sys.argv` before executing scripts.
