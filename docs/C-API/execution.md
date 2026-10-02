---
title: Execution and Exceptions
icon: dot
order: 9
---

# Execution and exceptions

## Execute statements or evaluate an expression

| API | Input | Result on success |
| --- | --- | --- |
| `py_exec(source, filename, EXEC_MODE, module)` | Statements | `None` in `py_retval()` |
| `py_exec(source, filename, EVAL_MODE, module)` | One expression | Expression value in `py_retval()` |
| `py_eval(source, module)` | One expression | Shorthand using filename `<string>` |
| `py_execo(data, size, filename, module)` | pocketpy bytecode | Result in `py_retval()`; see [deployment](../features/deploy.md) |

Use a module reference to select the global namespace, or `NULL` for
`__main__`. The filename is used for diagnostics; it does not change the
working directory or the import search root.

For a C value, prefer constructing a Python value to interpolating it into
Python source. This avoids quoting problems and unnecessary parsing.

## Call a Python function from C

This complete program defines a Python function and calls it with two integers:

```c
#include "pocketpy.h"
#include <inttypes.h>
#include <stdio.h>

int main(void) {
    py_initialize();
    py_StackRef base = py_peek(0);

    if(!py_exec("def add(a, b):\n    return a + b\n",
                "app.py", EXEC_MODE, NULL)) goto error;

    py_Ref add = py_getglobal(py_name("add"));
    if(add == NULL) {
        py_exception(tp_NameError, "add is not defined");
        goto error;
    }

    py_push(add);
    py_pushnil();                         /* No bound self argument. */
    py_newint(py_pushtmp(), 20);
    py_newint(py_pushtmp(), 22);
    if(!py_vectorcall(2, 0)) goto error;
    if(!py_checkint(py_retval())) goto error;

    printf("%" PRId64 "\n", py_toint(py_retval()));  /* 42 */
    py_finalize();
    return 0;

error:
    py_printexc();
    py_clearexc(base);
    py_finalize();
    return 1;
}
```

The call stack layout is:

```text
callable, self-or-nil, positional arguments..., keyword-name, keyword-value...
```

`py_vectorcall(argc, kwargc)` counts positional arguments excluding `self`,
and counts keyword **pairs**. Push keyword names with `py_pushname()`.
On success, the call consumes this layout and leaves the result in
`py_retval()`. Do not pop those call arguments again.

When arguments already occupy consecutive slots, `py_call(f, argc, argv)`
prepares the call for you. Keep the callable and argument values rooted during
the call; see [reference lifetimes](introduction.md#a-reference-points-to-a-value-slot).

## Handle an exception at the host boundary

An exception is stored in the VM until handled or cleared.
`py_printexc()` prints it but **does not clear it**. If the host will continue
running Python, save the stack boundary before the operation and restore it with
`py_clearexc(base)` on failure.

The following fragment runs inside an initialized host:

```c
py_StackRef base = py_peek(0);
if(!py_exec("raise ValueError('invalid level')", "level.py", EXEC_MODE, NULL)) {
    py_printexc();
    py_clearexc(base);
}

bool ok = py_exec("print('Host recovered')", "app.py", EXEC_MODE, NULL);
if(!ok) {
    py_printexc();
    py_clearexc(base);
}
```

For a log string, use `py_formatexc()` and release the returned allocation with
`py_free()`. A `NULL` result means no exception is set.

A native callback normally propagates failures with `return false;`.
Only clear the exception if your code actually handles the failure and can
continue. Never return `false` without setting an exception, or return
`true` while an exception is pending.

## Return values are temporary

Any operation marked `PY_RETURN` may overwrite `py_retval()`. Copy a result
that must survive another call:

```c
/* After a successful operation that returned a Python value: */
py_push(py_retval());
py_StackRef saved = py_peek(-1);
/* Other calls may now overwrite py_retval(); saved remains rooted. */
py_setglobal(py_name("last_result"), saved);
py_pop();
```

For long-lived application data, store the value in a module or a reachable
Python container rather than leaving temporary stack entries indefinitely.
