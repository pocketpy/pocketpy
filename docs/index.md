---
icon: home
label: Welcome
---

# Welcome to PocketPy

pkpy is a lightweight(~8000 LOC) Python interpreter for game engine/apps.

It is extremely easy to embed. Including a compiler, optimizer and bytecode virtual machine. All of them are available in a single header file `pocketpy.h`, without external dependencies.

## What it looks like

```python
def is_prime(x):
  if x < 2:
    return False
  for i in range(2, x):
    if x % i == 0:
      return False
  return True

primes = [i for i in range(2, 20) if is_prime(i)]
print(primes)
# [2, 3, 5, 7, 11, 13, 17, 19]
```