---
icon: package
label: json
---

# json

Serialize simple data as JSON text.

!!!danger Trusted input only
`json.loads()` evaluates its input as a Python expression in the `json`
module. It can execute code and is not a strict JSON parser. Do not pass
untrusted input to it.
!!!

| Function | Behavior |
| --- | --- |
| `loads(text: str)` | Evaluate text and return a Python value. JSON names `null`, `true`, and `false` are available. |
| `dumps(obj, indent=0)` | Return a string; a positive integer indent enables multiline formatting. |

Supported output values are `None`, booleans, integers, floats, strings, lists,
tuples, and dictionaries with **string keys**. Tuples become JSON arrays and
load as lists. Unsupported values raise `TypeError`. Keep containers acyclic.

```python
import json

settings = {'name': 'Ada', 'lives': 3, 'sound': True}
text = json.dumps(settings, indent=2)
assert json.loads(text) == settings
assert json.loads('{"value": null}') == {'value': None}
```

This API does not provide file-based `load`/`dump`, custom encoders,
`object_hook`, `sort_keys`, or the full CPython option set.
Non-finite floats are emitted as `NaN` or `Infinity`, which are outside
strict JSON.

For external data, bind a dedicated JSON parser in the host. A host may replace
`json.loads` and `json.dumps` using [C binding helpers](../bindings.md);
preserve the intended signatures and error reporting.

Implementation: [json.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/json.c).
