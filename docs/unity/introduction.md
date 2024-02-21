---
label: Introduction
icon: dot
order: 30
---

# Welcome to PocketPython

PocketPython is a C# plugin that allows you to do Python scripting in Unity. It provides a sandboxed Python environment, which adds dynamic capabilities to your game, which can be used for dynamic game logic, modding, hot fixing, and more.

The virtual machine is written in pure C#, which means you can fully control the internal state of the Python interpreter.

!!!
PocketPython is designed for game scripting, not for scientific computing.
You cannot use it to run NumPy, OpenCV, or any other CPython extension modules.
!!!


## Features

### Python 3.x Syntax

PocketPython uses [pocketpy](https://github.com/pocketpy/pocketpy)
as frontend to parse and compile Python source code.
It supports most of the Python 3.x syntax.

The following table shows a feature comparison of PocketPython
with respect to the original [pocketpy](https://github.com/pocketpy/pocketpy).
The features marked with `YES` are supported, and the features marked with `NO` are not supported.

| Name            | Example                         | Cpp | Unity |
| --------------- | ------------------------------- | --------- | --- |
| If Else         | `if..else..elif`                | YES       | YES |
| Loop            | `for/while/break/continue`      | YES       | YES |
| Function        | `def f(x,*args,y=1):`           | YES       | YES |
| Subclass        | `class A(B):`                   | YES       | YES |
| List            | `[1, 2, 'a']`                   | YES       | YES |
| ListComp        | `[i for i in range(5)]`         | YES       | YES |
| Slice           | `a[1:2], a[:2], a[1:]`          | YES       | YES |
| Tuple           | `(1, 2, 'a')`                   | YES       | YES |
| Dict            | `{'a': 1, 'b': 2}`              | YES       | YES |
| F-String        | `f'value is {x}'`               | YES       | YES |
| Unpacking       | `a, b = 1, 2`                   | YES       | YES |
| Star Unpacking  | `a, *b = [1, 2, 3]`             | YES       | YES |
| Exception       | `raise/try..catch`              | YES       | NO |
| Dynamic Code    | `eval()/exec()`                 | YES       | YES |
| Reflection      | `hasattr()/getattr()/setattr()` | YES       | YES |
| Import          | `import/from..import`           | YES       | YES |
| Context Block   | `with <expr> as <id>:`          | YES       | NO |
| Type Annotation | `def  f(a:int, b:float=1)`      | YES       | YES |
| Generator       | `yield i`                       | YES       | NO |
| Decorator       | `@cache`                        | YES       | YES |

### Sandboxed Python Environment

PocketPython provides a sandboxed Python environment.
All python code is executed in a C# virtual machine.
The user cannot access the file system, network, or any other resources of the host machine.

### Seamless Interop with C#

PocketPython uses `object` in C# to represent dynamic typed Python objects.
Most of the basic Python types correspond to a C# type,
which means passing arguments between C# and Python is extremely easy and intuitive.

| Python Type | C# Type |
| ----------- | ------- |
| `None`      | `NoneType` |
| `object`    | `System.Object` |
| `bool`      | `System.Boolean` |
| `int`       | `System.Int32` |
| `float`     | `System.Single` |
| `str`       | `System.String` |
| `tuple`     | `System.Object[]` |
| `list`      | `System.Collections.Generic.List<object>` |
| `dict`      | `System.Collections.Generic.Dictionary<PyDictKey, object>` |
| ...         | ... |

### Python Console in Editor

PocketPython provides a Python console in Unity editor,
which allows you to do quick debugging and testing.