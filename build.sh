#!/bin/bash

# Check if clang is installed
if ! type -P clang >/dev/null 2>&1; then
    echo "clang is required and not installed. Kindly install it."
    echo "Run: sudo apt-get install clang"
    exit 1
fi

echo "> Running prebuild.py... "
python prebuild.py

if [ $? -ne 0 ]; then
    echo "prebuild.py failed."
    exit 1
fi

SRC=$(find src/ -name "*.c")
SRC2=${1:-src2/main.c}

echo "> Compiling and linking source files... "

clang -std=c11 -O2 -Wfatal-errors -Iinclude -DNDEBUG -o main $SRC $SRC2 -lm -ldl -lpthread

if [ $? -eq 0 ]; then
    echo "Build completed. Type \"./main\" to enter REPL."
else
    echo "Build failed."
    exit 1
fi
