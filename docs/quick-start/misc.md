---
icon: dot
label: 'Misc'
order: 0
---

## The scope lock of gc

Sometimes you need to use the following code to prevent the gc from collecting objects.

```cpp
auto _lock = vm->heap.gc_scope_lock()
```

The scope lock is required if you create a PyObject and then try to run python-level bytecodes.

For example, you create a temporary object on the stack and then call `vm->py_str`.

Because users can have an overload of `__str__`, the call process is unsafe.

When the vm is running python-level bytecodes, gc may start and delete your temporary object.

The scope lock prevents this from happening.
