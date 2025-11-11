try:
    import msgpack
except ImportError:
    print('msgpack is not enabled, skipping test...')
    exit()

a = {
    'a': 1,
    'b': 2,
    'c': None,
    'd': [1, 2, 3],
    'e': {
        'a': 100,
        'b': 2.5,
        'c': None,
        'd': [142, 2785, 39767],
    },
    "f": 'This is a string',
    'g': [True, False, None],
    'h': False
}

import msgpack

assert msgpack.loads(b'\x01') == 1
assert msgpack.loads(b'\xa11') == "1"
assert msgpack.loads(b'\xcb\x00\x00\x00\x00\x00\x00\x00\x00') == 0.0
assert msgpack.loads(b'\x92\x01\x02') == [1, 2]
assert msgpack.loads(b'\xc0') == None
assert msgpack.loads(b'\xc3') == True
assert msgpack.loads(b'\xc2') == False
assert msgpack.loads(b'\x80') == {}

_j = msgpack.dumps(a)
_a = msgpack.loads(_j)

for k, v in a.items():
    assert (a[k] == _a[k]), f'{a[k]} != {_a[k]}'

for k, v in _a.items():
    assert (a[k] == _a[k]), f'{a[k]} != {_a[k]}'

b = [1, 2, True, None, False]

_j = msgpack.dumps(b)
_b = msgpack.loads(_j)

assert b == _b

c = 1.0
_j = msgpack.dumps(c)
_c = msgpack.loads(_j)
assert c == _c

d = True
_j = msgpack.dumps(d)
_d = msgpack.loads(_j)
assert d == _d

# assert msgpack.dumps((1,)) == '[1]'
# assert msgpack.dumps((1, 2, 3)) == '[1, 2, 3]'
# assert msgpack.dumps(tuple()) == '[]'

assert msgpack.dumps([]) == b'\x90'
assert msgpack.dumps([1, 2, 3]) == b'\x93\x01\x02\x03'
assert msgpack.dumps([1]) == b'\x91\x01'

try:
    msgpack.dumps({1: 2})
    assert False
except TypeError:
    assert True

try:
    msgpack.dumps(type)
    assert False
except TypeError:
    assert True
