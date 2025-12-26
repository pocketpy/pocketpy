set -e

# Use python3 if available, otherwise fall back to python
PYTHON=$(command -v python3 >/dev/null 2>&1 && echo python3 || echo python)

$PYTHON prebuild.py

SRC=$(find src/ -name "*.c")

gcc -pg -Og -std=c11 -Wfatal-errors -o main $SRC src2/main.c -Iinclude -lm -ldl -DNDEBUG -flto
./main benchmarks/fib.py
gprof main gmon.out > gprof.txt
rm gmon.out
