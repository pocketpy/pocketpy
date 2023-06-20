# pocketpy `v0.9.3` LTS

<p>
<a title="Build" href="https://github.com/blueloveTH/pocketpy/actions/workflows" ><img src="https://github.com/blueloveTH/pocketpy/actions/workflows/main.yml/badge.svg" /></a>
<a href="https://github.com/blueloveth/pocketpy/blob/main/LICENSE">
<img alt="GitHub" src="https://img.shields.io/github/license/blueloveth/pocketpy.svg?color=blue"></a>
<a href="https://github.com/blueloveth/pocketpy/releases">
<img alt="GitHub release" src="https://img.shields.io/github/release/blueloveth/pocketpy.svg"></a>
<a title="Pub" href="https://pub.dev/packages/pocketpy" ><img src="https://img.shields.io/pub/v/pocketpy" /></a>
</p>

pocketpy is a lightweight(~5000 LOC) Python interpreter.
And this branch is a long-term support version based on `v0.9.3`.

It is extremely easy to embed. Including a compiler, optimizer and bytecode virtual machine. All of them are available in a single header file `pocketpy.h`, without external dependencies.

Please see https://pocketpy.dev for details or try [Live Demo](https://blueloveth.github.io/pocketpy).

![sample_img](docs/sample.png)

## Features

| Name            | Example                         | Supported |
| --------------- | ------------------------------- | --------- |
| If Else         | `if..else..elif`                | YES       |
| Loop            | `for/while/break/continue`      | YES       |
| Function        | `def f(x,*args,y=1):`           | YES       |
| Subclass        | `class A(B):`                   | YES       |
| List            | `[1, 2, 'a']`                   | YES       |
| ListComp        | `[i for i in range(5)]`         | YES       |
| Slice           | `a[1:2], a[:2], a[1:]`          | YES       |
| Tuple           | `(1, 2, 'a')`                   | YES       |
| Dict            | `{'a': 1, 'b': 2}`              | YES       |
| F-String        | `f'value is {x}'`               | YES       |
| Unpacking       | `a, b = 1, 2`                   | YES       |
| Star Unpacking  | `a, *b = [1, 2, 3]`             | YES       |
| Exception       | `raise/try..catch`              | YES       |
| Dynamic Code    | `eval()/exec()`                 | YES       |
| Reflection      | `hasattr()/getattr()/setattr()` | YES       |
| Import          | `import/from..import`           | YES       |
| Context Block   | `with <expr> as <id>:`          | YES       |
| Type Annotation | `def  f(a:int, b:float=1)`      | YES       |
| Generator       | `yield i`                       | YES       |
| Decorator       | `@cache`                        | YES       |

## Getting Started

#### C/C++

For C/C++ developers, you can download the `pocketpy.h` on our GitHub release page.

https://github.com/blueloveTH/pocketpy/releases/latest

Check [C-API](https://pocketpy.dev/c-api/vm/) for references. For further customization, you can use [C++ API](https://pocketpy.dev/getting-started/cpp/).

```cpp
#include "pocketpy.h"

int main(){
    // Create a virtual machine
    VM* vm = pkpy_new_vm(true);
    
    // Hello world!
    pkpy_vm_exec(vm, "print('Hello world!')");

    // Create a list
    pkpy_vm_exec(vm, "a = [1, 2, 3]");

    // Eval the sum of the list
    char* result = pkpy_vm_eval(vm, "sum(a)");
    printf("%s", result);   // 6

    // Free the resources
    pkpy_delete(result);
    pkpy_delete(vm);
    return 0;
}
```

## Contribution

All kinds of contributions are welcome.

- Submit a Pull Request
  - fix a bug
  - add a new feature
- Open an Issue
  - any suggestions
  - any questions

Check our [Coding Style Guide](https://pocketpy.dev/coding_style_guide/) if you want to contribute C++ code.

## Reference

+ [cpython](https://github.com/python/cpython)

  The official implementation of Python programming language.

+ [byterun](https://www.aosabook.org/en/500L/a-python-interpreter-written-in-python.html)

  An excellent learning material. It illustrates how Python's virtual machine works.


## License

pocketpy is licensed under the [MIT License](http://opensource.org/licenses/MIT).
