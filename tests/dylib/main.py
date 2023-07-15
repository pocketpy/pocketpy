import os
import sys

print('platform:', sys.platform)
print(os.getcwd())

if sys.platform == 'linux':
    test = __import__('build/linux/libtest.so')
elif sys.platform == 'win32':
    test = __import__('build/win32/test.dll')
else:
    raise Exception('Unsupported platform')

test.hello()