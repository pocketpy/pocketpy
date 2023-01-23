# pocketpy

<p>
<a title="Build" href="https://github.com/blueloveTH/pocketpy/actions/workflows" ><img src="https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a title="Pub" href="https://pub.dev/packages/pocketpy" ><img src="https://img.shields.io/pub/v/pocketpy" /></a>
</p>

> This project is undergoing a major reconstruction!!!
> 
> Advanced features such as exception, yield/coroutine and complete C bindings support will be added.
> 
> Interfaces will be changed a lot. Be cautious!

C++17 header-only Python interpreter for game engines.

Please see https://pocketpy.dev for detailed documentations.

![sample_img](docs/sample.png)


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

