---
title: Error Handling
icon: dot
order: 5
---

#### `bool pkpy_clear_error(pkpy_vm*, char** message)`

+ If a method returns false, call the `pkpy_clear_error` method to check the error and clear it
+ If `pkpy_clear_error` returns false, it means that no error was set, and it takes no action
+ If `pkpy_clear_error` returns true, it means there was an error and it was cleared. It will provide a string summary of the error in the message parameter if it is not `NULL`.

!!!
You are responsible for freeing `message`.
!!!

#### `bool pkpy_check_error(pkpy_vm*)`

Return true if the vm is currently in an error state.

#### `bool pkpy_error(pkpy_vm*, const char* name, pkpy_CString message)`

Set the error state of the vm. It is almost equivalent to `raise` in python.
