set -e

python prebuild.py

SRC=$(find src/ -name "*.c")

gcc -pg -Og -std=c11 -Wfatal-errors -o main $SRC src2/main.c -Iinclude -lm -ldl -DNDEBUG -flto
./main benchmarks/fib.py
gprof main gmon.out > gprof.txt
rm gmon.out
