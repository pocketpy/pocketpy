---
icon: package
label: heapq
---

# heapq

Use a list as a min-heap: `heap[0]` is the smallest item, while the rest of the
list is not necessarily sorted.

| Function | Behavior |
| --- | --- |
| `heapify(items)` | Rearrange a list in place into a heap, in linear time. |
| `heappush(heap, item)` | Add an item while maintaining the heap. |
| `heappop(heap)` | Remove and return the smallest item; fail with `IndexError` if empty. |
| `heappushpop(heap, item)` | Push, then remove the smallest; on an empty heap, return `item`. |
| `heapreplace(heap, item)` | Remove the old smallest item, then insert `item`; require a nonempty heap. |

Items must support ordering. Push and pop operations take logarithmic time.
This module provides these five functions; CPython helpers such as `merge`,
`nlargest`, and `nsmallest` are not included.

```python
import heapq

jobs = [(3, 'save'), (1, 'input'), (2, 'update')]
heapq.heapify(jobs)
heapq.heappush(jobs, (0, 'network'))
assert heapq.heappop(jobs) == (0, 'network')
assert heapq.heappop(jobs) == (1, 'input')
```

Tuple priorities compare subsequent fields to break ties. Add a numeric sequence
number if tied jobs contain objects that cannot be compared.

Implementation: [heapq.py](https://github.com/pocketpy/pocketpy/blob/main/python/heapq.py).
