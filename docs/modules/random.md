---
icon: package
label: random
---

# random

Pseudo-random numbers from a Mersenne Twister generator. Module-level functions
use a shared generator in the current VM; `Random` creates independent state.
This generator is for simulations and games, not cryptographic secrets.

| API | Behavior |
| --- | --- |
| `seed(value)` | Seed with an integer, or `None` for the clock. Integer seeds use the low 32 bits. |
| `random()` | Float in `[0.0, 1.0)`. |
| `randint(a, b)` | Integer including both endpoints; require `a <= b`. |
| `uniform(a, b)` | Float between the endpoints; floating-point rounding can affect the boundary. |
| `choice(sequence)` | One item from a nonempty list, tuple, or string. |
| `shuffle(items)` | Shuffle a list in place; return `None`. |
| `choices(population, weights=None, k=1)` | List of `k` selections with replacement; population and weights are lists or tuples. |
| `getstate()`, `setstate(state)` | Save/restore the generator's state as bytes. |
| `Random(value=None)` | New generator, optionally initialized with an integer seed or saved state bytes. |

For weighted choices, supply finite nonnegative weights with matching length
and total greater than `1e-6`; use a nonnegative `k`.
Unseeded generators initialize from the clock on first use.

## Repeatable independent draws

```python
from random import Random

rng = Random(7)
state = rng.getstate()
rolls = [rng.randint(1, 6) for _ in range(5)]
rng.setstate(state)
assert rolls == [rng.randint(1, 6) for _ in range(5)]

loot = rng.choices(['coin', 'gem'], weights=[9, 1], k=3)
assert len(loot) == 3
assert all([item in ['coin', 'gem'] for item in loot])
```

State bytes use pocketpy's internal representation, not CPython's state tuple.
Do not assume sequences or saved states are interchangeable with CPython or
across future runtime versions.

A generator can also be [pickled](pickle.md):

```python
import pickle
from random import Random

original = Random(7)
restored = pickle.loads(pickle.dumps(original))
assert original.random() == restored.random()
```
