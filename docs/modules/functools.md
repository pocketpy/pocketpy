---
icon: package
label: functools
---

### `functools.cache`

A decorator that caches a function's return value each time it is called. If called later with the same arguments, the cached value is returned, and not re-evaluated.

### `functools.reduce(function, sequence, initial=...)`

Apply a function of two arguments cumulatively to the items of a sequence, from left to right, so as to reduce the sequence to a single value. For example, `functools.reduce(lambda x, y: x+y, [1, 2, 3, 4, 5])` calculates `((((1+2)+3)+4)+5)`. The left argument, `x`, is the accumulated value and the right argument, `y`, is the update value from the sequence. If the optional `initial` is present, it is placed before the items of the sequence in the calculation, and serves as a default when the sequence is empty.

### `functools.partial(f, *args, **kwargs)`

Return a new partial object which when called will behave like `f` called with the positional arguments `args` and keyword arguments `kwargs`. If more arguments are supplied to the call, they are appended to `args`. If additional keyword arguments are supplied, they extend and override `kwargs`.
