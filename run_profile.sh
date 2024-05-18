python prebuild.py
SRC=$(find src/ -name "*.cpp")
g++ -pg -Og -std=c++17 -frtti -Wfatal-errors -o main $SRC src2/main.cpp -Iinclude
./main benchmarks/fib.py
gprof main gmon.out > gprof.txt
rm gmon.out
