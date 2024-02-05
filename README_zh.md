# pocketpy: python interpreter in 1 file

<p>
<a title="Build" href="https://github.com/pocketpy/pocketpy/actions/workflows" ><img src="https://github.com/pocketpy/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://codecov.io/gh/pocketpy/pocketpy" > 
 <img src="https://codecov.io/gh/pocketpy/pocketpy/branch/main/graph/badge.svg?token=TI9KAFL0RG"/> 
 </a>
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
</p>
pocketpy 是一个轻量级的 Python 解释器，为嵌入至游戏引擎而设计，基于 C++17 和 STL。

它包含一个编译器和基于字节码的虚拟机，以及交互式命令窗的实现。所有功能均集成在单个头文件 `pocketpy.h` 中，不包含外部依赖项，能很方便地嵌入至你的应用。

你可以 [在浏览器中体验](https://pocketpy.dev/static/web/) pocketpy 的交互式界面（REPL）。

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

## 参考

+ [cpython](https://github.com/python/cpython)
+ [byterun](http://qingyunha.github.io/taotao/)

## 开源协议

MIT License

