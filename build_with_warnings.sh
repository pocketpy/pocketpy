SRC=$(find src/ -name "*.cpp")

FLAGS="-std=c++17 -O1 -stdlib=libc++ -Iinclude -frtti -W -Wno-unused-parameter -Wno-sign-compare"

clang++ $FLAGS -o main -O1 src2/main.cpp $SRC
