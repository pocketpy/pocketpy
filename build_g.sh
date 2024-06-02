python prebuild.py

SRC=$(find src/ -name "*.cpp")

FLAGS="-std=c++17 -Og -stdlib=libc++ -Iinclude -frtti -Wfatal-errors -g -DDEBUG"

clang++ $FLAGS -o main src2/main.cpp $SRC
