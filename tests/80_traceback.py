import traceback

try:
    a = {'123': 4}
    b = a[6]
except KeyError:
    s = traceback.format_exc()

ok = s == '''Traceback (most recent call last):
  File "80_traceback.py", line 5
    b = a[6]
KeyError: 6'''

assert ok, s