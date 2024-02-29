python prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -pg -O1 -std=c++17 -stdlib=libc++ -frtti -Wfatal-errors -o main $SRC src2/main.cpp -Iinclude
time ./main benchmarks/fib.py
mv benchmarks/gmon.out .
gprof main gmon.out > gprof.txt
rm gmon.out
