---
icon: package
label: enum
---

# enum

Subclass `Enum` to give constants readable names and values.

```python
from enum import Enum

class Color(Enum):
    RED = 1
    GREEN = 2
    BLUE = 3

assert Color.RED.name == 'RED'
assert Color.RED.value == 1
assert str(Color.RED) == 'Color.RED'
assert Color.RED is Color.RED
```

Members expose read-only `name` and `value` properties.
A finished enum class cannot be subclassed further.

This is a small implementation. It does not provide `IntEnum`, `Flag`,
`auto()`, iteration over the enum class, or lookup by value through
`Color(1)`. Public class attributes are wrapped as members, so keep enum
definitions to constants rather than adding public helper methods.
Duplicate values are not merged into CPython-style aliases.

Implementation: [enum.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/enum.c).
