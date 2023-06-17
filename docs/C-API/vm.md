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

#### `void pkpy_free(void* p)`

Free a pointer via `free`.

#### `void pkpy_delete_vm(VM* vm)`

Delete a virtual machine.

#### `void pkpy_delete_repl(REPL* repl)`

Delete a REPL.

#### `void pkpy_vm_compile(VM* vm, const char* source, const char* filename, int mode, bool* ok, char** res)`

Compile a source into bytecode and serialize it into a string.

+ `ok`: whether the compilation is successful.
+ `res`: if `ok` is true, `res` is the bytecode string, otherwise it is the error message.