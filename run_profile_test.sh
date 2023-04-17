clang++ -O2 -std=c++17 -fno-rtti --coverage -stdlib=libc++ -Wall -o pocketpy src/main.cpp
time ./pocketpy benchmarks/fib.py
rm -rf .coverage
mkdir -p .coverage
llvm-cov-15 gcov main.gc -r -s src/ >> .coverage/coverage.txt
mv *.gcov .coverage
rm main.gc*
