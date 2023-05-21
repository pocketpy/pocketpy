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

def max(a, b):
    return a if a > b else b

def min(a, b):
    return a if a < b else b

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
        n += 1

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
    for i in range(min(len(a), len(b))):
        yield (a[i], b[i])

def reversed(iterable):
    a = list(iterable)
    a.reverse()
    return a

def sorted(iterable, reverse=False):
    a = list(iterable)
    a.sort(reverse=reverse)
    return a

##### str #####
def str@split(self, sep):
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
            i += 1
    res.append(self)
    return res

def str@format(self, *args):
    if '{}' in self:
        for i in range(len(args)):
            self = self.replace('{}', str(args[i]), 1)
    else:
        for i in range(len(args)):
            self = self.replace('{'+str(i)+'}', str(args[i]))
    return self

def str@strip(self, chars=None):
    chars = chars or ' \t\n\r'
    i = 0
    while i < len(self) and self[i] in chars:
        i += 1
    j = len(self) - 1
    while j >= 0 and self[j] in chars:
        j -= 1
    return self[i:j+1]

##### list #####
list.__repr__ = lambda self: '[' + ', '.join([repr(i) for i in self]) + ']'
tuple.__repr__ = lambda self: '(' + ', '.join([repr(i) for i in self]) + ')'
list.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'
tuple.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'

def __qsort(a: list, L: int, R: int):
    if L >= R: return;
    mid = a[(R+L)//2];
    i, j = L, R
    while i<=j:
        while a[i]<mid: i+=1
        while a[j]>mid: j-=1
        if i<=j:
            a[i], a[j] = a[j], a[i]
            i+=1
            j-=1
    __qsort(a, L, j)
    __qsort(a, i, R)

def list@sort(self, reverse=False):
    __qsort(self, 0, len(self)-1)
    if reverse:
        self.reverse()

class property:
    def __init__(self, fget, fset=None):
        self.fget = fget
        self.fset = fset

    def __get__(self, obj):
        return self.fget(obj)
    
    def __set__(self, obj, value):
        if self.fset is None:
            raise AttributeError("readonly property")
        self.fset(obj, value)

class staticmethod:
    def __init__(self, f):
        self.f = f

    def __get__(self, obj):
        return self.f

    def __call__(self, *args):
        return self.f(*args)
    
def type@__repr__(self):
    return "<class '" + self.__name__ + "'>"

def help(obj):
    if hasattr(obj, '__func__'):
        obj = obj.__func__
    if hasattr(obj, '__doc__'):
        print(obj.__doc__)
    else:
        print("No docstring found")
