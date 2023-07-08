python3 prebuild.py
SRC=$(find src/ -name "*.cpp")
clang++ -pg -O1 -std=c++17 -fno-rtti -stdlib=libc++ -Wfatal-errors -o pocketpy $SRC -Iinclude
time ./pocketpy benchmarks/fib.py
mv benchmarks/gmon.out .
gprof pocketpy gmon.out > gprof.txt
rm gmon.out
