clang++ -std=c++17 -fno-rtti --coverage -stdlib=libc++ -Wall -o pocketpy src/main.cpp
python3 scripts/run_tests.py
rm -rf .coverage
mkdir -p .coverage
llvm-cov-15 gcov main.gc -r -s src/ >> .coverage/coverage.txt
mv *.gcov .coverage
rm main.gc*
