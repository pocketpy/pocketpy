#!/bin/bash

# Check if clang++ is installed
if ! type -P clang++ >/dev/null 2>&1; then
    echo "clang++ is required and not installed. Kindly install it."
    echo "Run: sudo apt-get install libc++-dev libc++abi-dev clang"
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

SRC=$(find src/ -name "*.cpp")

echo "> Compiling and linking source files... "

FLAGS="-std=c++17 -O1 -stdlib=libc++ -frtti -Wfatal-errors -Iinclude"

if [[ "$OSTYPE" == "darwin"* ]]; then
    LIB_EXTENSION=".dylib"
    FLAGS="$FLAGS -undefined dynamic_lookup"
    LINK_FLAGS=""
else
    LIB_EXTENSION=".so"
    LINK_FLAGS="-Wl,-rpath=."
fi

clang++ $FLAGS -o libpocketpy$LIB_EXTENSION $SRC -fPIC -shared

# compile main.cpp and link to libpocketpy.so
echo "> Compiling main.cpp and linking to libpocketpy$LIB_EXTENSION..."

clang++ $FLAGS -o main -O1 src2/main.cpp -L. -lpocketpy $LINK_FLAGS

if [ $? -eq 0 ]; then
    echo "Build completed. Type \"./main\" to enter REPL."
else
    echo "Build failed."
    exit 1
fi
