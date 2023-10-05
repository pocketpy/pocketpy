#!/bin/bash

echo -n "Running prebuild.py... "
python3 prebuild.py
echo "Done"

echo -n "Finding source files... "
SRC=$(find src/ -name "*.cpp")
echo "Done"

echo -n "Compiling and linking source files... "
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

if [ $? -eq 0 ]; then
    echo "Library build successful: libpocketpy$LIB_EXTENSION"
else
    echo "Library build failed."
    exit 1
fi

# compile main.cpp and link to libpocketpy.so
echo "Compiling main.cpp and linking to libpocketpy$LIB_EXTENSION..."
clang++ $FLAGS -o main src2/main.cpp -L. -lpocketpy $LINK_FLAGS

if [ $? -eq 0 ]; then
    echo "Build completed successfully."
else
    echo "Build failed."
    exit 1
fi
