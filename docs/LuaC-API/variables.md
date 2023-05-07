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

