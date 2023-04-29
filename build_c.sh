python3 preprocess.py

echo "compiling c++ lib"
clang++ -c -o pocketpy_c.o c_bindings/pocketpy_c.cpp -Wfatal-errors --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti -stdlib=libc++ -I src/
echo "compiling c executable"
clang -c -o main.o c_bindings/main.c -Wfatal-errors -O2 -Wall -Wno-sign-compare -Wno-unused-variable -I src/
echo "linking"
clang++ -o pocketpy_c main.o pocketpy_c.o -stdlib=libc++
echo "cleaning up"
rm pocketpy_c.o
rm main.o

