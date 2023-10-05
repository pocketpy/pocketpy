#!/usr/bin/env bash

python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
FLAGS="-std=c++17 -O2 -stdlib=libc++ -Wfatal-errors -Iinclude"

if [[ "$OSTYPE" == "darwin"* ]]; then
    LIB_EXTENSION=".dylib"
    FLAGS="$FLAGS -undefined dynamic_lookup"
    LINK_FLAGS=""
else
    LIB_EXTENSION=".so"
    LINK_FLAGS="-Wl,-rpath=."
fi
clang++ $FLAGS -o libpocketpy$LIB_EXTENSION $SRC -fPIC -shared -ldl

# compile main.cpp and link to libpocketpy.so
clang++ $FLAGS -o main src2/main.cpp -L. -lpocketpy $LINK_FLAGS