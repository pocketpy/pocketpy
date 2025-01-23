---
icon: home
label: Welcome
---

# Welcome to pocketpy

pkpy is a lightweight(~15K LOC) Python 3.x interpreter for game scripting, written in C11.

It aims to be an alternative to lua for game scripting, with elegant syntax, powerful features and competitive performance.
pkpy is extremely easy to embed via a single header file `pocketpy.h`, without external dependencies.


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

pkpy should work on any platform with a C11 compiler.
These platforms are officially tested.

> C99 compilers may also work currently according to users' feedback.

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

## Upgrade to v2.0

pkpy v2.0 is a C11 project instead of C++17. All your existing code for v1.x won't work anymore.

We provide two API sets for v2.0, C-API and pybind11 API (C\+\+17). If you are a C user, use the C-API. If you are a C\+\+ user, use the pybind11 API.

