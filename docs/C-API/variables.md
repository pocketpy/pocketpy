---
title: Variables
icon: dot
order: 6
---


#### `bool pkpy_check_global(pkpy_vm*, const char* name)`

Return true if the global variable exists.

#### `bool pkpy_set_global(pkpy_vm*, const char* name)`

Set the global variable to the value at the top of the stack.

#### `bool pkpy_get_global(pkpy_vm*, const char* name)`

Get the global variable and push it to the top of the stack.

#### `bool pkpy_getattr(pkpy_vm*, const char* name)`

A wrapper of `OP_LOAD_ATTR` bytecode.

#### `bool pkpy_setattr(pkpy_vm*, const char* name)`

A wrapper of `OP_STORE_ATTR` bytecode.

#### `bool pkpy_eval(pkpy_vm*, const char* code)`

Evaluate the code and push the result to the top of the stack.