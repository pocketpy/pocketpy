---
icon: home
label: Welcome
---

# Welcome to pocketpy

pocketpy is a portable Python 3.x interpreter, written in C11.
It aims to be an alternative to Lua for game scripting, with elegant syntax, powerful features and competitive performance.
pocketpy has no dependencies other than the C standard library, which can be easily integrated into your C/C++ project.
Developers are able to write Python bindings via C-API or pybind11 compatible interfaces.

+ [Live Python Demo](https://pocketpy.dev/static/web/): Run Python code in your browser
+ [Live C Examples](https://pocketpy.github.io/examples/): Explore C-APIs in your browser
+ [Godot Extension](https://github.com/pocketpy/godot-pocketpy): Use pocketpy in Godot Engine
+ [VSCode Extension](https://marketplace.visualstudio.com/items?itemName=pocketpy.pocketpy): Debug and profile pocketpy scripts in VSCode
+ [Flutter Plugin](https://pub.dev/packages/pocketpy): Use pocketpy in Flutter apps

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

