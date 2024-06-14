python prebuild.py

SRC_C=$(find src/ -name "*.c")
SRC_CPP=$(find src/ -name "*.cpp")

COMMON_FLAGS="-Iinclude -O0 -Wfatal-errors -g -DDEBUG -DPK_ENABLE_OS=1"

FLAGS_C="-std=c11 $COMMON_FLAGS"
FLAGS_CPP="-std=c++17 -stdlib=libc++ -frtti $COMMON_FLAGS"

echo "Compiling C files..."
clang $FLAGS_C -c $SRC_C
ar rcs libpocketpy_c.a *.o
rm *.o

echo "Compiling C++ files..."
clang++ $FLAGS_CPP -o main src2/main.cpp $SRC_CPP libpocketpy_c.a 
rm libpocketpy_c.a
