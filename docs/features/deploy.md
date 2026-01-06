---
icon: dot
title: Deploy Bytecodes
order: 81
---

!!!
The feature requires pocketpy version >= `2.1.7`
!!!

You can deploy your pocketpy program as `.pyc` files, which are compiled bytecodes with necessary metadata.
This slightly improves the loading speed of your program.

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

## Running `.pyc` files

The command-line executable `main` can run `.pyc` files directly:

```sh
./main output_file.pyc
```

If you are using C-APIs, you can use the `py_execo()` function.

```c
/// Run a compiled code object.
PK_API bool py_execo(const void* data, int size, const char* filename, py_Ref module) PY_RAISE PY_RETURN;
```

## Trackback Support

Since `.pyc` files do not contain raw sources,
trackbacks will show line numbers but not the actual source code lines.
