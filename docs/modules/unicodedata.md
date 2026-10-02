---
icon: package
label: unicodedata
---

# unicodedata

This module provides `east_asian_width(character)`, returning the Unicode
East Asian width classification of one character.

| Result | Meaning |
| --- | --- |
| `F` | Fullwidth |
| `H` | Halfwidth |
| `N` | Neutral |
| `Na` | Narrow |
| `W` | Wide |
| `A` | Ambiguous |

```python
from unicodedata import east_asian_width

assert east_asian_width('A') == 'Na'
assert east_asian_width('中') == 'W'
```

The argument must be a string containing exactly one Unicode character.
A longer string, including a multi-character grapheme, raises `TypeError`.

A width classification is not a complete terminal-layout algorithm:
combining characters, emoji sequences, and ambiguous-width characters need
additional handling. This module does not implement Unicode normalization,
names, or the rest of CPython's `unicodedata` API.
