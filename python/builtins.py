import sys as _sys

def print(*args, sep=' ', end='\n'):
    s = sep.join([str(i) for i in args])
    _sys.stdout.write(s + end)

def round(x, ndigits=0):
    assert ndigits >= 0
    if ndigits == 0:
        return int(x + 0.5) if x >= 0 else int(x - 0.5)
    if x >= 0:
        return int(x * 10**ndigits + 0.5) / 10**ndigits
    else:
        return int(x * 10**ndigits - 0.5) / 10**ndigits

def abs(x):
    return -x if x < 0 else x

def max(*args):
    if len(args) == 0:
        raise TypeError('max expected 1 arguments, got 0')
    if len(args) == 1:
        args = args[0]
    args = iter(args)
    res = next(args)
    if res is StopIteration:
        raise ValueError('max() arg is an empty sequence')
    while True:
        i = next(args)
        if i is StopIteration:
            break
        if i > res:
            res = i
    return res

def min(*args):
    if len(args) == 0:
        raise TypeError('min expected 1 arguments, got 0')
    if len(args) == 1:
        args = args[0]
    args = iter(args)
    res = next(args)
    if res is StopIteration:
        raise ValueError('min() arg is an empty sequence')
    while True:
        i = next(args)
        if i is StopIteration:
            break
        if i < res:
            res = i
    return res

def all(iterable):
    for i in iterable:
        if not i:
            return False
    return True

def any(iterable):
    for i in iterable:
        if i:
            return True
    return False

def enumerate(iterable, start=0):
    n = start
    for elem in iterable:
        yield n, elem
        ++n

def sum(iterable):
    res = 0
    for i in iterable:
        res += i
    return res

def map(f, iterable):
    for i in iterable:
        yield f(i)

def filter(f, iterable):
    for i in iterable:
        if f(i):
            yield i

def zip(a, b):
    a = iter(a)
    b = iter(b)
    while True:
        ai = next(a)
        bi = next(b)
        if ai is StopIteration or bi is StopIteration:
            break
        yield ai, bi

def reversed(iterable):
    a = list(iterable)
    a.reverse()
    return a

def sorted(iterable, reverse=False, key=None):
    a = list(iterable)
    a.sort(reverse=reverse, key=key)
    return a

##### str #####
def __f(self, sep=None):
    sep = sep or ' '
    if sep == "":
        return list(self)
    res = []
    i = 0
    while i < len(self):
        if self[i:i+len(sep)] == sep:
            res.append(self[:i])
            self = self[i+len(sep):]
            i = 0
        else:
            ++i
    res.append(self)
    return res
str.split = __f

def __f(self, *args):
    if '{}' in self:
        for i in range(len(args)):
            self = self.replace('{}', str(args[i]), 1)
    else:
        for i in range(len(args)):
            self = self.replace('{'+str(i)+'}', str(args[i]))
    return self
str.format = __f

def __f(self, chars=None):
    chars = chars or ' \t\n\r'
    i = 0
    while i < len(self) and self[i] in chars:
        ++i
    return self[i:]
str.lstrip = __f

def __f(self, chars=None):
    chars = chars or ' \t\n\r'
    j = len(self) - 1
    while j >= 0 and self[j] in chars:
        --j
    return self[:j+1]
str.rstrip = __f

def __f(self, chars=None):
    chars = chars or ' \t\n\r'
    i = 0
    while i < len(self) and self[i] in chars:
        ++i
    j = len(self) - 1
    while j >= 0 and self[j] in chars:
        --j
    return self[i:j+1]
str.strip = __f

##### list #####
list.__repr__ = lambda self: '[' + ', '.join([repr(i) for i in self]) + ']'
tuple.__repr__ = lambda self: '(' + ', '.join([repr(i) for i in self]) + ')'
list.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'
tuple.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'

def __qsort(a: list, L: int, R: int, key):
    if L >= R: return;
    mid = a[(R+L)//2];
    mid = key(mid)
    i, j = L, R
    while i<=j:
        while key(a[i])<mid: ++i;
        while key(a[j])>mid: --j;
        if i<=j:
            a[i], a[j] = a[j], a[i]
            ++i; --j;
    __qsort(a, L, j, key)
    __qsort(a, i, R, key)

def __f(self, reverse=False, key=None):
    if key is None:
        key = lambda x:x
    __qsort(self, 0, len(self)-1, key)
    if reverse:
        self.reverse()
list.sort = __f

def __f(self, other):
    for i, j in zip(self, other):
        if i != j:
            return i < j
    return len(self) < len(other)
tuple.__lt__ = __f
list.__lt__ = __f

def __f(self, other):
    for i, j in zip(self, other):
        if i != j:
            return i > j
    return len(self) > len(other)
tuple.__gt__ = __f
list.__gt__ = __f

def __f(self, other):
    for i, j in zip(self, other):
        if i != j:
            return i <= j
    return len(self) <= len(other)
tuple.__le__ = __f
list.__le__ = __f

def __f(self, other):
    for i, j in zip(self, other):
        if i != j:
            return i >= j
    return len(self) >= len(other)
tuple.__ge__ = __f
list.__ge__ = __f

type.__repr__ = lambda self: "<class '" + self.__name__ + "'>"

def help(obj):
    if hasattr(obj, '__func__'):
        obj = obj.__func__
    if hasattr(obj, '__doc__'):
        print(obj.__doc__)
    else:
        print("No docstring found")


del __f