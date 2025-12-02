set -e

python prebuild.py

SRC=$(find src/ -name "*.c")

clang -std=c11 --coverage -O1 -Wfatal-errors -o main src2/main.c $SRC -Iinclude -DPK_ENABLE_OS=1 -lm -ldl -DNDEBUG

python scripts/run_tests.py

# if prev error exit
if [ $? -ne 0 ]; then
    exit 1
fi

rm -rf .coverage
mkdir .coverage

UNITS=$(find ./ -name "*.gcno")
llvm-cov-17 gcov ${UNITS} -r -s include/ -r -s src/ >> .coverage/coverage.txt

mv *.gcov .coverage
rm *.gcda
rm *.gcno

# remove .gcno files if the 1st line contains "Source:src/debugger/"
find .coverage/ -type f -name "*.gcov" | while read -r file; do
    first_line=$(head -n 1 "$file")
    if [[ "$first_line" == *"Source:src/debugger/"* ]]; then
        echo "Removing: $file"
        rm "$file"
    fi
done
