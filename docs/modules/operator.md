---
icon: package
label: operator
---

# operator

Function forms of Python operators, useful for callbacks and reductions.

| Operation | Functions |
| --- | --- |
| Comparison | `lt`, `le`, `eq`, `ne`, `ge`, `gt` |
| Arithmetic | `add`, `sub`, `mul`, `truediv`, `floordiv`, `mod`, `pow`, `matmul`, `neg` |
| Bitwise | `and_`, `or_`, `xor`, `invert`, `lshift`, `rshift` |
| Identity/truth | `is_`, `is_not`, `not_`, `truth` |
| Containers | `contains(container, value)`, `getitem(obj, key)`, `setitem(obj, key, value)`, `delitem(obj, key)` |
| Accessor factories | `itemgetter(key)`, `attrgetter(name)` |

For binary operators, `operator.add(a, b)` means `a + b`, and similarly
for the other names. `contains(a, b)` means `b in a`, so the container
comes first.

```python
from operator import itemgetter, mul
from functools import reduce

players = [('Ada', 30), ('Bo', 10), ('Cy', 20)]
assert sorted(players, key=itemgetter(1)) == [('Bo', 10), ('Cy', 20), ('Ada', 30)]
assert reduce(mul, [2, 3, 4], 1) == 24
```

`itemgetter` accepts one key and `attrgetter` accepts one attribute name.
Multiple keys/names and dotted attribute traversal are not implemented.

The in-place helpers are `iadd`, `isub`, `imul`, `itruediv`,
`ifloordiv`, `imod`, `iand`, `ior`, `ixor`, `ilshift`, and
`irshift`. They perform the corresponding augmented assignment and return
the result. They do not rebind the caller's variable:

```python
from operator import iadd

value = 2
value = iadd(value, 3)
assert value == 5
```

Custom `__iadd__`-style hooks remain unsupported; see
[language differences](../features/differences.md).
Implementation: [operator.py](https://github.com/pocketpy/pocketpy/blob/main/python/operator.py).
