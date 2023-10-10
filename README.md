# pocketpy: python interpreter in 1 file

<p>
<a title="Build" href="https://github.com/blueloveTH/pocketpy/actions/workflows" ><img src="https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://codecov.io/gh/blueloveTH/pocketpy" > 
 <img src="https://codecov.io/gh/blueloveTH/pocketpy/branch/main/graph/badge.svg?token=TI9KAFL0RG"/> 
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

**English |** [**简体中文**](README_zh.md)

pkpy is a lightweight(~14K LOC) Python interpreter for game scripting, built on C++17 with STL.

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

Download the `pocketpy.h` on our [GitHub Release](https://github.com/blueloveTH/pocketpy/releases) page.
And `#include` it in your project.

You can also use cmake to build it from source. See CMakeLists.txt for details.
These variables can be set to control the build process:
+ `PK_BUILD_STATIC_LIB` - Build the static library
+ `PK_BUILD_SHARED_LIB` - Build the shared library

### Compile Flags

To compile it with your project, these flags must be set:

+ `--std=c++17` flag must be set
+ Exception must be enabled
+ For MSVC, `/utf-8` flag must be set

For development build on Linux, use this snippet.
```bash
# prerequisites
sudo apt-get install libc++-dev libc++abi-dev clang
# build the repo
bash build.sh
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
    std::cout << py_cast<int>(vm, result);   // 6

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
    std::cout << py_cast<int>(vm, result);   // 10

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
| Exception       | `raise/try..catch`              | ✅       |
| Dynamic Code    | `eval()/exec()`                 | ✅       |
| Reflection      | `hasattr()/getattr()/setattr()` | ✅       |
| Import          | `import/from..import`           | ✅       |
| Context Block   | `with <expr> as <id>:`          | ✅       |
| Type Annotation | `def  f(a:int, b:float=1)`      | ✅       |
| Generator       | `yield i`                       | ✅       |
| Decorator       | `@cache`                        | ✅       |

## Used By

|                                                              | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [TIC-80](https://github.com/nesbox/TIC-80)                   | TIC-80 is a fantasy computer for making, playing and sharing tiny games. |
| [ct-py](https://github.com/blueloveTH/ct-py)                 | Ct.py🥕 is cross-platform 2D game framework built on raylib and imgui. |
| [MiniPythonIDE](https://github.com/CU-Production/MiniPythonIDE) | A python ide base on pocketpy                                |
| [py-js](https://github.com/shakfu/py-js)                     | Python3 externals for Max / MSP                              |

Submit a pull request to add your project here.

## Contribution

All kinds of contributions are welcome.

- Submit a Pull Request
  - fix a bug
  - add a new feature
- Open an Issue
  - any suggestions
  - any questions

If you find pocketpy useful, consider star this repository (●'◡'●)

## Sponsor me

You can sponsor me via [Github Sponsors](https://github.com/sponsors/blueloveTH). Your sponsorship will help me develop this project continuously.

## Reference

+ [cpython](https://github.com/python/cpython)

  The official implementation of Python programming language.

+ [byterun](https://www.aosabook.org/en/500L/a-python-interpreter-written-in-python.html)

  An excellent learning material. It illustrates how Python's virtual machine works.

+ [box2d](https://box2d.org/)

  The world's best 2D physics engine, written by Erin Catto. `box2d` now becomes a built-in module in pkpy `v1.1.3` and later.


## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=blueloveth/pocketpy&type=Date)](https://star-history.com/#blueloveth/pocketpy&Date)



## License

[MIT License](http://opensource.org/licenses/MIT)
