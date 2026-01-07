---
icon: rocket
order: 10
label: "Application Guide"
---

Welcome to the Google Summer of Code 2026 application guide for pocketpy.
We are recruiting a student who is passionate about vibe coding and mobile game development.

See [Project Ideas (TBA)](./ideas.md) for more details about the project.

## Prerequisites

To apply for this project, you need to satisfy the following prerequisites:

+ You are a student enrolled in an accredited institution (university, college, etc.) pursuing a degree in computer science or a related field. And this is your first time participating in Google Summer of Code.
+ You have interest in vibe coding and mobile game development.
+ You are experienced in Python and backend technologies, such as FastAPI or Flask.
+ You are glad to learn mobile app development using frameworks like Flutter.

## Application steps

### Step 1

If you think you meet the prerequisites,
send an email to `blueloveth@foxmail.com` with the following information.

1. A brief introduction about yourself, including the most related open sourced project you have worked on before. It is highly recommended to attach your Github profile link.
2. Your understanding of this project and why you are capable of completing it.
3. Your free time during the whole GSoC period (From 2026-03-01 to 2026-08-31).

### Step 2

After you get a positive reply from us,
you need to complete 1~2 pull requests to pocketpy's repository on GitHub.
This is mandatory as it demonstrates your coding skills and commitment to the project.

### Step 3

Once your pull requests are merged,
we will guide you to write a full proposal
for the project you are going to work on during GSoC 2026.
This proposal will be submitted to Google for review.

## Build guide for pocketpy

First, you need to install these tools:

1. Python(>= 3.8), I am sure you already have it.
2. A C11 compiler, such as GCC, Clang or MSVC. If you are on Linux, `gcc` is already installed. If you are on Windows, you can install Visual Studio with C/C++ development tools.
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
pocketpy 2.1.7 (Jan  7 2026, 16:42:45) [64 bit] on darwin
https://github.com/pocketpy/pocketpy
Type "exit()" to exit.
>>> 
>>> "Hello, world"
'Hello, world'
```

## Coding style guide

See [Coding Style Guide](../coding-style-guide.md).

