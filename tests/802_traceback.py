import traceback
import sys

if sys.argv[0].endswith('.pyc'):
    exit()

try:
    a = {'123': 4}
    b = a[6]
except KeyError:
    actual = traceback.format_exc()
    assert traceback.print_exc() is None
    assert traceback.format_exc() == actual

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


def format_from_helper():
    return traceback.format_exc()


def format_from_try():
    try:
        return format_from_helper()
    except Exception:
        assert False


assert traceback.format_exc() is None
assert format_from_try() is None
assert traceback.print_exc() is None

try:
    raise ValueError('outer')
except ValueError:
    outer = traceback.format_exc()
    assert outer.endswith('ValueError: outer')
    assert format_from_helper() == outer
    assert format_from_try() == outer

    # A try body has no exception of its own and must not hide its handler.
    try:
        assert traceback.format_exc() == outer
        assert format_from_try() == outer
        raise KeyError('inner')
    except KeyError:
        inner = traceback.format_exc()
        assert inner.endswith("KeyError: 'inner'")
        assert format_from_try() == inner

    # Leaving the inner handler restores the outer exception.
    assert traceback.format_exc() == outer
    assert format_from_try() == outer

assert traceback.format_exc() is None
assert format_from_try() is None


def handle_in_helper():
    try:
        raise RuntimeError('helper')
    except RuntimeError:
        actual = traceback.format_exc()
        assert actual.endswith('RuntimeError: helper')
        assert format_from_try() == actual


try:
    raise ValueError('caller')
except ValueError:
    caller = traceback.format_exc()
    handle_in_helper()
    assert traceback.format_exc() == caller
    assert format_from_try() == caller


class BrokenStr(Exception):
    def __str__(self):
        raise RuntimeError('str failed')


try:
    raise BrokenStr()
except BrokenStr:
    actual = traceback.format_exc()
    assert actual.endswith('BrokenStr: <exception str() failed>')
    assert format_from_try() == actual
    assert traceback.print_exc() is None
    assert traceback.format_exc() == actual

assert traceback.format_exc() is None
