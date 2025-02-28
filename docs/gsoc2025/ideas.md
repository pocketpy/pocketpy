---
icon: light-bulb
order: 0
label: "Project Ideas"
---

### VSCode plugin for debugging pocketpy applications

+ Difficulty Level: 3/5 (Medium)
+ Skill: TypeScript; C
+ Project Length: Small

Community users have reported that there is no convenient way to debug python applications interpreted by pocketpy. Fortunately, VSCode provides a mechanism of [Debugger Extension](https://code.visualstudio.com/api/extension-guides/debugger-extension) that allows us to integrate pocketpy debugger into VSCode UI through Debug Adapter Protocol (DAP).

This project aims to develop a VSCode plugin like [Python Debugger](https://marketplace.visualstudio.com/items?itemName=ms-python.debugpy), which implements DAP for pocketpy. With this plugin, users can launch their pocketpy applications in VSCode with breakpoints, call stacks, and variable inspection. Students with experience in TypeScript will be helpful for this project.

### Develop `cTensor` library for neural networks

+ Difficulty Level: 4/5 (Hard)
+ Skill: C; Further Mathematics
+ Project Length: Medium

pocketpy is planning to provide a tensor library `cTensor` for users who want to integrate neural networks into their applications. `cTensor` implements automatic differentiation and dynamic compute graph. It allows users to train and deploy neural networks on client-side devices like mobile phones and microcontrollers (e.g. ESP32-C3). We have a early prototype located at [pocketpy/cTensor](https://github.com/pocketpy/cTensor).

In this project, students will help develop and test the `cTensor` library, which is written in C11. We expect students to have a good understanding of further mathematics and C programming.

> This year we also accept custom project ideas. If you are not interested in the above, you can propose your own idea and contact me via `blueloveth@foxmail.com`. We will discuss the feasibility and mentorship for your idea.

