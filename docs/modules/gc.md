---
icon: package
label: gc
---

# gc

Control the current VM's tracing garbage collector.

| Function | Behavior |
| --- | --- |
| `isenabled()` | Whether automatic collection is enabled. |
| `enable()`, `disable()` | Enable/disable automatic collection. |
| `collect()` | Run a full collection now and return the number of reclaimed objects. |
| `collect_hint()` | Collect if the allocation threshold calls for it; return the reclaimed count. |
| `setup_debug_callback(callback)` | Register collection diagnostics; pass `None` to remove the callback. |
| `is_tracked(obj)`, `track(obj)`, `untrack(obj)` | Inspect or change whether the collector traverses an object's references. |

```python
import gc

cycle = []
cycle.append(cycle)
del cycle
reclaimed = gc.collect()
assert type(reclaimed) is int
assert reclaimed >= 1
```

Disabling automatic collection does not stop allocation or prevent explicit
`collect()`/`collect_hint()` calls. A frame-driven application can disable
automatic collection, call `collect_hint()` at an appropriate frame boundary,
and re-enable it when leaving that mode. Memory may grow between collections;
measure the effect before adopting this policy.

`untrack()` is an advanced optimization for containers whose contents do not
need tracing, such as a list containing only integer values. If an untracked
container later contains GC-managed objects, its children can be reclaimed
while still in use. Track it again before storing such values.

Native code must also keep references visible to the collector; see
[C API lifetimes](../C-API/introduction.md).
Full declarations: [gc.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/gc.pyi).
