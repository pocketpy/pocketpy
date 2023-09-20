import sys as _sys

def print(*args, sep=' ', end='\n'):
    s = sep.join([str(i) for i in args])
    _sys.stdout.write(s + end)

def max(*args, key=None):
    if key is None:
        key = lambda x: x
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
        if key(i) > key(res):
            res = i
    return res

def min(*args, key=None):
    if key is None:
        key = lambda x: x
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
        if key(i) < key(res):
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

def __f(self, width: int):
    delta = width - len(self)
    if delta <= 0:
        return self
    return '0' * delta + self
str.zfill = __f

def __f(self, width: int, fillchar=' '):
    delta = width - len(self)
    if delta <= 0:
        return self
    assert len(fillchar) == 1
    return fillchar * delta + self
str.rjust = __f

def __f(self, width: int, fillchar=' '):
    delta = width - len(self)
    if delta <= 0:
        return self
    assert len(fillchar) == 1
    return self + fillchar * delta
str.ljust = __f

##### list #####
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
type.__getitem__ = lambda self, *args: self     # for generics

def help(obj):
    if hasattr(obj, '__func__'):
        obj = obj.__func__
    print(obj.__signature__)
    print(obj.__doc__)

del __f

class Exception: pass

from _long import long