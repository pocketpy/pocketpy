---
icon: light-bulb
order: 0
label: "Project Ideas"
---

### Implement pybind11 for bindings

+ Difficulty Level: 5/5 (Hard)
+ Skill: Advanced C++ with metaprogramming; Python
+ Mentor: [blueloveTH](https://github.com/blueloveTH)
+ Project Length: Medium (~180 hours)

pocketpy has provided a low-level API for creating bindings. It is fast, lightweight and easy to debug.
However, it still requires a lot of boilerplate code to create bindings for complex C++ classes.
The community has long expected a high-level API for creating bindings.

[pybind11](https://github.com/pybind/pybind11)
is the most popular C++ library for creating Python bindings for CPython. A bunch of Python libraries are using it. pybind11 adopts a template metaprogramming approach to automatically generate bindings for C++ classes.

Our goal is to introduce a pybind11 compatible solution to pocketpy as an alternative way to create bindings
for functions and classes.
You can use C\+\+17 features to implement it, instead of C++11 used in pybind11.

### Add `numpy` module

+ Difficulty Level: 4/5 (Intermediate)
+ Skill: Intermediate C++; Python; Linear Algebra
+ Mentor: [zhs628](https://github.com/zhs628)
+ Project Length: Small (~120 hours)

Though pocketpy is designed for game scripting,
some people are using it for scientific computing.
It would be nice to have a `numpy` module in pocketpy.

We know `numpy` is a huge project.
Our goal is to implement a most commonly used subset of `numpy` in pocketpy.
You can mix C++ and Python code to simplify the overall workloads.

### Port secret labs' regex engine to pocketpy

+ Difficulty Level: 5/5 (Hard)
+ Skill: Advanced C++; Regular Expression; Algorithm; CPython Details
+ Mentor: TBA
+ Project Length: Medium (~180 hours)

pocketpy does not have `re` module yet.
We have considered other regex engines, such as [PCRE](https://www.pcre.org/), [RE2](https://github.com/google/re2) and `<regex>` in C++11. However, none of them is compatible with CPython's regex syntax.
Because CPython uses its special regex engine, a.k.a. [Secret Labs' Regular Expression Engine](https://github.com/python/cpython/tree/main/Modules/_sre).

In order to make pocketpy compatible with CPython in `re` module,
we need to port this engine into pocketpy.
