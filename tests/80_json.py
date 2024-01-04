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

try:
    import cjson as json
    print('[INFO] cjson is used')
except ImportError:
    import json

assert json.loads("1") == 1
assert json.loads('"1"') == "1"
assert json.loads("0.0") == 0.0
assert json.loads("[1, 2]") == [1, 2]
assert json.loads("null") == None
assert json.loads("true") == True
assert json.loads("false") == False
assert json.loads("{}") == {}

assert json.loads(b"false") == False

_j = json.dumps(a)
_a = json.loads(_j)

for k, v in a.items():
    assert (a[k] == _a[k]), f'{a[k]} != {_a[k]}'

for k, v in _a.items():
    assert (a[k] == _a[k]), f'{a[k]} != {_a[k]}'

b = [1, 2, True, None, False]

_j = json.dumps(b)
_b = json.loads(_j)

assert b == _b

c = 1.0
_j = json.dumps(c)
_c = json.loads(_j)
assert c == _c

d = True
_j = json.dumps(d)
_d = json.loads(_j)
assert d == _d


assert repr((1,)) == '(1,)'
assert repr((1, 2, 3)) == '(1, 2, 3)'
assert repr(tuple()) == '()'
assert json.dumps((1,)) == '[1]'
assert json.dumps((1, 2, 3)) == '[1, 2, 3]'
assert json.dumps(tuple()) == '[]'

assert repr([]) == '[]'
assert repr([1, 2, 3]) == '[1, 2, 3]'
assert repr([1]) == '[1]'
assert json.dumps([]) == '[]'
assert json.dumps([1, 2, 3]) == '[1, 2, 3]'
assert json.dumps([1]) == '[1]'

try:
    json.dumps({1: 2})
    assert False
except TypeError:
    assert True

try:
    json.dumps(type)
    assert False
except TypeError:
    assert True
