set -e

FLAGS="-std=c11 -Iinclude -O0 -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1"

# link libpocketpy.dylib
clang $FLAGS -shared -fPIC src2/hello.c -o libhello.dylib -L. -lpocketpy
