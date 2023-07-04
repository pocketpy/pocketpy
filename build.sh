python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -std=c++17 -fno-rtti -O2 -stdlib=libc++ -Wfatal-errors -o pocketpy src2/main.cpp $SRC -Iinclude