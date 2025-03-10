---
icon: package
label: sys
---

### `sys.version`

The version of pkpy.

### `sys.platform`

May be one of:
+ `win32`
+ `linux`
+ `darwin`
+ `android`
+ `ios`
+ `emscripten`

### `sys.argv`

The command line arguments. Set by `py_sys_setargv`.

### `sys.setrecursionlimit(limit: int)`

Set the maximum depth of the Python interpreter stack to `limit`. This limit prevents infinite recursion from causing an overflow of the C stack and crashing the interpreter.

### `sys.getrecursionlimit() -> int`

Return the current value of the recursion limit.
