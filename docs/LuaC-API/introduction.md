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

#### `pkpy_vm* pkpy_vm_create(bool use_stdio, bool enable_os)`

Creates a new Lua Style VM.

+ `use_stdio`: if true, the VM will use stdout and stderr
+ `enable_os`: if true, the VM will have access to the os library

#### `bool pkpy_vm_run(pkpy_vm*, const char* source)`

Runs the given source code in the VM.

+ `source`: the source code to run

#### `void pkpy_vm_destroy(pkpy_vm*)`

Destroys the VM.

