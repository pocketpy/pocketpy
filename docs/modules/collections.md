---
icon: package
label: collections
---

# collections

Small container helpers for counting, grouping, and queues.

## Counter

`Counter(iterable)` returns a **plain dictionary** of occurrence counts,
rather than a CPython `Counter` instance. Missing keys raise `KeyError`;
use `get(key, 0)` for a zero default. Methods such as `most_common()` are
not provided.

```python
from collections import Counter, defaultdict, deque

counts = Counter('banana')
assert counts == {'b': 1, 'a': 3, 'n': 2}
assert counts.get('z', 0) == 0

groups = defaultdict(list)
groups['fruit'].append('apple')
assert groups['fruit'] == ['apple']

recent = deque([1, 2], maxlen=3)
recent.append(3)
recent.append(4)
assert list(recent) == [2, 3, 4]
assert recent.popleft() == 2
```

## defaultdict

`defaultdict(default_factory, *args)` calls the factory with no arguments when
`mapping[key]` is missing, stores the result, and returns it.
Provide a callable factory such as `list` or `int`.
`get()` does not create an entry. `copy()` preserves the factory.

## deque

`deque(iterable=None, maxlen=None)` is a double-ended queue. A bounded queue
requires a positive `maxlen`; appending to a full queue removes an item from
the opposite end.

| Operations | Methods |
| --- | --- |
| Add items | `append`, `appendleft`, `extend`, `extendleft` |
| Remove items | `pop`, `popleft`, `clear` |
| Inspect/copy | `count`, `copy`, `maxlen`, `len()`, iteration, membership, equality |
| Rotate | `rotate(n=1)`: positive moves right, negative moves left |

`extendleft()` inserts each element at the left, reversing the input order.
Popping an empty queue raises `IndexError`. Indexing and slicing are not
implemented; convert to a list when needed.

Implementation: [collections.py](https://github.com/pocketpy/pocketpy/blob/main/python/collections.py).
