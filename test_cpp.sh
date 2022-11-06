g++ -o pocketpy src/main.cpp --std=c++17 -pg -O1

./pocketpy tests/1.py

gprof pocketpy gmon.out > gprof.txt

#gprof pocketpy | gprof2dot | dot -Tsvg -o output.svg
rm gmon.out



