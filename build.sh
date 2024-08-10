#!/bin/bash

# Check if clang is installed
if ! type -P clang >/dev/null 2>&1; then
    echo "clang is required and not installed. Kindly install it."
    echo "Run: sudo apt-get install clang"
    exit 1
fi

echo "It takes a moment to finish building."
echo ""
echo "> Running prebuild.py... "

python prebuild.py

if [ $? -ne 0 ]; then
    echo "prebuild.py failed."
    exit 1
fi

SRC=$(find src/ -name "*.c")

echo "> Compiling and linking source files... "

FLAGS="-std=c11 -O1 -Wfatal-errors -Iinclude -DNDEBUG"

if [[ "$OSTYPE" == "darwin"* ]]; then
    LIB_EXTENSION=".dylib"
    FLAGS="$FLAGS -undefined dynamic_lookup"
    LINK_FLAGS=""
else
    LIB_EXTENSION=".so"
    LINK_FLAGS="-Wl,-rpath=."
fi

clang $FLAGS -o libpocketpy$LIB_EXTENSION $SRC -fPIC -shared -lm

# compile main.cpp and link to libpocketpy.so
echo "> Compiling main.c and linking to libpocketpy$LIB_EXTENSION..."

clang $FLAGS -o main -O1 src2/main.c -L. -lpocketpy $LINK_FLAGS

if [ $? -eq 0 ]; then
    echo "Build completed. Type \"./main\" to enter REPL."
else
    echo "Build failed."
    exit 1
fi
