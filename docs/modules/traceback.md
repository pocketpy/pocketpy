---
icon: package
label: traceback
---

# traceback

Report the exception currently being handled by a Python `except` block.

| Function | Behavior |
| --- | --- |
| `print_exc()` | Print the active exception and traceback; return `None`. |
| `format_exc()` | Return the traceback string, or `None` when no exception is being handled. |

```python
import traceback

def describe_error():
    return traceback.format_exc()

try:
    raise ValueError('invalid level')
except ValueError:
    message = describe_error()
    assert 'ValueError: invalid level' in message
    print(message)

assert traceback.format_exc() is None
```

Both functions work in helper functions called from an exception handler and
inside nested `try` blocks. Outside a handler, `print_exc()` prints nothing.
There are no `limit`, `file`, or `chain` parameters.

For failures returned to C rather than handled by Python, use
`py_printexc()` or `py_formatexc()`; see
[host exception handling](../C-API/execution.md).
