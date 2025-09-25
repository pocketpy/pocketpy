# pocketpy: portable python 3.x interpreter

<p>
<!-- Build -->
<a title="Build" href="https://github.com/pocketpy/pocketpy/actions/workflows" >
<img src="https://github.com/pocketpy/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<!-- Codecov -->
<a href="https://codecov.io/gh/pocketpy/pocketpy" > 
<img src="https://codecov.io/gh/pocketpy/pocketpy/branch/main/graph/badge.svg?token=TI9KAFL0RG"/></a>
<!-- C11 -->
<a href="https://en.wikipedia.org/wiki/C11_(C_standard_revision)">
<img alt="Python" src="https://img.shields.io/badge/C-11-blue.svg"></a>
<!-- License -->
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<!-- Github Release -->
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
<!-- Discord -->
<a title="Discord" href="https://discord.gg/WWaq72GzXv" >
<img src="https://img.shields.io/discord/1048978026131640390.svg" /></a>
<!-- HelloGithub -->
<a href="https://hellogithub.com/repository/dd9c509d72a64caca03d99d5b1991a33" target="_blank"><img src="https://abroad.hellogithub.com/v1/widgets/recommend.svg?rid=dd9c509d72a64caca03d99d5b1991a33&claim_uid=jhOYmWGE75AL0Bp&theme=small" alt="Featured｜HelloGitHub" /></a>
<!-- DeepWiki -->
<a href="https://deepwiki.com/pocketpy/pocketpy"><img src="https://deepwiki.com/badge.svg" alt="Ask DeepWiki"></a>
</p>

pocketpy is a portable Python 3.x interpreter, written in C11.
It aims to be an alternative to Lua for game scripting, with elegant syntax, powerful features and competitive performance.
pocketpy has no dependencies other than the C standard library, which can be easily integrated into your C/C++ project.
Developers are able to write Python bindings via C-API or pybind11 compatible interfaces.

Please see https://pocketpy.dev for details and try the following resources.
+ [Live Python Demo](https://pocketpy.dev/static/web/): Run Python code in your browser
+ [Live C Examples](https://pocketpy.github.io/examples/): Explore C-APIs in your browser
+ [Godot Extension](https://github.com/pocketpy/godot-pocketpy): Use pocketpy in Godot Engine
+ [VSCode Extension](https://marketplace.visualstudio.com/items?itemName=pocketpy.pocketpy): Debug and profile pocketpy scripts in VSCode
+ [Flutter Plugin](https://pub.dev/packages/pocketpy): Use pocketpy in Flutter apps

## Supported Platforms

pkpy should work on any platform with a C11 compiler.
These platforms are officially tested.

+ Windows 64-bit
+ Linux 64-bit / 32-bit
+ macOS 64-bit
+ Android 64-bit / 32-bit
+ iOS 64-bit
+ Emscripten 32-bit
+ Raspberry Pi OS 64-bit

## Quick Start

You have two options to integrate pkpy into your project.

#### Use the single header file

Download the `pocketpy.h` and `pocketpy.c` on our [GitHub Release](https://github.com/pocketpy/pocketpy/releases) page.
And `#include` it in your project.

#### Use CMake

Clone the whole repository as a submodule into your project,
In your CMakelists.txt, add the following lines:

```cmake
add_subdirectory(pocketpy)
target_link_libraries(<your_target> pocketpy)
```

See [CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt) for details.

It is safe to use `main` branch in production if CI badge is green.

### Compile Flags

To compile it with your project, these flags must be set:

+ `--std=c11` flag must be set
+ For MSVC, `/utf-8` and `/experimental:c11atomics` flag must be set
+ `NDEBUG` macro should be defined for release build, or you will get poor performance

For amalgamated build, run `python amalgamate.py` to generate `pocketpy.c` and `pocketpy.h` in `amalgamated/` directory.

### Example

```c
#include "pocketpy.h"
#include <stdio.h>

static bool int_add(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    py_newint(py_retval(), a + b);
    return true;
}

int main() {
    // Initialize pocketpy
    py_initialize();

    // Hello world!
    bool ok = py_exec("print('Hello world!')", "<string>", EXEC_MODE, NULL);
    if(!ok) goto __ERROR;

    // Create a list: [1, 2, 3]
    py_Ref r0 = py_getreg(0);
    py_newlistn(r0, 3);
    py_newint(py_list_getitem(r0, 0), 1);
    py_newint(py_list_getitem(r0, 1), 2);
    py_newint(py_list_getitem(r0, 2), 3);

    // Eval the sum of the list
    py_Ref f_sum = py_getbuiltin(py_name("sum"));
    py_push(f_sum);
    py_pushnil();
    py_push(r0);
    ok = py_vectorcall(1, 0);
    if(!ok) goto __ERROR;

    printf("Sum of the list: %d\n", (int)py_toint(py_retval()));  // 6

    // Bind native `int_add` as a global variable
    py_newnativefunc(r0, int_add);
    py_setglobal(py_name("add"), r0);

    // Call `add` in python
    ok = py_exec("add(3, 7)", "<string>", EVAL_MODE, NULL);
    if(!ok) goto __ERROR;

    py_i64 res = py_toint(py_retval());
    printf("Sum of 2 variables: %d\n", (int)res);  // 10

    py_finalize();
    return 0;

__ERROR:
    py_printexc();
    py_finalize();
    return 1;
}
```

## Features

| Name            | Example                         | Supported |
| --------------- | ------------------------------- | --------- |
| If Else         | `if..else..elif`                | ✅       |
| Loop            | `for/while/break/continue`      | ✅       |
| Function        | `def f(x,*args,y=1):`           | ✅       |
| Subclass        | `class A(B):`                   | ✅       |
| List            | `[1, 2, 'a']`                   | ✅       |
| ListComp        | `[i for i in range(5)]`         | ✅       |
| Slice           | `a[1:2], a[:2], a[1:]`          | ✅       |
| Tuple           | `(1, 2, 'a')`                   | ✅       |
| Dict            | `{'a': 1, 'b': 2}`              | ✅       |
| F-String        | `f'value is {x}'`               | ✅       |
| Unpacking       | `a, b = 1, 2`                   | ✅       |
| Star Unpacking  | `a, *b = [1, 2, 3]`             | ✅       |
| Exception       | `raise/try..except..`           | ✅       |
| Dynamic Code    | `eval()/exec()`                 | ✅       |
| Reflection      | `hasattr()/getattr()/setattr()` | ✅       |
| Import          | `import/from..import`           | ✅       |
| Context Block   | `with <expr> as <id>:`          | ✅       |
| Type Annotation | `def f(a:int, b:float=1)`       | ✅       |
| Generator       | `yield i`                       | ✅       |
| Decorator       | `@cache`                        | ✅       |
| Match Case      | `match code: case 200:`         | ✅       |

## Performance

Currently, pkpy is as fast as cpython 3.9.
Performance results for cpython 3.9 are applicable to for pkpy.

See https://pocketpy.dev/performance/ for details.

And these are the results of the primes benchmark on Intel i5-12400F, WSL (Ubuntu 20.04 LTS), which *roughly* reflects the performance among c++, lua, pkpy and cpython.

| name | version | time | file |
| ---- | ---- | ---- | ---- |
| c++ | gnu++11 | `0.104s ■□□□□□□□□□□□□□□□` | [benchmarks/primes.cpp](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.cpp) |
| lua | 5.3.3 | `1.576s ■■■■■■■■■□□□□□□□` | [benchmarks/primes.lua](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.lua) |
| pkpy | 1.2.7 | `2.385s ■■■■■■■■■■■■■□□□` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |
| cpython | 3.8.10 | `2.871s ■■■■■■■■■■■■■■■■` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |

## Used By

|                                                                 | Description                                                              |
|-----------------------------------------------------------------|--------------------------------------------------------------------------|
| [godot-pocketpy](https://github.com/pocketpy/godot-pocketpy)    | Godot extension for using pocketpy in Godot Engine.                      |
| [TIC-80](https://github.com/nesbox/TIC-80)                      | TIC-80 is a fantasy computer for making, playing and sharing tiny games. |
| [py-js](https://github.com/shakfu/py-js)                        | Python3 externals for Max / MSP.                                         |
| [crescent](https://github.com/chukobyte/crescent)               | Crescent is a cross-platform 2D fighting and beat-em-up game engine.     |
| [orxpy](https://github.com/hcarty/orx)                          | Python extension for orx engine.                                         |
| [CANopenTerm](https://canopenterm.de/python-api)                | Open-source software tool for CANopen CC networks and devices.           |

Submit a pull request to add your project here.

## Contribution

All kinds of contributions are welcome.

- Submit a Pull Request
  - fix a bug
  - add a new feature
- Open an Issue
  - any suggestions
  - any questions

If you find pkpy useful, consider star this repository (●'◡'●)

## Sponsor this project

You can sponsor this project via these ways.

+ [Github Sponsors](https://github.com/sponsors/blueloveTH)
+ [Buy me a coffee](https://www.buymeacoffee.com/blueloveth)

Your sponsorship will help us develop pkpy continuously.

## Reference

+ [cpython](https://github.com/python/cpython)

  The official implementation of Python programming language.

+ [byterun](https://www.aosabook.org/en/500L/a-python-interpreter-written-in-python.html)

  An excellent learning material. It illustrates how Python's virtual machine works.


## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=pocketpy/pocketpy&type=Date)](https://www.star-history.com/#pocketpy/pocketpy&Date)


## License

[MIT License](http://opensource.org/licenses/MIT)
