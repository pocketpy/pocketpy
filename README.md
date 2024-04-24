# pocketpy: python interpreter in 1 file

<p>
<a title="Build" href="https://github.com/pocketpy/pocketpy/actions/workflows" ><img src="https://github.com/pocketpy/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://codecov.io/gh/pocketpy/pocketpy" > 
 <img src="https://codecov.io/gh/pocketpy/pocketpy/branch/main/graph/badge.svg?token=TI9KAFL0RG"/> 
 </a>
<a href="https://en.wikipedia.org/wiki/C%2B%2B#Standardization">
<img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-blue.svg"></a>
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
<!-- docs -->
<a href="https://pocketpy.dev">
<img alt="Website" src="https://img.shields.io/website/https/pocketpy.dev.svg?down_color=red&down_message=offline&up_color=blue&up_message=online"></a>
<a title="Discord" href="https://discord.gg/WWaq72GzXv" >
<img src="https://img.shields.io/discord/1048978026131640390.svg" /></a>
</p>

pkpy is a lightweight(~15K LOC) Python interpreter for game scripting, built on C++17 with STL.

It aims to be an alternative to lua for game scripting, with elegant syntax, powerful features and competitive performance.
pkpy is extremely easy to embed via a single header file `pocketpy.h`, without external dependencies.

Please see https://pocketpy.dev for details or try [Live Demo](https://pocketpy.dev/static/web/).

## Supported Platforms

pkpy should work on any platform with a C++17 compiler.
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

Download the `pocketpy.h` on our [GitHub Release](https://github.com/pocketpy/pocketpy/releases) page.
And `#include` it in your project. The header can only be included once.

#### Use CMake

Clone the whole repository as a submodule into your project,
In your CMakelists.txt, add the following lines:

```cmake
add_subdirectory(pocketpy)
target_link_libraries(<your_target> pocketpy)

if(EMSCRIPTEN)
    # exceptions must be enabled for emscripten
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions")
endif()
```

See [CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt) for details.

It is safe to use `main` branch in production if CI badge is green.

### Compile Flags

To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ RTTI must be enabled
+ Exception must be enabled
+ For MSVC, `/utf-8` flag must be set

For development build, use this snippet.
```bash
# prerequisites
pip install cmake
# build the repo
python cmake_build.py
# unittest
python scripts/run_tests.py
```

### Example

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Hello world!
    vm->exec("print('Hello world!')");

    // Create a list
    vm->exec("a = [1, 2, 3]");

    // Eval the sum of the list
    PyObject* result = vm->eval("sum(a)");
    std::cout << "Sum of the list: "<< py_cast<int>(vm, result) << std::endl;   // 6

    // Bindings
    vm->bind(vm->_main, "add(a: int, b: int)",
      [](VM* vm, ArgsView args){
        int a = py_cast<int>(vm, args[0]);
        int b = py_cast<int>(vm, args[1]);
        return py_var(vm, a + b);
      });

    // Call the function
    PyObject* f_add = vm->_main->attr("add");
    result = vm->call(f_add, py_var(vm, 3), py_var(vm, 7));
    std::cout << "Sum of 2 variables: "<< py_cast<int>(vm, result) << std::endl;   // 10

    // Dispose the virtual machine
    delete vm;
    return 0;
}
```

## Features

Check this [Cheatsheet](https://reference.pocketpy.dev/python.html)
for a quick overview of the supported features.

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
| Exception       | `raise/try..catch..finally`     | ✅       |
| Dynamic Code    | `eval()/exec()`                 | ✅       |
| Reflection      | `hasattr()/getattr()/setattr()` | ✅       |
| Import          | `import/from..import`           | ✅       |
| Context Block   | `with <expr> as <id>:`          | ✅       |
| Type Annotation | `def f(a:int, b:float=1)`       | ✅       |
| Generator       | `yield i`                       | ✅       |
| Decorator       | `@cache`                        | ✅       |

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
| [TIC-80](https://github.com/nesbox/TIC-80)                      | TIC-80 is a fantasy computer for making, playing and sharing tiny games. |
| [MiniPythonIDE](https://github.com/CU-Production/MiniPythonIDE) | A python ide base on pocketpy                                            |
| [py-js](https://github.com/shakfu/py-js)                        | Python3 externals for Max / MSP                                          |
| [crescent](https://github.com/chukobyte/crescent)               | Crescent is a cross-platform 2D fighting and beat-em-up game engine.     |

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

[![Star History Chart](https://api.star-history.com/svg?repos=blueloveth/pocketpy&type=Date)](https://star-history.com/#blueloveth/pocketpy&Date)



## License

[MIT License](http://opensource.org/licenses/MIT)
