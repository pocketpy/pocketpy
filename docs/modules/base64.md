---
icon: package
label: base64
---

# base64

Convert binary data to and from the standard Base64 alphabet.

| Function | Input and output |
| --- | --- |
| `b64encode(data)` | `bytes` to encoded `bytes`. |
| `b64decode(data)` | Encoded `str` or `bytes` to decoded `bytes`. |

```python
import base64

encoded = base64.b64encode(b'hello')
assert encoded == b'aGVsbG8='
assert base64.b64decode(encoded) == b'hello'
assert base64.b64decode('aGVsbG8=') == b'hello'
```

Only these two functions are provided. There are no URL-safe alphabet,
alternate-alphabet, or `validate=` options. Malformed input can return empty
bytes instead of raising an error, so do not use the decoder as an input
validator.

Implementation: [base64.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/base64.c).
