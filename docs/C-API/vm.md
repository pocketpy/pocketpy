---
title: VM
icon: dot
order: 10
---

#### `VM* pkpy_new_vm()`

Create a virtual machine.

#### `void pkpy_vm_add_module(VM* vm, const char* name, const char* source)`

Add a source module into a virtual machine.

#### `void pkpy_vm_exec(VM* vm, const char* source)`

Run a given source on a virtual machine.

#### `void pkpy_vm_exec_2(pkpy::VM* vm, const char* source, const char* filename, int mode, const char* module)`

Advanced version of `pkpy_vm_exec`.

#### `void pkpy_delete(void* p)`

Delete a pointer allocated by `pkpy_xxx_xxx`.
It can be `VM*`, `REPL*`, `char*`, etc.

!!!
If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
!!!
