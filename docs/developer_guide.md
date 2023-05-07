---
icon: book
order: 0
label: Developer guide
---

There are some scripts to help you develop this project.

## Build scripts

`build.py` is the main script to build the project.
```bash
# equivalent to `python build.py linux`
python build.py

# build for linux executable or `.so` library
python build.py linux [-lib]

# build for windows executable or `.dll` library
python build.py windows [-lib]

# build for web (wasm)
python build.py web
```

## Test scripts

```bash
# run unit tests
python scripts/run_tests.py

# run benchmarks
python scripts/run_tests.py benchmarks/
```

## Distribution scripts

```bash
python amalgamate.py
```

It will generate a single `pocketpy.h` and `main.cpp` in `amalgamate/` directory.