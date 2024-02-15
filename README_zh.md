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

pocketpy 是一个轻量级的 Python 解释器，为嵌入至游戏引擎而设计，基于 C++17 和 STL。

它包含一个编译器和基于字节码的虚拟机，以及交互式命令窗的实现。所有功能均集成在单个头文件 `pocketpy.h` 中，不包含外部依赖项，能很方便地嵌入至你的应用。

你可以浏览 https://pocketpy.dev 获取更多信息或是 [在浏览器中体验](https://pocketpy.dev/static/web/) pocketpy 的交互式界面（REPL）。

## 支持的平台

pkpy 支持任何拥有 C++17 编译器的平台。
以下平台由官方测试通过。

+ Windows 64-bit
+ Linux 64-bit / 32-bit
+ macOS 64-bit
+ Android 64-bit / 32-bit
+ iOS 64-bit
+ Emscripten 32-bit
+ Raspberry Pi OS 64-bit

## 快速上手

你可以在 [Github Release](https://github.com/pocketpy/pocketpy/releases) 页面下载 `pocketpy.h`，
并加入到你的工程中。请参阅 https://pocketpy.dev 以获取更详细的文档。

你也可以是用CMake从源代码开始构建。详情请见[CMakeLists.txt](https://github.com/pocketpy/pocketpy/blob/main/CMakeLists.txt)
修改以下变量可以控制构建过程
+ `PK_BUILD_STATIC_LIB` - 构建静态库 (默认, 推荐)
+ `PK_BUILD_SHARED_LIB` - 构建动态库

在生产中使用 `main` 分支是安全的。

### 编译标志

若要将pocketpy与你的项目一起编译，必须设置以下标志：

+ `--std=c++17` 标志必须被设置
+ Exception 必须被允许
+ 对于MSVC, `/utf-8` 标志必须被设置

对于开发者构建，使用以下脚本
```bash
# 前提
pip install cmake
# 构建仓库
python cmake_build.py
# 单元测试
python scripts/run_tests.py
```

```cpp
#include "pocketpy.h"

using namespace pkpy;

int main(){
    // 建立一个虚拟机
    VM* vm = new VM();

    // Hello world!
    vm->exec("print('Hello world!')");

    // 构造一个列表
    vm->exec("a = [1, 2, 3]");

    // 计算列表元素之和
    PyObject* result = vm->eval("sum(a)");
    std::cout << py_cast<int>(vm, result);   // 6

    // 绑定一个函数
    vm->bind(vm->_main, "add(a: int, b: int)",
      [](VM* vm, ArgsView args){
        int a = py_cast<int>(vm, args[0]);
        int b = py_cast<int>(vm, args[1]);
        return py_var(vm, a + b);
      });

    // 调用函数
    PyObject* f_add = vm->_main->attr("add");
    result = vm->call(f_add, py_var(vm, 3), py_var(vm, 7));
    std::cout << py_cast<int>(vm, result);   // 10

    // 释放虚拟机
    delete vm;
    return 0;
}
```

## 支持的语法特性

| 特性         | 示例                            | 支持 |
| ------------ | ------------------------------- | ---- |
| 分支         | `if..else..elif`                | ✅ |
| 循环         | `for/while/break/continue`      | ✅ |
| 函数         | `def f(x,*args,y=1):`           | ✅ |
| 类与继承     | `class A(B):`                   | ✅ |
| 列表         | `[1, 2, 'a']`                   | ✅ |
| 列表生成式   | `[i for i in range(5)]`         | ✅ |
| 切片         | `a[1:2], a[:2], a[1:]`          | ✅ |
| 元组         | `(1, 2, 'a')`                   | ✅ |
| 字典         | `{'a': 1, 'b': 2}`              | ✅ |
| 格式化字符串 | `f'value is {x}'`               | ✅ |
| 序列解包     | `a, b = 1, 2`                   | ✅ |
| 异常         | `raise/try..catch`              | ✅ |
| 动态分发     | `eval()/exec()`                 | ✅ |
| 反射         | `hasattr()/getattr()/setattr()` | ✅ |
| 导入模块     | `import/from..import`           | ✅ |
| 上下文管理器 | `with <expr> as <id>:`          | ✅ |
| 类型标注     | `def f(a:int, b:float=1)`      | ✅ |
| 生成器       | `yield i`                       | ✅ |
| 装饰器       | `@cache`                        | ✅ |

## 性能

目前，pkpy的速度与cpython 3.9一样快。
cpython 3.9的性能结果适用于pkpy。
访问 https://pocketpy.dev/performance/ 以获取详细信息。
以下是 Intel i5-12400F、WSL（Ubuntu 20.04 LTS）上的primes基准测试的结果，大致反映了c++、lua、pkpy和cpython之间的性能。

| name | version | time | file |
| ---- | ---- | ---- | ---- |
| c++ | gnu++11 | `0.104s ■□□□□□□□□□□□□□□□` | [benchmarks/primes.cpp](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.cpp) |
| lua | 5.3.3 | `1.576s ■■■■■■■■■□□□□□□□` | [benchmarks/primes.lua](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.lua) |
| pkpy | 1.2.7 | `2.385s ■■■■■■■■■■■■■□□□` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |
| cpython | 3.8.10 | `2.871s ■■■■■■■■■■■■■■■■` | [benchmarks/primes.py](https://github.com/pocketpy/pocketpy/blob/9481d653b60b81f4590a4d48f2be496f6962261e/benchmarks/primes.py) |

## 被使用

|                                                                 | Description                                                              |
|-----------------------------------------------------------------|--------------------------------------------------------------------------|
| [TIC-80](https://github.com/nesbox/TIC-80)                      | TIC-80 is a fantasy computer for making, playing and sharing tiny games. |
| [MiniPythonIDE](https://github.com/CU-Production/MiniPythonIDE) | A python ide base on pocketpy                                            |
| [py-js](https://github.com/shakfu/py-js)                        | Python3 externals for Max / MSP                                          |
| [crescent](https://github.com/chukobyte/crescent)               | Crescent is a cross-platform 2D fighting and beat-em-up game engine.     |

如果你想添加你的项目，请提交一个pr

## 贡献

任何形式的共享都是受欢迎的

- 提交pr
  - 修复错误
  - 添加新的特性
- 打开Issue
  - 任何的建议
  - 任何的疑问

如果你觉得pkpy有用，就给这个仓库一颗星星吧 (●'◡'●)

## 赞助这个项目

你可以通过以下途径赞助这个项目

+ [Github Sponsors](https://github.com/sponsors/blueloveTH)
+ [Buy me a coffee](https://www.buymeacoffee.com/blueloveth)

您的赞助将帮助我们不断发展pkpy。

## 参考

+ [cpython](https://github.com/python/cpython)
+ [byterun](http://qingyunha.github.io/taotao/)
+ [box2d](https://box2d.org/)

## 历史

[![Star History Chart](https://api.star-history.com/svg?repos=blueloveth/pocketpy&type=Date)](https://star-history.com/#blueloveth/pocketpy&Date)

## 开源协议

MIT License

