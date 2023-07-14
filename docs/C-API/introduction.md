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

## Basic Functions

#### `pkpy_vm* pkpy_new_vm(bool enable_os)`

Create a new VM.

+ `enable_os`: if true, the VM will have access to the os library

#### `bool pkpy_vm_run(pkpy_vm* vm_handle, const char* source)`

Run the given source code in the VM.

+ `source`: the source code to run

#### `void pkpy_delete_vm(pkpy_vm* vm_handle)`

Dispose the VM.

#### `bool pkpy_vm_exec(pkpy_vm* vm_handle, const char* source)`

A wrapper of `vm->exec(...)`.

#### `bool pkpy_vm_exec_2(pkpy_vm* vm_handle, const char* source, const char* filename, int mode, const char* module)`

A wrapper of `vm->exec_2(...)`.