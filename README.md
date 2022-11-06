# pocketpy

![build](https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg)

`pocketpy` is a C++17 single-file header-only cross platform Python Interpreter.

![sample_img](docs/readme_sample.png)

**CURRENTLY IN DEVELOPMENT!!**

## Prebuilt Windows Binaries

Go to https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml to get an artifact.

## Build From Source

First clone the repository

```bash
git clone https://github.com/blueloveTH/pocketpy
cd pocketpy
```

**If you want to get a single header file:**

```bash
python3 amalgamate.py
```

It will generate `pocketpy.h` and `main.cpp` in `amalgamate/` directory. You can use `main.cpp` to build a REPL console or use `pocketpy.h` only to embed it to your game engine.

**If you want to do development:**

```bash
g++ -o pocketpy src/main.cpp --std=c++17 -O1
```

## Reference

+ [cpython](https://github.com/python/cpython)

+ [byterun](http://qingyunha.github.io/taotao/)

