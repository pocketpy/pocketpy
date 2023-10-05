#!/usr/bin/env bash

python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -pg -O1 -std=c++17 -stdlib=libc++ -Wfatal-errors -o main $SRC -Iinclude -ldl
time ./main benchmarks/fib.py
mv benchmarks/gmon.out .
gprof pocketpy gmon.out > gprof.txt
rm gmon.out
