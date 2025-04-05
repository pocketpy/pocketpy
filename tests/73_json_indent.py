import json

# test empty
assert json.dumps({}, indent=2) == '{}'
assert json.dumps([], indent=2) == '[]'
assert json.dumps(object().__dict__, indent=2) == '{}'

assert json.dumps([1, 2, [3, 4], 5], indent=2) == '[\n  1,\n  2,\n  [\n    3,\n    4\n  ],\n  5\n]'

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

assert json.dumps(a, indent=2) == '''{
  "a": 1,
  "b": 2,
  "c": null,
  "d": [
    1,
    2,
    3
  ],
  "e": {
    "a": 100,
    "b": 2.5,
    "c": null,
    "d": [
      142,
      2785,
      39767
    ]
  },
  "f": "This is a string",
  "g": [
    true,
    false,
    null
  ],
  "h": false
}'''
