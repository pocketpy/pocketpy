---
icon: package
label: pickle
---

# pickle

Save Python object graphs as bytes and restore them within pocketpy.

| Function | Result |
| --- | --- |
| `dumps(obj)` | Serialized `bytes`. |
| `loads(data: bytes)` | Restored object. |

!!!warning
Only load data you trust. Deserialization can resolve classes and call
reconstruction functions. This is pocketpy's own format, not CPython's pickle
protocol; use a matching runtime when exchanging saved data.
!!!

## Shared references and cycles

```python
import pickle

shared = ['coin']
inventory = [shared, shared]
restored = pickle.loads(pickle.dumps(inventory))
assert restored == [['coin'], ['coin']]
assert restored[0] is restored[1]

cycle = []
cycle.append(cycle)
restored_cycle = pickle.loads(pickle.dumps(cycle))
assert restored_cycle[0] is restored_cycle
```

## Supported objects

- `None`, booleans, integers, floats, strings, and bytes.
- Tuples, lists, sets, and dictionaries containing supported values.
- User-defined functions and classes accessible by name at module scope.
- Instances of such classes, and native types with supported reconstruction
  behavior, such as [random.Random](random.md).

Functions and classes are resolved by their module/name; their source code is
not packed into the data. Make those definitions available when loading,
especially in a [worker VM](../features/threading.md). Lambdas and local
functions are not suitable replacements for module-level definitions.

Lists, dictionaries, and Python class instances are memoized before their
contents, preserving shared identity and supported reference cycles.
Cycles involving a tuple or a `__reduce__` reconstruction can be restricted;
a tuple-only cycle or a self-reference through a reduce result is unsupported.

`array2d` instances cannot be pickled and raise `TypeError`.
Previously pickled `array2d` data is not supported. Convert appropriate
contents to lists if needed.

## Custom reconstruction

`__reduce__()` must return a two-item tuple
`(reconstruction_callable, argument_tuple)`.
The callable must be resolvable when loading. The hooks `__getnewargs__`,
`__getstate__`, and `__setstate__` are not implemented.

Implementation: [pickle.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/pickle.c).
