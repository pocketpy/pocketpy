---
icon: dot
label: 'Miscellaneous'
order: 0
---

## The scope lock of gc

Sometimes you need to use the following code to prevent the gc from collecting objects.

```cpp
auto _lock = vm->heap.gc_scope_lock();
```

The scope lock is required if you create a PyVar and then try to run python-level bytecodes.

For example, you create a temporary object on the stack and then call `vm->py_next`.

```cpp
void some_func(VM* vm){
    PyVar obj = VAR(List(5));
    // unsafe
    PyVar iter = vm->py_iter(obj);
    PyVar next = vm->py_next(iter);
}
```

Because users can have an overload of `__next__`, this call is unsafe.
When the vm is running python-level bytecodes, gc may start and delete your temporary object.

The scope lock prevents this from happening.

```cpp
void some_func(VM* vm){
    PyVar obj = VAR(List(5));
    // safe
    auto _lock = vm->heap.gc_scope_lock();
    PyVar iter = vm->py_iter(obj);
    PyVar next = vm->py_next(iter);
}
```
