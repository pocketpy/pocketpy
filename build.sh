#!/bin/bash

# Check if python3 is installed
if ! type -P python3 >/dev/null 2>&1; then
    echo "python3 is required and not installed. Kindly install it."
    echo "Run: sudo apt install python3"
    exit 1
fi

# Check if clang++ is installed
if ! type -P clang++ >/dev/null 2>&1; then
    echo "clang++ is required and not installed. Kindly install it."
    echo "Run: sudo apt-get install libc++-dev libc++abi-dev clang++"
    exit 1
fi

echo "Requirements satisfied: python3 and clang++ are installed."
echo "It takes a moment to finish building."
echo ""
echo "> Running prebuild.py... "

python3 prebuild.py

# echo -n "Finding source files... "
SRC=$(find src/ -name "*.cpp")
# echo "Done"

echo "> Compiling and linking source files... "

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
echo "> Compiling main.cpp and linking to libpocketpy$LIB_EXTENSION..."

clang++ $FLAGS -o main src2/main.cpp -L. -lpocketpy $LINK_FLAGS

if [ $? -eq 0 ]; then
    echo "Build completed successfully. To use pocketpy, run : ./main"
else
    echo "Build failed."
    exit 1
fi
