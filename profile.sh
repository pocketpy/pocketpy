g++ -o pocketpy src/main.cpp --std=c++17 -pg -O1 -fno-rtti

./pocketpy benchmarks/simple.py

gprof pocketpy gmon.out > gprof.txt

rm gmon.out