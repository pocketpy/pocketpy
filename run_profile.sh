python prebuild.py

SRC_C=$(find src/ -name "*.c")
SRC_CPP=$(find src/ -name "*.cpp")
SRC="$SRC_C $SRC_CPP"

g++ -pg -Og -std=c++17 -frtti -Wfatal-errors -o main $SRC src2/main.cpp -Iinclude
./main benchmarks/fib.py
gprof main gmon.out > gprof.txt
rm gmon.out
