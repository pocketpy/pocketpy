# pocketpy: python interpreter in 1 file

<p>
<a title="Build" href="https://github.com/blueloveTH/pocketpy/actions/workflows" ><img src="https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://codecov.io/gh/blueloveTH/pocketpy" > 
 <img src="https://codecov.io/gh/blueloveTH/pocketpy/branch/main/graph/badge.svg?token=TI9KAFL0RG"/> 
 </a>
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
</p>
pocketpy是一个轻量级的Python解释器，为嵌入至游戏引擎而设计，基于C++17和STL。

它包含一个编译器和基于字节码的虚拟机，以及交互式命令窗的实现。所有功能均集成在单个头文件`pocketpy.h`中，不包含外部依赖项，能很方便地嵌入至你的应用。

你可以 [在浏览器中体验](https://pocketpy.dev/static/web/) pocketpy的交互式界面（REPL）。

## 快速上手

你可以在 [Github Release](https://github.com/blueloveTH/pocketpy/releases) 页面下载`pocketpy.h`，
并加入到你的工程中。请参阅 https://pocketpy.dev 以获取更详细的文档。

如果你使用 [Unity引擎](https://unity.com/)，你可以在Asset Store下载我们的插件 [PocketPython](https://assetstore.unity.com/packages/tools/visual-scripting/pocketpy-241120)。

```cpp
#include "pocketpy.h"

int main(){
    // 创建一个虚拟机
    auto vm = pkpy_new_vm();
    
    // Hello world!
    pkpy_vm_exec(vm, "print('Hello world!')");

    // 构建一个列表
    pkpy_vm_exec(vm, "a = [1, 2, 3]");

    // 对列表进行求和
    pkpy_vm_exec(vm, "print(sum(a))");

    // 释放资源
    pkpy_delete_vm(vm);
    return 0;
}
```

## 支持的语法特性

| 特性         | 示例                            | 支持 |
| ------------ | ------------------------------- | ---- |
| 分支         | `if..else..elif`                | YES  |
| 循环         | `for/while/break/continue`      | YES  |
| 函数         | `def f(x,*args,y=1):`           | YES  |
| 类与继承     | `class A(B):`                   | YES  |
| 列表         | `[1, 2, 'a']`                   | YES  |
| 列表生成式   | `[i for i in range(5)]`         | YES  |
| 切片         | `a[1:2], a[:2], a[1:]`          | YES  |
| 元组         | `(1, 2, 'a')`                   | YES  |
| 字典         | `{'a': 1, 'b': 2}`              | YES  |
| 格式化字符串 | `f'value is {x}'`               | YES  |
| 序列解包     | `a, b = 1, 2`                   | YES  |
| 异常         | `raise/try..catch`              | YES  |
| 动态分发     | `eval()/exec()`                 | YES  |
| 反射         | `hasattr()/getattr()/setattr()` | YES  |
| 导入模块     | `import/from..import`           | YES  |
| 上下文管理器 | `with <expr> as <id>:`          | YES  |
| 类型标注 | `def  f(a:int, b:float=1)`      | YES       |
| 生成器       | `yield i`                       | YES       |
| 装饰器 | `@cache` | YES |

## 参考

+ [cpython](https://github.com/python/cpython)
+ [byterun](http://qingyunha.github.io/taotao/)

## 开源协议

MIT License

