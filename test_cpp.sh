g++ -o pocketpy src/main.cpp --std=c++17 -pg -O1 -pthread -fno-rtti

./pocketpy tests/1.py

gprof pocketpy gmon.out > gprof.txt

rm gmon.out