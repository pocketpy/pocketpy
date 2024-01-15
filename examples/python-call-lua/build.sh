SRC=$(find lua51/ -name "*.c")

# build liblua51.a first
gcc -O1 -c $SRC -std=c11 -Wfatal-errors -Ilua51
ar rcs liblua51.a *.o
rm *.o

# build pocketpy
SRC=$(find ../../src/ -name "*.cpp")
g++ -o main -O1 \
    $SRC ../../3rd/lua_bridge/src/lua_bridge.cpp main.cpp \
    -std=c++17 -Wfatal-errors \
    -Ilua51 -I../../include -I../../3rd/lua_bridge/include \
    -L. -llua51
