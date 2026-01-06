import traceback
import sys

if sys.argv[0].endswith('.pyc'):
    exit()

try:
    a = {'123': 4}
    b = a[6]
except KeyError:
    actual = traceback.format_exc()

expected = '''Traceback (most recent call last):
  File "tests/802_traceback.py", line 9
    b = a[6]
KeyError: 6'''

if actual != expected:
    print('--- ACTUAL RESULT -----')
    print(actual)
    print('--- EXPECTED RESULT ---')
    print(expected)
    exit(1)