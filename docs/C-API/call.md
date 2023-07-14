---
title: Call
icon: dot
order: 6
---

### `bool pkpy_vectorcall(pkpy_vm*, int argc)`

Wraps `vm->vectorcall(argc)`. This function is used to call a function with a fixed number of arguments. The arguments are popped from the stack. The return value is pushed onto the stack.

1. First push the function to call.
2. Push `self` argument if it is a method call. Otherwise, call `pkpy_push_null`.
3. Push arguments from left to right.

!!!
Unlike lua, a python function always returns a value.
If the function returns `void`, it will push `None` onto the stack.
You can call `pkpy_pop_top` to discard the return value.
!!!