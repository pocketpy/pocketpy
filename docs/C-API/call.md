---
title: Call
icon: dot
order: 6
---

### `bool pkpy_vectorcall(pkpy_vm*, int argc)`

Wraps `vm->vectorcall(argc)`.

This is the only way to call a function in the C-APIs.

1. First push the function to call.
2. Push `self` argument if it is a method call. Otherwise, call `pkpy_push_null`.
3. Push arguments from left to right.
