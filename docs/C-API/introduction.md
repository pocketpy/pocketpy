---
title: C API Essentials
icon: dot
order: 10
---

# C API essentials

Include `pocketpy.h` and initialize the runtime before calling the API.
The [quick start](../quick-start.md) has a complete host program.
Public declarations live in
[pocketpy.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/pocketpy.h);
the [function reference](functions.md) is generated from that header.

Read [execution and exceptions](execution.md) for calls and error handling,
[module loading](modules.md) for imports, and [C bindings](../bindings.md) for
native functions.

## A reference points to a value slot

A `py_Ref` points to a slot containing a Python value. Copying the C pointer
does **not** copy the value or keep its object alive. Use `py_assign(dst, src)`
to copy a value into another valid slot. pocketpy uses garbage collection, so
there is no public `Py_INCREF`/`Py_DECREF` protocol.

The reference typedefs have the same C representation but document different
lifetimes:

| Type | Meaning and lifetime |
| --- | --- |
| `py_Ref` | Generic reference; the function returning it determines its lifetime. |
| `py_GlobalRef` | VM-owned reference; invalid after VM reset or finalization. |
| `py_StackRef` | Value-stack slot; valid while that slot remains on the stack. |
| `py_ObjectRef` | Slot owned by an object; the owner must remain alive. |
| `py_ItemRef` | Container item; reacquire it after modifying the container. |
| `py_OutRef` | A caller-provided destination slot for an output value. |

A local C variable containing a reference is not a garbage-collector root.
Keep values in a VM register, on the value stack, or in a reachable Python
container or module for as long as they are needed.

## Registers and stack

| Storage | Use |
| --- | --- |
| `py_r0()` ... `py_r7()` | Eight application registers. Shared with other native code using the same VM. |
| `py_tmpr0()` ... `py_tmpr3()` | Scratch registers also used internally; do not keep values here across API calls. |
| `py_retval()` | Return slot, overwritten by operations producing a result. |
| `py_sysr0()`, `py_sysr1()` | Reserved for the debugger and C++ binding layer. |
| `py_push()`, `py_pushtmp()` | Stack storage for values that must survive nested calls. |

`py_peek(-1)` addresses the top value. `py_peek(0)` is the stack boundary,
useful as a saved position for error recovery; it is not an initialized value.

This fragment runs after initialization and creates a rooted list:

```c
py_StackRef values = py_pushtmp();
py_newlistn(values, 3);
py_newint(py_list_getitem(values, 0), 10);
py_newint(py_list_getitem(values, 1), 20);
py_newint(py_list_getitem(values, 2), 30);

py_setglobal(py_name("scores"), values);
py_pop();  /* The __main__.scores global now keeps the list alive. */
```

Initialize a slot returned by `py_pushtmp()` immediately. On successful return
from a native function, pop every temporary you pushed. On failure, propagate
the exception to the caller; the host's recovery boundary restores the stack.
See [exception handling](execution.md#handle-an-exception-at-the-host-boundary).

## Convert values deliberately

| Python value | Create from C | Read in C |
| --- | --- | --- |
| `int` (signed 64-bit) | `py_newint(out, value)` | `py_toint(ref)` returning `py_i64` |
| `float` (double) | `py_newfloat(out, value)` | `py_tofloat(ref)` |
| `bool` | `py_newbool(out, value)` | `py_tobool(ref)` |
| UTF-8 `str` | `py_newstr(out, text)`, `py_newstrv(out, view)` | `py_tostr(ref)`, `py_tosv(ref)` |
| `bytes` | `py_newbytes(out, size)`, then fill the returned buffer | `py_tobytes(ref, &size)` |
| `None` | `py_newnone(out)` | `py_isnone(ref)` |

The `py_to*` functions are low-level accessors, not general Python conversions.
Check types first with `py_checktype()` or the `PY_CHECK_ARG_TYPE` macro.
For a numeric input accepting either `int` or `float`, use
`py_castfloat(ref, &value)` and check its boolean result. Range-check before
narrowing a `py_i64` to a smaller C integer.

String and byte pointers refer to interpreter-owned memory. Copy the data if it
must outlive the owning object. Use `c11_sv` or an explicit byte length for data
that may contain embedded NUL characters.

## API annotations

### `PY_RAISE` macro

Marks an operation that can set a Python exception. A boolean return of `false`
indicates failure; for integer-returning operations, `-1` indicates failure.
Other integer values have function-specific meanings: for example,
`py_import()` returns `0` for "not found" and `1` for success.

### `PY_RETURN` macro

Marks an operation whose successful result is written to `py_retval()`.
Read or copy it before another operation overwrites it.

### `PY_MAYBENULL` macro

Marks a pointer or callback that may be `NULL`. Check the function's contract
before dereferencing it.

These macros document contracts; they do not perform checks themselves.

## Runtime lifetime

`py_initialize()` creates VM 0. Up to 16 VM slots exist, selected with
`py_switchvm(index)`. A value belongs to its VM and must not be passed to
another VM. Each VM can be used by only one thread at a time; see
[compute threads](../features/threading.md).

`py_resetvm()` discards the current VM's state, invalidating its references.
`py_finalize()` destroys all VMs and is terminal for the process: the API must
not be used afterward.
