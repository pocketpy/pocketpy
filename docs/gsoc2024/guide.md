---
icon: rocket
order: 10
label: "Application Guide"
---

Before starting, please read the [Ideas](./ideas.md) page and choose a project you are interested in.
Set up a C++ compiler, clone pocketpy sources from github and try to build.
This helps you confirm that your skills and experience match the requirements of the project.

### Build guide for beginners

First, you need to install these tools:

1. Python(>= 3.8), I am sure you already have it.
2. A C++ compiler, such as GCC, Clang or MSVC. If you are on Linux, `gcc` and `g++` are already installed. If you are on Windows, you can install Visual Studio with C++ development tools.
3. CMake(>= 3.10), a cross-platform build tool. You can use `pip install cmake` to install it.

Then, clone pocketpy sources from github and try to build:
```bash
git clone https://github.com/pocketpy/pocketpy
cd pocketpy

python cmake_build.py
```

If everything goes well, you will get a `main` executable (`main.exe` on Windows) in the root directory of pocketpy.
Simply run it and you will enter pocketpy's REPL.
```txt
pocketpy 1.4.0 (Jan 24 2024, 12:39:13) [32 bit] on emscripten
https://github.com/pocketpy/pocketpy
Type "exit()" to exit.
>>>
>>> "Hello, world"
'Hello, world'
```

### Application guide

**Your need to send an email to `blueloveth@foxmail.com` with the following information:**

1. A brief introduction about yourself, including the most related open sourced project you have worked on before. It is highly recommended to attach your Github profile link.
2. A technical proposal for the project you are interested in working on, including:
    + Your understanding of the project.
    + The technical approach/architecture you will adopt.
    + The challenges you might face and how you will overcome them.
3. A timeline for the project, including the milestones and deliverables.
4. Other information required by the Google Summer of Code program.

### Coding style guide

See [Coding Style Guide](../coding-style-guide.md).

### Contact us

If you have any questions, you can join our [Discord](https://discord.gg/WWaq72GzXv)
or contact me via email.
We are glad to help you with your application.
