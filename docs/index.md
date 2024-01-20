---
icon: home
label: Welcome
---

# Welcome to pocketpy

pkpy is a lightweight(~15K LOC) Python interpreter for game scripting, built on C++17 with STL.

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

## Supported platforms

pkpy should work on any platform with a C++17 compiler.
These platforms are officially tested.

+ Windows 64-bit
+ Linux 64-bit / 32-bit
+ macOS 64-bit
+ Android 64-bit / 32-bit
+ iOS 64-bit
+ Emscripten 32-bit
+ Raspberry Pi OS 64-bit

## Star the repo

If you find pkpy useful, consider [star this repository](https://github.com/blueloveth/pocketpy) (●'◡'●)

## Sponsor this project

You can sponsor this project via these ways.

+ [Github Sponsors](https://github.com/sponsors/blueloveTH)
+ [Buy me a coffee](https://www.buymeacoffee.com/blueloveth)

Your sponsorship will help us develop pkpy continuously.
