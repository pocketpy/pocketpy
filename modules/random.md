### `random.seed(a)`

Set the random seed.

### `random.random()`

Return a random float number in the range [0.0, 1.0).

### `random.randint(a, b)`

Return a random integer in the range [a, b].

### `random.uniform(a, b)`

Return a random float number in the range [a, b).

### `random.choice(seq)`

Return a random element from a sequence.

### `random.shuffle(seq)`

Shuffle a sequence inplace.

### `random.choices(population, weights=None, k=1)`

Return a k sized list of elements chosen from the population with replacement.

### `random.getstate()`

Return the internal state of the generator as a `bytes` object.

### `random.setstate(state)`

Restore the internal state of the generator from a `bytes` object
returned by a previous call to `getstate()`.

### `random.Random(x=None)`

Create a new generator. `x` may be an `int` seed, `None` (seeded lazily from the
system clock on first use), or a state returned by `getstate()`.

A `Random` object supports `pickle`, so its state can be saved and restored:

```python
import pickle, random

r = random.Random(7)
data = pickle.dumps(r)
r2 = pickle.loads(data)
assert r.random() == r2.random()
```
