import os
import sys

print('platform:', sys.platform)
print(os.getcwd())

if sys.platform == 'linux':
    test = __import__('build/linux/libtest.so')
else:
    raise Exception('Unsupported platform')

test.hello()