python3 preprocess.py

if [ ! -f "pocketpy_c.o" ] 
then
    echo "compiling c++ lib"
    clang++ -c -o pocketpy_c.o c_bindings/pocketpy_c.cpp -Wfatal-errors --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti -stdlib=libc++ -I src/ -fsanitize=address -g
else
    echo "DETECTED PREVIOUS COMPILATION USING IT"
fi

echo "compiling c executable" 
clang -c -o test.o c_bindings/test.c -Wfatal-errors -O2 -Wall -Wno-sign-compare -Wno-unused-variable -I src/ -fsanitize=address -g
echo "linking"
clang++ -o c_binding_test test.o pocketpy_c.o -stdlib=libc++ -fsanitize=address -g
echo "running, leaksanitizer is finding a false postive leak in the CVM constructor"
echo "ignore that but pay attention to anything else"
./c_binding_test > binding_test_scratch
echo "checking results (they should be identical)"
diff -q -s  binding_test_scratch c_bindings/test_answers.txt

echo "cleaning up"
rm pocketpy_c.o
rm test.o
rm binding_test_scratch
rm c_binding_test

