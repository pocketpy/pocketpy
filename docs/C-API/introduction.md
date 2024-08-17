---
title: Introduction
icon: dot
order: 10
---

TBA


### `PY_RAISE` macro

Mark a function that can raise an exception on failure.

+ If the function returns `bool`, then `false` means an exception is raised.
+ If the function returns `int`, then `-1` means an exception is raised.

### `PY_RETURN` macro

Mark a function that can store a value in `py_retval()` on success.