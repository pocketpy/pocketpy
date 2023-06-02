import traceback

try:
    a = {'123': 4}
    b = a[6]
except KeyError:
    s = traceback.format_exc()

assert s == r'''Traceback (most recent call last):
KeyError: 6'''