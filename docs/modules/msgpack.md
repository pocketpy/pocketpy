---
icon: package
label: msgpack
---

# msgpack

Encode simple values in the MessagePack binary format.

!!!info Optional module
Enable `PK_BUILD_MODULE_MSGPACK=ON`; see [build configuration](../build.md).
!!!

| Function | Result |
| --- | --- |
| `dumps(obj)` | Encoded `bytes`. |
| `loads(data: bytes)` | Decoded Python value. |

```python
import msgpack

state = {'level': 2, 'items': ['key', 'coin'], 'payload': b'\x00\x01'}
packed = msgpack.dumps(state)
assert msgpack.loads(packed) == state
```

The built-in conversion supports `None`, booleans, signed 64-bit integers,
floats, strings, bytes, lists, and dictionaries. Use string or integer map keys.
Arrays decode to lists; tuples and arbitrary class instances are not part of
the default encoder.

Keep integer values within pocketpy's signed 64-bit range, including when
receiving MessagePack unsigned integers. Extension values require host-provided
conversion hooks; they are not automatically mapped to Python classes.
The API has no CPython third-party `msgpack` keyword option set.

Implementation:
[bindings.c](https://github.com/pocketpy/pocketpy/blob/main/3rd/msgpack/src/bindings.c).
