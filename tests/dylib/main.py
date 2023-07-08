import os

print(os.getcwd())
test = __import__('build/libtest.so')

test.hello()