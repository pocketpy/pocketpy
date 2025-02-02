---
icon: light-bulb
order: 0
label: "Project Ideas"
---

> GSoC has not started yet. Please don't send application email or ask about it in community Discord server until 2025/04.

### Porting LDTK importer for python games

+ Difficulty Level: 2/5 (Easy)
+ Skill: Python; Haxe
+ Project Length: Small

[LDTK](https://ldtk.io/) is a modern 2D level editor, created by the director of Dead Cells. It is free and open-source, used by many game developers.
LDTK exports raw level data in JSON format, which can be further parsed by game frameworks. Currently, there is no convenient LDTK importer library in python (except the [QuickType](https://ldtk.io/api/#Python) loader, which has very limited functionality because it wraps the JSON schema only).

This project aims to develop a full-featured python library for importing LDTK levels, with advanced support of [Auto Tiles](https://ldtk.io/wp-content/uploads/2020/11/autoLayer-demo2.gif) for games with random map generation. The library should be written in pure python, compatible with pocketpy and cpython. If successful, it will be published on [PyPI](https://pypi.org/) and benefit all python game developers.

### Development `cTensor` library for machine learning

+ Difficulty Level: 3/5 (Medium)
+ Skill: C; Further Mathematics
+ Project Length: Medium

pocketpy is planning to provide a tensor library `cTensor` for users who want to integrate neural networks into their applications. `cTensor` implements automatic differentiation and dynamic compute graph. It allows users to train and deploy neural networks on client-side devices like mobile phones and microcontrollers (e.g. ESP32-C3). We have a early prototype located at [pocketpy/cTensor](https://github.com/pocketpy/cTensor).
In this project, students will help develop and test the `cTensor` library, which is written in C11. We expect students to have a good understanding of further mathematics and C programming.

### VSCode plugin for debugging pocketpy applications

+ Difficulty Level: 3/5 (Medium)
+ Skill: TypeScript; C
+ Project Length: Small

Community users have reported that there is no convenient way to debug python applications interpreted by pocketpy. Fortunately, VSCode provides a mechanism of [Debugger Extension](https://code.visualstudio.com/api/extension-guides/debugger-extension) that allows us to integrate pocketpy debugger into VSCode UI through Debug Adapter Protocol (DAP).

This project aims to develop a VSCode plugin like [Python Debugger](https://marketplace.visualstudio.com/items?itemName=ms-python.debugpy), which implements DAP for pocketpy. With this plugin, users can launch their pocketpy applications in VSCode with breakpoints, call stacks, and variable inspection. Students with experience in TypeScript will be helpful for this project.
