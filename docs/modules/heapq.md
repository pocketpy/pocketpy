---
icon: package
label: heapq
---

### `heapq.heappush(heap, item)`

Push the value `item` onto the heap, maintaining the heap invariant.

### `heapq.heappop(heap)`

Pop and return the smallest item from the heap, maintaining the heap invariant. If the heap is empty, IndexError is raised. To access the smallest item without popping it, use `heap[0]`.

### `heapq.heapify(x)`

Transform list `x` into a heap, in-place, in linear time.

### `heapq.heappushpop(heap, item)`

Push `item` on the heap, then pop and return the smallest item from the heap. The combined action runs more efficiently than `heappush()` followed by a separate `heappop()`.

### `heapq.heapreplace(heap, item)`

Pop and return the smallest item from the heap, and also push the new item. The heap size doesn’t change. If the heap is empty, IndexError is raised.