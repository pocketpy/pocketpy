---
icon: package
label: sys
---

# sys

Interpreter identification, host-supplied arguments, and recursion limits.

| Member | Meaning |
| --- | --- |
| `version` | pocketpy's version string, such as `'2.2.0'`; not a CPython version. |
| `platform` | Target platform identifier, such as `win32`, `linux`, `darwin`, `android`, `ios`, or `emscripten`. |
| `argv` | Argument list supplied by the host. |
| `getrecursionlimit()` | Current Python call-depth limit. |
| `setrecursionlimit(limit)` | Change the limit; values at or below the current depth raise `ValueError`. |

```python
import sys

print(sys.version, sys.platform)
print(sys.argv)
original = sys.getrecursionlimit()
sys.setrecursionlimit(original + 100)
assert sys.getrecursionlimit() == original + 100
sys.setrecursionlimit(original)
```

The initial recursion limit is 1000. Raising it increases the depth the runtime
allows; it does not increase the native stack or VM value-stack capacity.

In an embedded VM, `argv` starts empty unless the host sets it.
The standalone executable passes command-line entries after its executable
name through `py_sys_setargv()`, including interpreter flags.
Do not assume CPython's argument filtering.

This module does not provide `sys.path`, `sys.modules`, or stream objects
such as `sys.stdout`. Use [host callbacks](../C-API/modules.md) to customize
imports and output.
