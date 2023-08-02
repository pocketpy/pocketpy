---
icon: home
label: Welcome
---

# Welcome to pocketpy

pkpy is a lightweight(~10000 LOC) Python interpreter for game scripting, built on C++17 with STL.

It aims to be an alternative to lua for game scripting, with elegant syntax, powerful features and competitive performance.
pkpy is extremely easy to embed via a single header file `pocketpy.h`, without external dependencies.

> **Caution**: pocketpy should not be your first C++ project. Please learn C++ programming, compiling, linking, and debugging before working with pocketpy. There are many resources for this on the net.

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

## Unity plugin

If you are working with [Unity Engine](https://unity.com/), you can download our plugin [PocketPython](https://assetstore.unity.com/packages/tools/visual-scripting/pocketpy-241120) on the Asset Store.

## Sponsor me

You can sponsor me via [Github Sponsors](https://github.com/sponsors/blueloveTH).
