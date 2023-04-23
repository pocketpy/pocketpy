---
icon: home
label: Welcome
---

# Welcome to PocketPy

PocketPy is a lightweight(~8000 LOC) Python interpreter for game engines.

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

## Source Integration

We recommend to use our pre-built `pocketpy.h` in [Github Release](https://github.com/blueloveTH/pocketpy/releases/latest) page.
To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ Exception must be enabled
+ RTTI is not required

!!!
You can use `g++`, `cl.exe` or `clang++` to compile your project.
For maximum performance, we recommend to `clang++` with `-O2` flag.
`clang++` can produce faster binary than `g++` or `cl.exe`.
!!!