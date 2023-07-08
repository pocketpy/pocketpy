SRC=$(find ../../src/ -name "*.cpp")
clang++ -std=c++17 -fno-rtti -O2 -stdlib=libc++ -Wfatal-errors -o libtest.so test.cpp -I../../include -fPIC -shared