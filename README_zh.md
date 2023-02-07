# PocketPy

<p>
<a title="Build" href="https://github.com/blueloveTH/pocketpy/actions/workflows" ><img src="https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
<a title="Pub" href="https://pub.dev/packages/pocketpy" ><img src="https://img.shields.io/pub/v/pocketpy" /></a>
<a title="Discord" href="https://discord.gg/WWaq72GzXv" ><img src="https://img.shields.io/discord/1048978026131640390" /></a>
</p>
PocketPy 是一个轻量级的Python解释器，为嵌入至游戏引擎而设计。

它包含一个编译器和基于字节码的虚拟机，以及交互式命令窗的实现。所有功能均集成在单个头文件`pocketpy.h`中，不包含外部依赖项，能很方便地嵌入至你的应用。

你可以 [在浏览器中体验](https://blueloveth.github.io/pocketpy) PocketPy的交互式界面（REPL）。

![sample_img](docs/sample.png)



## 快速上手

根据你所使用的语言和平台选择对应的插件。

#### C/C++

你可以在 [Github Release](https://github.com/blueloveTH/pocketpy/releases/latest) 页面下载`pocketpy.h`，并加入到你的工程中。请参考[C-API](https://pocketpy.dev/c-api/vm/)相关的说明。

```cpp
#include "pocketpy.h"

int main(){
    // 创建一个虚拟机
    VM* vm = pkpy_new_vm(true);
    // 执行代码
    pkpy_vm_exec(vm, "print('Hello world!')");
    return 0;
}
```

> 对于C++，你也可以使用VM类的方法来操作虚拟机。

#### Unity Engine

你可以在Unity资源商店下载PocketPy的C#插件。

https://assetstore.unity.com/packages/slug/241120

```csharp
using UnityEngine;

public class Test01 : MonoBehaviour
{
    void Start()
    {
        // 创建一个虚拟机
        pkpy.VM vm = new pkpy.VM();

        // 构造一个列表
        vm.exec("a = [1, 2, 3]");

        // 对列表进行求和
        string result = vm.eval("sum(a)");
        Debug.Log(result);   // 6

        // 打印变量`a`，并读取标准输出
        vm.exec("print(a)");
        pkpy.PyOutput o = vm.read_output();
        Debug.Log(o.stdout); // [1, 2, 3]

        // 构造一个函数绑定
        vm.bind("builtins", "test", (double x) => x+1);  
        Debug.Log(vm.eval("test(3.14)")); // '4.14'
    }
}
```

#### Flutter

执行下列命令安装pocketpy的[Flutter插件](https://pub.dev/packages/pocketpy)。

```
flutter pub add pocketpy
```

详细配置请参考 https://pocketpy.dev/getting-started/flutter/



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
| 类型标注     | `def  f(a: int, b : float = 1)` | YES  |



## 参考

+ [cpython](https://github.com/python/cpython)

+ [byterun](http://qingyunha.github.io/taotao/)

+ [emhash](https://github.com/ktprime/emhash)

## 开源协议

MIT License