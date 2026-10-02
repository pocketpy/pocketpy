---
icon: cpu
title: C Bindings
order: 17
---

# C bindings

A binding converts Python arguments to native values, calls your C code, and
writes a Python result. All native functions use the `py_CFunction` signature:

```c
typedef bool (*py_CFunction)(int argc, py_StackRef argv);
```

`argc` is the argument count and `py_arg(i)` accesses argument `i`.
The parameter names `argc` and `argv` are required by the convenience macros.

## A complete binding

This program exports `native.add(a, b=1)`:

```c
#include "pocketpy.h"
#include <stdint.h>

static bool native_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);

    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    if((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) {
        return ValueError("sum is outside the signed 64-bit range");
    }

    py_newint(py_retval(), a + b);
    return true;
}

int main(void) {
    py_initialize();

    py_GlobalRef mod = py_newmodule("native");
    py_bind(mod, "add(a, b=1)", native_add);

    bool ok = py_exec("from native import add\n"
                      "assert add(3, 7) == 10\n"
                      "assert add(4) == 5\n"
                      "assert add(4, b=2) == 6\n",
                      "app.py", EXEC_MODE, NULL);
    if(!ok) py_printexc();

    py_finalize();
    return ok ? 0 : 1;
}
```

The signature passed to `py_bind()` resolves defaults before the callback
runs, so the callback receives two arguments even for `add(4)`.
Required parameters such as `a` are positional in pocketpy; see
[argument compatibility](features/differences.md#function-arguments).

Use `py_i64` for Python integers and validate the native library's accepted
range before narrowing to `int` or another smaller type. The example checks
addition overflow to avoid undefined C arithmetic.

## Choose a binding helper

| Helper | Use |
| --- | --- |
| `py_bind(obj, "name(args...)", callback)` | Signature-based binding with names and defaults. |
| `py_bindfunc(module, "name", callback)` | Simple positional-argument function; the callback validates the count. |
| `py_bindmethod(type, "name", callback)` | Instance method; `self` is argument 0 and included in `argc`. |
| `py_bindstaticmethod(type, "name", callback)` | Method without an implicit `self`. |
| `py_bindproperty(type, "name", getter, setter)` | Property; a `NULL` setter makes it read-only. |
| `py_bindmagic(type, py_name("__len__"), callback)` | Special method on a native type. |

For a global function, bind to `py_getmodule("__main__")`. For a reusable API,
create a named module.

## Results and errors

On success, write a value to `py_retval()` and return `true`. A function with
no meaningful result must still call `py_newnone(py_retval())`.

On failure, set an exception and return `false`. For example:

```c
static bool require_positive(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    if(py_toint(py_arg(0)) <= 0) return ValueError("value must be positive");
    py_newnone(py_retval());
    return true;
}
```

Type-check macros return immediately when validation fails. If an API you call
fails, propagate its error rather than replacing it with a generic exception.
See [execution and exceptions](C-API/execution.md) for the host's recovery path.

## Native object lifetimes

`py_newtype()` registers a Python type. `py_newobject()` allocates an instance
with native userdata and optional Python slots or an attribute dictionary.

Store references to Python objects in Python slots or another GC-visible
container. A raw pointer inside userdata is not automatically a GC root.
Keep a native resource alive for as long as a bound object can access it, and
use the type's destructor callback to release native allocations.

For class-heavy C++ libraries, the [C++ binding layer](bindings-cpp.md) handles
much of this plumbing. The [function reference](C-API/functions.md) documents
the lower-level type, slot, and binding operations.
