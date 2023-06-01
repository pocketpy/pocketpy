from datetime import timedelta

assert repr(timedelta(days=50, seconds=27)) == 'datetime.timedelta(days=50, seconds=27)'
assert repr(timedelta(days=1.0 / (60 * 60 * 24), seconds=0)) == 'datetime.timedelta(seconds=1)'
assert repr(timedelta(days=0, seconds=1.0)) == 'datetime.timedelta(seconds=1)'
assert repr(timedelta(days=1.0 / (60 * 60 * 24), seconds=0)) == 'datetime.timedelta(seconds=1)'
assert repr(timedelta(42)) == 'datetime.timedelta(days=42)'


def eq(a, b):
    assert a == b


# test_constructor:
# Check keyword args to constructor
eq(timedelta(), timedelta(days=0, seconds=0, minutes=0, hours=0, weeks=0))
eq(timedelta(1), timedelta(days=1))
eq(timedelta(0, 1), timedelta(seconds=1))

# Check float args to constructor
eq(timedelta(days=1.0), timedelta(seconds=60 * 60 * 24))
eq(timedelta(days=1.0 / 24), timedelta(hours=1))
eq(timedelta(hours=1.0 / 60), timedelta(minutes=1))
eq(timedelta(minutes=1.0 / 60), timedelta(seconds=1))

# test_hash_equality:
t1 = timedelta(days=100, seconds=-8640000)
t2 = timedelta()
eq(hash(t1), hash(t2))

a = timedelta(days=7)  # One week
b = timedelta(0, 60)  # One minute
eq(a + b, timedelta(7, 60))
eq(a - b, timedelta(6, 24 * 3600 - 60))
eq(b.__rsub__(a), timedelta(6, 24 * 3600 - 60))
eq(-a, timedelta(-7))
eq(a, timedelta(7))
eq(timedelta(-1, 24 * 3600 - 60), -b)

eq(timedelta(6, 24 * 3600), a)
# TODO __rmul__
# eq(a * 10, timedelta(70))
# eq(a * 10, 10 * a)
# eq(a * 10, 10 * a)
# eq(b * 10, timedelta(0, 600))
# eq(10 * b, timedelta(0, 600))
eq(b * 10, timedelta(0, 600))
eq(a * -1, -a)
eq(b * -2, -b - b)
eq(b * (60 * 24), (b * 60) * 24)
eq(a // 7, timedelta(1))
eq(b // 10, timedelta(0, 6))
eq(a // 10, timedelta(0, 7 * 24 * 360))
eq(a / 0.5, timedelta(14))
eq(b / 0.5, timedelta(0, 120))
eq(a / 7, timedelta(1))
eq(b / 10, timedelta(0, 6))
eq(a / 10, timedelta(0, 7 * 24 * 360))

# test_compare:
t1 = timedelta(2, 4)
t2 = timedelta(2, 4)
eq(t1, t2)

assert t1 <= t2
assert t1 >= t2
assert not t1 < t2
assert not t1 > t2

# test_repr:
eq(repr(timedelta(1)), "datetime.timedelta(days=1)")
eq(repr(timedelta(10, 2)), "datetime.timedelta(days=10, seconds=2)")
eq(repr(timedelta(seconds=60)), "datetime.timedelta(seconds=60)")
eq(repr(timedelta()), "datetime.timedelta(0)")

# test_resolution_info:
assert isinstance(timedelta.min, timedelta)
assert isinstance(timedelta.max, timedelta)
assert isinstance(timedelta.resolution, timedelta)
assert timedelta.max > timedelta.min
eq(timedelta.min, timedelta(-999999999))
eq(timedelta.max, timedelta(999999999, 59))
eq(timedelta.resolution, timedelta(0, 1))

# test_bool:
assert timedelta(1)
assert timedelta(0, 1)
assert timedelta(0)