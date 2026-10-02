---
icon: dot
title: Bytecode Deployment
order: 81
---

# Bytecode deployment

pocketpy can compile scripts to `.pyc` files and run them without the original
source. This avoids parsing at load time; it does not make the executed
bytecode itself faster.

These files use **pocketpy's own format**, not CPython's `.pyc` format.
Compile and run with the same pocketpy revision and compatible build settings;
do not assume bytecode compatibility across interpreter updates.

## Compile and run a file

After [building the standalone interpreter](../quick-start.md):

```sh
./build/main --compile hello.py hello.pyc
./build/main hello.pyc
```

With the Visual Studio generator:

```powershell
.\build\Release\main.exe --compile hello.py hello.pyc
.\build\Release\main.exe hello.pyc
```

Supply both input and output paths. The `--compile` option cannot be combined
with `--debug` or `--profile`.

## Compile a directory

Run the repository's `compileall.py` with a host Python interpreter:

```sh
python compileall.py ./build/main scripts dist
```

It recursively compiles `scripts/*.py`, preserving subdirectories under
`dist/`. For example, `scripts/game/__init__.py` becomes
`dist/game/__init__.pyc`. Non-Python assets are not copied.

Keep the module/package layout when distributing the output.
The importer tries `.py` before `.pyc`, so a source file left beside the
bytecode takes precedence. See [module loading](../C-API/modules.md).

## Run bytecode from an embedded host

`py_execo(data, size, filename, module)` accepts a byte buffer and its length.
Pass `NULL` as the module to use `__main__`, and handle its boolean result
the same way as `py_exec()`. A custom `importfile` callback must set the
byte length when returning compiled modules.

## Diagnostics and distribution limits

Tracebacks retain filenames and line numbers, but bytecode does not include
the original source lines. Keep matching source files during debugging.

Bytecode is not encryption: constants and program structure remain available
for inspection. Distribute only code you intend the recipient to run, and
do not load untrusted bytecode. Keep credentials and other secrets outside
distributed scripts and bytecode.
