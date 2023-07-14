---
title: Introduction
icon: dot
order: 10
---

We take a lot of inspiration from the lua api for these bindings.
The key difference being most methods return a bool, 
true if it succeeded false if it did not.

!!!
Special thanks for [@koltenpearson](https://github.com/koltenpearson) for bringing us the Lua Style API implementation.
!!!

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