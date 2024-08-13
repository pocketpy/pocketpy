import traceback

try:
    a = {'123': 4}
    b = a[6]
except KeyError:
    actual = traceback.format_exc()

expected = '''Traceback (most recent call last):
  File "tests/80_traceback.py", line 5
    b = a[6]
KeyError: 6'''

if actual != expected:
    print('--- ACTUAL RESULT -----')
    print(actual)
    print('--- EXPECTED RESULT ---')
    print(expected)
    exit(1)