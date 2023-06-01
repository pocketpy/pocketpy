import math


def float_as_integer_ratio(x):
    if x == 0:
        return (0, 1)

    sign = 1 if x > 0 else -1
    x = abs(x)

    p0, q0 = 0, 1
    p1, q1 = 1, 0
    while True:
        n = int(x)
        p2 = n * p1 + p0
        q2 = n * q1 + q0
        if x == n:
            break
        x = 1 / (x - n)
        p0, q0 = p1, q1
        p1, q1 = p2, q2
    gcd_value = math.gcd(p2, q2)
    return (sign * p2 // gcd_value, q2 // gcd_value)


def _divide_and_round(a, b):
    """divide a by b and round result to the nearest integer

    When the ratio is exactly half-way between two integers,
    the even integer is returned.
    """
    # Based on the reference implementation for divmod_near
    # in Objects/longobject.c.
    q, r = divmod(a, b)
    # round up if either r / b > 0.5, or r / b == 0.5 and q is odd.
    # The expression r / b > 0.5 is equivalent to 2 * r > b if b is
    # positive, 2 * r < b if b negative.
    r *= 2
    greater_than_half = r > b if b > 0 else r < b
    if greater_than_half or r == b and q % 2 == 1:
        q += 1

    return q


class timedelta:
    def __init__(self, days=0, seconds=0, minutes=0, hours=0, weeks=0):
        d = 0
        s = 0

        days += weeks * 7
        seconds += minutes * 60 + hours * 3600
        # Get rid of all fractions, and normalize s and us.
        # Take a deep breath <wink>.
        if isinstance(days, float):
            dayfrac, days = math.modf(days)
            daysecondsfrac, daysecondswhole = math.modf(dayfrac * float(24 * 3600))
            assert daysecondswhole == int(daysecondswhole)  # can't overflow
            s = int(daysecondswhole)
            assert days == int(days)
            d = int(days)
        else:
            daysecondsfrac = 0.0
            d = days

        assert isinstance(daysecondsfrac, float)
        assert abs(daysecondsfrac) <= 1.0
        assert isinstance(d, int)
        assert abs(s) <= 24 * 3600
        # days isn't referenced again before redefinition

        if isinstance(seconds, float):
            secondsfrac, seconds = math.modf(seconds)
            assert seconds == int(seconds)
            seconds = int(seconds)
            secondsfrac += daysecondsfrac
            assert abs(secondsfrac) <= 2.0
        else:
            secondsfrac = daysecondsfrac

        # daysecondsfrac isn't referenced again
        assert isinstance(secondsfrac, float)
        assert abs(secondsfrac) <= 2.0
        if round(secondsfrac) == 1.0:
            s += 1
        assert isinstance(seconds, int)
        days, seconds = divmod(seconds, 24 * 3600)

        d += days
        s += int(seconds)  # can't overflow

        while s > 0 and d < 0:
            s -= 86400
            d += 1

        while s < 0 and d > 0:
            s += 86400
            d -= 1

        assert isinstance(s, int)
        assert abs(s) <= 2 * 24 * 3600

        # seconds isn't referenced again before redefinition

        if abs(d) > 999999999:
            raise OverflowError  # "timedelta # of days is too large: {d}"
        self.days = d
        self.second = s

    def __repr__(self) -> str:
        args = []
        if self.days:
            args.append(f'days={self.days:1d}')
        if self.second:
            args.append(f'seconds={self.second:1d}')
        if not args:
            args.append('0')
        args = ", ".join(map(str, args))
        return f'datetime.timedelta({args})'

    def __cmp__(self, other) -> int:
        assert isinstance(other, timedelta)
        return self._cmp(self._getstate(), other._getstate())

    def __eq__(self, other: 'timedelta') -> bool:
        if isinstance(other, timedelta):
            return self.__cmp__(other) == 0
        raise NotImplemented

    def __lt__(self, other: 'timedelta') -> bool:
        if isinstance(other, timedelta):
            return self.__cmp__(other) < 0
        raise NotImplemented

    def __le__(self, other: 'timedelta') -> bool:
        if isinstance(other, timedelta):
            return self.__cmp__(other) <= 0
        raise NotImplemented

    def __gt__(self, other: 'timedelta') -> bool:
        if isinstance(other, timedelta):
            return self.__cmp__(other) > 0
        raise NotImplemented

    def __ge__(self, other: 'timedelta') -> bool:
        if isinstance(other, timedelta):
            return self.__cmp__(other) >= 0
        raise NotImplemented

    def __radd__(self, other: 'timedelta') -> 'timedelta':
        if isinstance(other, timedelta):
            return timedelta(days=self.days + other.days, seconds=self.second + other.second)
        raise NotImplemented

    def __add__(self, other: 'timedelta') -> 'timedelta':
        if isinstance(other, timedelta):
            return timedelta(days=self.days + other.days, seconds=self.second + other.second)
        raise NotImplemented

    def __rsub__(self, other: 'timedelta'):
        if isinstance(other, timedelta):
            return -self + other
        raise NotImplemented

    def __sub__(self, other: 'timedelta') -> 'timedelta':
        if isinstance(other, timedelta):
            return timedelta(days=self.days - other.days, seconds=self.second - other.second)
        raise NotImplemented

    def __mul__(self, other: 'timedelta') -> 'timedelta':
        if isinstance(other, int):
            # for CPython compatibility, we cannot use
            # our __class__ here, but need a real timedelta
            return timedelta(self.days * other, self.second * other)
        raise NotImplemented

    def __neg__(self) -> 'timedelta':
        return timedelta(days=-self.days, seconds=-self.second)

    def __pos__(self):
        return self

    def __abs__(self) -> 'timedelta':
        if self.days < 0:
            return -self
        else:
            return self

    def _to_seconds(self) -> int:
        return self.days * (24 * 3600) + self.second

    def __floordiv__(self, other: 'timedelta') -> 'timedelta':
        if not (isinstance(other, int) or isinstance(other, timedelta)):
            raise NotImplemented
        usec = self._to_seconds()
        if isinstance(other, timedelta):
            return usec // other._to_seconds()
        if isinstance(other, int):
            return timedelta(0, usec // other)

    def __truediv__(self, other: 'timedelta') -> 'timedelta':
        if not (isinstance(other, int) or isinstance(other, float) or isinstance(other, timedelta)):
            raise NotImplemented
        usec = self._to_seconds()
        if isinstance(other, timedelta):
            return usec / other._to_seconds()
        if isinstance(other, int):
            return timedelta(0, _divide_and_round(usec, other))
        if isinstance(other, float):
            a, b = float_as_integer_ratio(other)
            return timedelta(0, _divide_and_round(b * usec, a))

    def _getstate(self):
        return (self.days, self.second)

    def _cmp(self, a, b) -> int:
        if a[0] > b[0]:
            return 1
        if a[0] < b[0]:
            return -1
        if a[1] > b[1]:
            return 1
        if a[1] < b[1]:
            return -1
        return 0

    def __hash__(self):
        return hash(self._getstate())


timedelta.min = timedelta(-999999999)
timedelta.max = timedelta(days=999999999, seconds=59)
timedelta.resolution = timedelta(seconds=1)
