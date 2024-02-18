SRC=$(find src/ -name "*.cpp")

FLAGS="-std=c++17 -O1 -stdlib=libc++ -Iinclude -W -Wno-unused-parameter"

clang++ $FLAGS -o main -O1 src2/main.cpp $SRC
