---
icon: dot
title: Deploy Bytecodes
order: 81
---

!!!
The feature requires pocketpy version >= `2.1.7`
!!!

You can deploy your pocketpy program as bytecode files, which slightly improves the loading speed of your program.

It also makes your users unable to get your source code directly, unless they do expensive reverse engineering.

To compile a `.py` file into a `.pyc` bytecode file, you need the command-line executable `main`,
which can be simply built by running `python cmake_build.py` in the repository root.


## Example

Once you have `main` executable, you can run the following command to compile `input_file.py`:

```sh
./main --compile input_file.py output_file.pyc
```

Alternatively, you can invoke the `compileall.py` script in the repository root.
It compiles all `.py` files in the specified directory into `.pyc` files.

```sh
python compileall.py ./main input_path output_path
```