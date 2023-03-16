clang++ -std=c++17 -fno-rtti --coverage -Wall -o pocketpy src/main.cpp
python3 scripts/run_tests.py
llvm-cov-15 gcov main.gc -r -s src/
rm -rf .coverage
mkdir -p .coverage
mv *.gcov .coverage