cd c_bindings
rm -rf build
mkdir build
cd build
cmake ..
cmake --build . --config Release

./test_c_bindings > binding_test_scratch

echo "checking results (they should be identical)"
diff -q -s  binding_test_scratch ../test_answers.txt

if [ $? -eq 1 ]
then
    echo "ERROR: c binding test failed"
    exit 1
fi