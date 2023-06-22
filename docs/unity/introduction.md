---
label: Introduction
icon: dot
order: 30
---

# Welcome to PocketPyUnity

PocketPyUnity is a C# plugin that allows you to do Python scripting in [Unity](https://unity.com/).
It provides a sandboxed Python environment, adding dynamic capabilities to your game,
which can be used for dynamic game logic, modding, hot fixing, and more.

The virtual machine is written in **pure C#**,
which means you can fully control the internal state of the Python interpreter.

!!!
PocketPyUnity is designed for game scripting, not for scientific computing.
You cannot use it to run NumPy, OpenCV, or any other CPython extension modules.
!!!