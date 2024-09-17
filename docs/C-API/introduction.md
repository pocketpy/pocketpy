---
title: Introduction
icon: dot
order: 10
---

All public functions in the C API are prefixed with `py_` in [pocketpy.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/pocketpy.h).


### `PY_RAISE` macro

Mark a function that can raise an exception on failure.

+ If the function returns `bool`, then `false` means an exception is raised.
+ If the function returns `int`, then `-1` means an exception is raised.

### `PY_RETURN` macro

Mark a function that can store a value in `py_retval()` on success.