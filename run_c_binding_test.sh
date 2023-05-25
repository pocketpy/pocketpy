python3 preprocess.py

echo "compiling c++ lib"
clang++ -c -o pocketpy_c.o c_bindings/pocketpy_c.cpp -Wfatal-errors --std=c++17 -O2 -Wall -Wno-sign-compare -Wno-unused-variable -fno-rtti -stdlib=libc++ -I src/ -g

echo "compiling c executable" 
clang -c -o test.o c_bindings/test.c -Wfatal-errors -O2 -Wall -Wno-sign-compare -Wno-unused-variable -I src/ -g
echo "linking"
clang++ -o c_binding_test test.o pocketpy_c.o -stdlib=libc++ -g
./c_binding_test > binding_test_scratch
echo "checking results (they should be identical)"
diff -q -s  binding_test_scratch c_bindings/test_answers.txt
if [ $? -eq 1 ]
then
    echo "ERROR: c binding test failed"
    exit 1
fi

echo "cleaning up"
rm pocketpy_c.o
rm test.o
rm binding_test_scratch
rm c_binding_test

