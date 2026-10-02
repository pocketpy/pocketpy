---
icon: package
label: bisect
---

# bisect

Find insertion positions in an already sorted list. Searching takes logarithmic
time; insertion still moves list elements and can take linear time.

| Function | Result |
| --- | --- |
| `bisect_left(a, x, lo=0, hi=None)` | Index before existing entries equal to `x`. |
| `bisect_right(a, x, lo=0, hi=None)` | Index after existing entries equal to `x`. |
| `insort_left(a, x, lo=0, hi=None)` | Insert at the left position; return `None`. |
| `insort_right(a, x, lo=0, hi=None)` | Insert at the right position; return `None`. |

`bisect` aliases `bisect_right`; `insort` aliases `insort_right`.
The search interval is `[lo, hi)`, with `hi=None` meaning `len(a)`.
Keep those bounds within the list. There is no `key=` parameter.

```python
from bisect import bisect_left, bisect_right, insort

scores = [10, 20, 20, 40]
assert bisect_left(scores, 20) == 1
assert bisect_right(scores, 20) == 3
insort(scores, 30)
assert scores == [10, 20, 20, 30, 40]
```

Implementation: [bisect.py](https://github.com/pocketpy/pocketpy/blob/main/python/bisect.py).
