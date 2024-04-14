---
icon: dot
title: Precompiling
---

pkpy allows you to precompile python code into two special forms, which can be executed later.

### In-memory precompilation

You can use `vm->compile` to compile your source code into a `CodeObject_` object.
This object can be executed later by `vm->_exec`.

```cpp
CodeObject_ code = vm->compile("print('Hello, world!')", "<string>", EXEC_MODE);
vm->_exec(code);        // Hello, world!
```

This `CodeObject_` object is a very non-generic form of the compiled code,
which is an in-memory form. Very efficient, but not portable.
You are not able to save it to a file or load it from a file.


### String precompilation

In order to save the compiled code to a file, you need to use `vm->precompile`.
It does some basic preprocessing and outputs the result as a human-readable string.

```cpp
// precompile the source code into a string
Str source = vm->precompile("print('Hello, world!')", "<string>", EXEC_MODE);

CodeObject code = vm->compile(source, "<string>", EXEC_MODE);
vm->_exec(code);        // Hello, world!
```

You can also use python's `compile` function to achieve the same effect.

```python
code = compile("print('Hello, world!')", "<string>", "exec")
exec(code)        # Hello, world!
```

Let's take a look at the precompiled string.
```python
print(code)
```

```txt
pkpy:1.4.5
0
=1
print
=6
5,1,0,
6,0,,,
42,,1,
8,,,S48656c6c6f2c20776f726c6421
43,,0,
3,,,

```

Comparing with **In-memory precompilation**,
**String precompilation** drops most of the information of the original source code.
It has an encryption effect, which can protect your source code from being stolen.
This also means there is no source line information when an error occurs.

```python
src = """
def f(a, b):
    return g(a, b)

def g(a, b):
    c = f(a, b)
    d = g(a, b)
    return c + d
"""

code = compile(src, "<exec>", "exec")
exec(code)
f(1, 2)
```

You will get this (without source line information):
```txt
Traceback (most recent call last):
  File "<exec>", line 3, in f
  File "<exec>", line 6, in g
  File "<exec>", line 3, in f
  File "<exec>", line 6, in g
  File "<exec>", line 3, in f
  File "<exec>", line 6, in g
  File "<exec>", line 3, in f
StackOverflowError
```

instead of this (with source line information):

```txt
Traceback (most recent call last):
  File "<stdin>", line 2, in f
    return g(a, b)
  File "<stdin>", line 2, in g
    c = f(a, b)
  File "<stdin>", line 2, in f
    return g(a, b)
  File "<stdin>", line 2, in g
    c = f(a, b)
  File "<stdin>", line 2, in f
    return g(a, b)
  File "<stdin>", line 2, in g
    c = f(a, b)
  File "<stdin>", line 2, in f
    return g(a, b)
StackOverflowError
```

!!!
String compilation has no guarantee of compatibility between different versions of pkpy.
!!!

You can use this snnipet to convert every python file in a directory into precompiled strings.

```python
# precompile.py
import sys, os

def precompile(filepath: str):
    """Precompile a python file inplace"""
    print(filepath)
    with open(filepath, 'r') as f:
        source = f.read()
    source = compile(source, filepath, 'exec')
    with open(filepath, 'w') as f:
        f.write(source)

def traverse(root: str):
    """Traverse a directory and precompile every python file"""
    for entry in os.listdir(root):
        entrypath = os.path.join(root, entry)
        if os.path.isdir(entrypath):
            traverse(entrypath)
        elif entrypath.endswith(".py"):
            precompile(entrypath)
```
