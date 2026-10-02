---
icon: package
label: functools
---

# functools

Helpers for caching calls, reducing a sequence, and pre-filling arguments.

| API | Behavior |
| --- | --- |
| `@cache` | Cache results without a size limit. |
| `@lru_cache(maxsize=128)` | Keep up to `maxsize` recently used argument tuples. |
| `reduce(function, sequence, initial=...)` | Combine items from left to right. An empty sequence needs an initial value. |
| `partial(function, *args, **kwargs)` | Store arguments for later calls. |

## Cache a computation

```python
from functools import lru_cache, reduce

@lru_cache(maxsize=32)
def fibonacci(n):
    if n < 2:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

assert fibonacci(10) == 55
assert reduce(lambda total, value: total + value, [1, 2, 3], 0) == 6
```

The cache decorators accept **positional calls only**, with hashable arguments.
Use a positive integer `maxsize`; the implementation does not support
CPython's `maxsize=None`, zero-size mode, `typed`, or `cache_info()`/
`cache_clear()` methods. Cached mutable results are returned as the same
object on later calls.

## Partial calls

```python
from functools import partial

def label(name, prefix='item'):
    return prefix + ': ' + name

enemy_label = partial(label, prefix='enemy')
assert enemy_label('slime') == 'enemy: slime'
assert enemy_label('slime', prefix='boss') == 'enemy: slime'
```

Stored positional arguments are prepended to new ones. **Stored keyword
arguments override keywords passed at call time** in this implementation.
This differs from CPython, where call-time keywords win.

Implementation: [functools.py](https://github.com/pocketpy/pocketpy/blob/main/python/functools.py).
