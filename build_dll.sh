set -e

FLAGS="-std=c11 -Iinclude -O0 -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1"

clang $FLAGS -shared -fPIC src2/hello.c -o libhello.dylib -undefined dynamic_lookup

