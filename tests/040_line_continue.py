a = 1 + 2 \
    + 3

assert a == 6

assert 1 + 2 \
    + 3 == 6

assert 1 + 2 + \
    3 + \
    4 == 10

assert 1 + 2 + \
    3 + \
    4 + 5 + 6 \
    == 21

if 1 and 2 \
    and 3 \
    and 4 \
    and 5:
    assert True
else:
    assert False

1 and 2 \
    and 3 \
    and 4

a = 1
assert a == 1