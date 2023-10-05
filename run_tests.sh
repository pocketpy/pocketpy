#!/usr/bin/env bash

python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -std=c++17 --coverage -O1 -stdlib=libc++ -Wfatal-errors -o main src2/main.cpp $SRC -Iinclude -ldl
python3 scripts/run_tests.py

# if prev error exit
if [ $? -ne 0 ]; then
    exit 1
fi

rm -rf .coverage
mkdir .coverage
rm pocketpy_c.gcno
UNITS=$(find ./ -name "*.gcno")
llvm-cov-15 gcov ${UNITS} -r -s include/ -r -s src/ >> .coverage/coverage.txt
mv *.gcov .coverage
rm *.gcda
rm *.gcno
