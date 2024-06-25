set -e

python prebuild.py

SRC=$(find src/ -name "*.c")

FLAGS="-std=c11 -lm -Iinclude -O0 -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1" # -fsanitize=address,leak,undefined"

echo "Compiling C files..."
clang $FLAGS $SRC src2/main.c -o main

