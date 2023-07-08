import os

print(os.getcwd())
test = __import__('libtest.so')

test.hello()