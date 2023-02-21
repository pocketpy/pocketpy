g++ -o pocketpy src/main.cpp --std=c++17 -pg -O2 -fno-rtti

./pocketpy benchmarks/fib.py

gprof pocketpy gmon.out > gprof.txt

rm gmon.out