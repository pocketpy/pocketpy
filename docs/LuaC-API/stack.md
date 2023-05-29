---
title: Stack Manipulation
icon: dot
order: 8
---

!!!
Stack index is 0-based instead of 1-based.
!!!

## Push and Pop

#### `bool pkpy_pop(pkpy_vm*, int n)`

Pop `n` items from the stack.

#### `bool pkpy_push_function(pkpy_vm*, pkpy_function, int argc)`

Push a function onto the stack. The function is of `typedef int (*pkpy_function)(pkpy_vm*);`

#### `bool pkpy_push_int(pkpy_vm*, int)`

Push an integer onto the stack.

#### `bool pkpy_push_float(pkpy_vm*, double)`

Push a float onto the stack.

#### `bool pkpy_push_bool(pkpy_vm*, bool)`

Push a boolean onto the stack.

#### `bool pkpy_push_string(pkpy_vm*, const char*)`

Push a string onto the stack.

#### `bool pkpy_push_stringn(pkpy_vm*, const char*, int length)`

Push a string onto the stack.

+ `length`: the length of the string

#### `bool pkpy_push_voidp(pkpy_vm*, void*)`

Push a void pointer onto the stack.

#### `bool pkpy_push_none(pkpy_vm*)`

Push `None` onto the stack.

## Size Queries

#### `bool pkpy_check_stack(pkpy_vm*, int free)`

Return true if at least `free` empty slots remain on the stack.

#### `int pkpy_stack_size(pkpy_vm*)`

Return the number of elements on the stack.

## Conversion

#### `bool pkpy_to_int(pkpy_vm*, int index, int* ret)`

Convert the value at the given index to an integer.

#### `bool pkpy_to_float(pkpy_vm*, int index, double* ret)`

Convert the value at the given index to a float.

#### `bool pkpy_to_bool(pkpy_vm*, int index, bool* ret)`

Convert the value at the given index to a boolean.

#### `bool pkpy_to_voidp(pkpy_vm*, int index, void** ret)`

Convert the value at the given index to a void pointer.

#### `bool pkpy_to_string(pkpy_vm*, int index, char** ret)`

Convert the value at the given index to a string (strong reference).

+ `ret` is null terminated.
+ You are responsible for freeing the string when you are done with it.

#### `bool pkpy_to_stringn(pkpy_vm*, int index, const char** ret, int* size)`

Convert the value at the given index to a string (weak reference).

+ `ret` is not null terminated.
+ `size` is the length of the string.
+ The string is only valid until the next API call.

