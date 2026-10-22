# Equal numeric keys share an entry, regardless of insertion order or type.
min_int = -9223372036854775807 - 1
max_int = 9223372036854775807
groups = [
    [0, 0.0, -0.0, False],
    [1, 1.0, True],
    [-1, -1.0],
    [3, 3.0],
    [min_int, float(min_int)],
    [9007199254740992, 9007199254740992.0],
    [9007199254740994, 9007199254740994.0],
    [9223372036854774784, 9223372036854774784.0],
]
for group in groups:
    assert len(set(group)) == 1
    for left in group:
        for right in group:
            assert left == right
            assert not (left != right)
            assert hash(left) == hash(right)
            d = {left: 'first'}
            assert right in d
            assert d[right] == 'first'
            d[right] = 'updated'
            assert len(d) == 1
            assert list(d)[0] is left
            assert d.pop(left) == 'updated'
            assert d == {}

# Tuple keys use the same hashes for equal numeric elements, including large ints.
for group in groups:
    d = {(group[0],): 'tuple'}
    for value in group:
        assert hash((value,)) == hash((group[0],))
        assert d[(value,)] == 'tuple'
        d[(value,)] = 'tuple'
    assert len(d) == 1

# Rounded floats must not alias a different integer near precision boundaries.
for integer, floating in [
    (9007199254740993, 9007199254740992.0),
    (-9007199254740993, -9007199254740992.0),
    (max_int, float(max_int)),
    (min_int + 1, float(min_int)),
    (min_int, float('-9223372036854777856')),
    (0, 0.5),
    (0, -0.5),
    (0, 5e-324),
    (0, -5e-324),
]:
    assert integer != floating
    assert floating != integer
    assert not (integer == floating)
    assert not (floating == integer)
    d = {integer: 'integer', floating: 'float'}
    assert len(d) == 2
    assert d[integer] == 'integer'
    assert d[floating] == 'float'
    assert len({integer, floating}) == 2

# Hashing non-integral, out-of-range and non-finite floats does not cast to int.
for value in (0.5, -0.5, 1e100, -1e100, float('inf'), -float('inf')):
    assert isinstance(hash(value), int)
    assert hash(value) == hash(value)
    assert {value: 'value'}[value] == 'value'
    assert value != 0
    assert value != max_int
    assert value != min_int
nan = float('nan')
assert isinstance(hash(nan), int)
assert nan != nan
assert nan != 0
assert not (nan == 0)

