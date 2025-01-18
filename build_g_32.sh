set -e

python prebuild.py

SRC=$(find src/ -name "*.c")

FLAGS="-std=c11 -lm -ldl -I3rd/lz4 -Iinclude -O0 -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1 -DPK_BUILD_MODULE_LZ4"

SANITIZE_FLAGS="-fsanitize=address,leak,undefined"

if [ "$(uname)" == "Darwin" ]; then
    SANITIZE_FLAGS="-fsanitize=address,undefined"
fi

echo "Compiling C files..."
gcc -m32 $FLAGS $SANITIZE_FLAGS $SRC src2/main.c 3rd/lz4/lz4libs/lz4.c -o main

