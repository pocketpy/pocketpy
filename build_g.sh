python prebuild.py

SRC_C=$(find src/ -name "*.c")
SRC_CPP=$(find src/ -name "*.cpp")
SRC="$SRC_C $SRC_CPP"

FLAGS="-std=c++17 -O0 -stdlib=libc++ -Iinclude -frtti -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1"

clang++ $FLAGS -o main src2/main.cpp $SRC

