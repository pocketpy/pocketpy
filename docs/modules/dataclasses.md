---
icon: package
label: dataclasses
---

# dataclasses

`@dataclass` uses annotated fields to supply `__init__`, `__repr__`,
`__eq__`, and `__ne__` when the class does not define them.

```python
from dataclasses import dataclass, asdict

@dataclass
class Player:
    name: str
    score: int = 0

player = Player('Ada', score=10)
assert player == Player('Ada', 10)
assert asdict(player) == {'name': 'Ada', 'score': 10}
assert repr(player) == "Player(name='Ada', score=10)"
```

Fields without defaults must precede fields with defaults. Base-class
annotations are included. Annotations describe fields; they do not validate
the values assigned to them.

## Differences from CPython

- Use `@dataclass` directly. Decorator options such as `frozen`, `order`,
  `slots`, and `kw_only` are not implemented.
- There is no `field()`, `default_factory`, or automatic `__post_init__()`
  call.
- A mutable class-level default is shared. Create per-instance containers in
  your own `__init__` instead.
- `asdict(obj)` produces a **shallow** field dictionary. Nested objects and
  containers are not recursively converted or copied.

Implementation: [dataclasses.py](https://github.com/pocketpy/pocketpy/blob/main/python/dataclasses.py).
