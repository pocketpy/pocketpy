---
icon: package
label: time
---

# time

Clock access and a small local-time interface.

| Function | Result |
| --- | --- |
| `time()` | Seconds since the Unix epoch as a float. |
| `time_ns()` | Epoch time as integer nanoseconds. |
| `monotonic()`, `monotonic_ns()` | Platform monotonic clock, in seconds or nanoseconds where supported. |
| `perf_counter()` | Uses the same clock as `monotonic()`. |
| `process_time()` | The C library's `clock() / CLOCKS_PER_SEC`; semantics depend on the platform. |
| `sleep(seconds)` | Wait for a duration. |
| `localtime()` | Current local time as a `struct_time` object. |

```python
import time

start = time.perf_counter()
total = sum([i * i for i in range(1000)])
elapsed = time.perf_counter() - start
print(total, elapsed)

now = time.localtime()
print(now.tm_year, now.tm_mon, now.tm_mday)
```

`struct_time` exposes `tm_year`, `tm_mon`, `tm_mday`, `tm_hour`,
`tm_min`, `tm_sec`, `tm_wday`, `tm_yday`, and `tm_isdst`.
Weekdays start at Monday = 0; day-of-year starts at 1.
`localtime()` takes no timestamp argument.

The fallback for platforms without a monotonic clock uses wall time, so do not
assume a monotonic guarantee on every target. `sleep()` currently waits in a
loop and yields when thread support is enabled; it is not a low-power OS sleep.
Use your host's frame scheduler or native wait primitive for long idle periods.

Implementation: [time.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/time.c).
