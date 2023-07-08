python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -std=c++17 -fno-rtti --coverage -O1 -stdlib=libc++ -Wfatal-errors -o main src2/main.cpp $SRC -Iinclude
python3 scripts/run_tests.py
rm -rf .coverage
mkdir .coverage
UNITS=$(find ./ -name "*.gcno")
llvm-cov-15 gcov ${UNITS} -r -s include/ -r -s src/ >> .coverage/coverage.txt
mv *.gcov .coverage
rm *.gcda
rm *.gcno
