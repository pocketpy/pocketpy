---
title: VM
icon: dot
order: 10
---
#### `VM* pkpy_new_vm(bool use_stdio)`

Create a virtual machine.

#### `void pkpy_vm_add_module(VM* vm, const char* name, const char* source)`

Add a source module into a virtual machine.

#### `char* pkpy_vm_eval(VM* vm, const char* source)`

Evaluate an expression.

Return `__repr__` of the result.
If there is any error, return `nullptr`.

#### `void pkpy_vm_exec(VM* vm, const char* source)`

Run a given source on a virtual machine.

#### `char* pkpy_vm_get_global(VM* vm, const char* name)`

Get a global variable of a virtual machine.

Return `__repr__` of the result.
If the variable is not found, return `nullptr`.

#### `char* pkpy_vm_read_output(VM* vm)`

Read the standard output and standard error as string of a virtual machine.
The `vm->use_stdio` should be `false`.
After this operation, both stream will be cleared.

Return a json representing the result.