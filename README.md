# pocketpy

![build](https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg)

C++17 single-file header-only cross platform Python Interpreter.

![sample_img](docs/sample.png)

## Documentations
See https://pocketpy.dev


## Build From Source (Linux)

First clone the repository

```bash
git clone https://github.com/blueloveTH/pocketpy
cd pocketpy
```

Then run

```bash
python3 amalgamate.py
```

It will generate `pocketpy.h` and `main.cpp` in `amalgamated/` directory. You can use `main.cpp` to build a REPL console or use `pocketpy.h` to embed it to your game engine.

## Reference

+ [cpython](https://github.com/python/cpython)

+ [byterun](http://qingyunha.github.io/taotao/)

