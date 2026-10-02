---
icon: package
label: typing
---

# typing

A compatibility module for annotations and static-analysis tools. It does not
enforce types at runtime.

```python
from typing import TYPE_CHECKING, cast

def total(values: list[int]) -> int:
    return sum(values)

assert total([1, 2, 3]) == 6
assert TYPE_CHECKING is False
assert cast(int, 'unchanged') == 'unchanged'
```

Common names such as `Any`, `Optional`, `Union`, `Callable`,
`Iterable`, `TypeVar`, `Literal`, and `Self` are placeholders.
`cast(type, value)` returns its value unchanged. The `overload`,
`override`, and `final` decorators also leave their input unchanged.

`Protocol` and `Generic` are aliases of `object`, and `TypedDict` is
an alias of `dict`. Do not use these names as runtime validators or rely on
CPython's typing introspection. Keep type-only imports inside
`if TYPE_CHECKING:` when appropriate.

Available names: [typing.py](https://github.com/pocketpy/pocketpy/blob/main/python/typing.py).
