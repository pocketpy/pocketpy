---
icon: package
label: pickle
---

### `pickle.dumps(obj) -> bytes`

Return the pickled representation of an object as a bytes object.

### `pickle.loads(b: bytes)`

Return the unpickled object from a bytes object.


## What can be pickled and unpickled?

The following types can be pickled:

- [x] None, True, and False;
- [x] integers, floating-point numbers;
- [x] strings, bytes;
- [x] tuples, lists, sets, and dictionaries containing only picklable objects;
- [ ] functions (built-in and user-defined) accessible from the top level of a module (using def, not lambda);
- [x] classes accessible from the top level of a module;
- [x] instances of such classes
- [x] `PY_STRUCT_LIKE` objects

The following magic methods are available:

- [x] `__getnewargs__`
- [ ] `__getstate__`
- [ ] `__setstate__`
- [ ] `__reduce__`
