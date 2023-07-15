python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
FLAGS="-std=c++17 -fno-rtti -O2 -stdlib=libc++ -Wfatal-errors -Iinclude"

if [[ "$OSTYPE" == "darwin"* ]]; then
    LIB_EXTENSION=".dylib"
    FLAGS="$FLAGS -undefined dynamic_lookup"
else
    LIB_EXTENSION=".so"
fi
clang++ $FLAGS -o libpocketpy$LIB_EXTENSION $SRC -fPIC -shared -ldl

# compile main.cpp and link to libpocketpy.so
clang++ $FLAGS -o main src2/main.cpp -L. -lpocketpy -Wl,-rpath=.