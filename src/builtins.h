#pragma once

const char* kBuiltinsCode = R"(
def print(*args, sep=' ', end='\n'):
    s = sep.join([str(i) for i in args])
    __sys_stdout_write(s + end)

def round(x, ndigits=0):
    assert ndigits >= 0
    if ndigits == 0:
        return x >= 0 ? int(x + 0.5) : int(x - 0.5)
    if x >= 0:
        return int(x * 10**ndigits + 0.5) / 10**ndigits
    else:
        return int(x * 10**ndigits - 0.5) / 10**ndigits

def isinstance(obj, cls):
    assert type(cls) is type
    obj_t = type(obj)
    while obj_t is not None:
        if obj_t is cls:
            return True
        obj_t = obj_t.__base__
    return False

def abs(x):
    return x < 0 ? -x : x

def max(a, b):
    return a > b ? a : b

def min(a, b):
    return a < b ? a : b

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

str.__mul__ = lambda self, n: ''.join([self for _ in range(n)])

def str::split(self, sep):
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

def str::index(self, sub):
    for i in range(len(self)):
        if self[i:i+len(sub)] == sub:
            return i
    return -1

def str::strip(self, chars=None):
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

def list::sort(self, reverse=False):
    __qsort(self, 0, len(self)-1)
    if reverse:
        self.reverse()

def list::extend(self, other):
    for i in other:
        self.append(i)

def list::remove(self, value):
    for i in range(len(self)):
        if self[i] == value:
            del self[i]
            return True
    return False

def list::index(self, value):
    for i in range(len(self)):
        if self[i] == value:
            return i
    return -1

def list::pop(self, i=-1):
    res = self[i]
    del self[i]
    return res

def list::__eq__(self, other):
    if len(self) != len(other):
        return False
    for i in range(len(self)):
        if self[i] != other[i]:
            return False
    return True
tuple.__eq__ = list.__eq__
list.__ne__ = lambda self, other: not self.__eq__(other)
tuple.__ne__ = lambda self, other: not self.__eq__(other)

def list::count(self, x):
    res = 0
    for i in self:
        if i == x:
            res += 1
    return res
tuple.count = list.count

def list::__contains__(self, item):
    for i in self:
        if i == item:
            return True
    return False

tuple.__contains__ = list.__contains__
list.__new__ = lambda obj: [i for i in obj]

class dict:
    def __init__(self, capacity=13):
        self._capacity = capacity
        self._a = [None] * self._capacity
        self._len = 0
        
    def __len__(self):
        return self._len

    def __probe(self, key):
        i = hash(key) % self._capacity
        while self._a[i] is not None:
            if self._a[i][0] == key:
                return True, i
            i = (i + 1) % self._capacity
        return False, i

    def __getitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        return self._a[i][1]

    def __contains__(self, key):
        ok, i = self.__probe(key)
        return ok

    def __setitem__(self, key, value):
        ok, i = self.__probe(key)
        if ok:
            self._a[i][1] = value
        else:
            self._a[i] = [key, value]
            self._len += 1
            if self._len > self._capacity * 0.67:
                self._capacity *= 2
                self.__rehash()

    def __delitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(repr(key))
        self._a[i] = None
        self._len -= 1

    def __rehash(self):
        old_a = self._a
        self._a = [None] * self._capacity
        self._len = 0
        for kv in old_a:
            if kv is not None:
                self[kv[0]] = kv[1]

    def get(self, key, default=None):
        ok, i = self.__probe(key)
        if ok:
            return self._a[i][1]
        return default

    def keys(self):
        for kv in self._a:
            if kv is not None:
                yield kv[0]

    def values(self):
        for kv in self._a:
            if kv is not None:
                yield kv[1]

    def items(self):
        for kv in self._a:
            if kv is not None:
                yield kv

    def clear(self):
        self._a = [None] * self._capacity
        self._len = 0

    def update(self, other):
        for k, v in other.items():
            self[k] = v

    def copy(self):
        d = dict()
        for kv in self._a:
            if kv is not None:
                d[kv[0]] = kv[1]
        return d

    def __repr__(self):
        a = [repr(k)+': '+repr(v) for k,v in self.items()]
        return '{'+ ', '.join(a) + '}'

    def __json__(self):
        a = []
        for k,v in self.items():
            if type(k) is not str:
                raise TypeError('json keys must be strings, got ' + repr(k) )
            a.append(k.__json__()+': '+v.__json__())
        return '{'+ ', '.join(a) + '}'

class set:
    def __init__(self, iterable=None):
        iterable = iterable or []
        self._a = dict()
        for item in iterable:
            self.add(item)

    def add(self, elem):
        self._a[elem] = None
        
    def discard(self, elem):
        if elem in self._a:
            del self._a[elem]

    def remove(self, elem):
        del self._a[elem]
        
    def clear(self):
        self._a.clear()

    def update(self,other):
        for elem in other:
            self.add(elem)
        return self

    def __len__(self):
        return len(self._a)
    
    def copy(self):
        return set(self._a.keys())
    
    def __and__(self, other):
        ret = set()
        for elem in self:
            if elem in other:
                ret.add(elem)
        return ret
    
    def __or__(self, other):
        ret = self.copy()
        for elem in other:
            ret.add(elem)
        return ret

    def __sub__(self, other):
        ret = set() 
        for elem in self:
            if elem not in other: 
                ret.add(elem) 
        return ret
    
    def __xor__(self, other): 
        ret = set() 
        for elem in self: 
            if elem not in other: 
                ret.add(elem) 
        for elem in other: 
            if elem not in self: 
                ret.add(elem) 
        return ret

    def union(self, other):
        return self | other

    def intersection(self, other):
        return self & other

    def difference(self, other):
        return self - other

    def symmetric_difference(self, other):      
        return self ^ other
    
    def __eq__(self, other):
        return self.__xor__(other).__len__() == 0

    def __ne__(self, other):
        return self.__xor__(other).__len__() != 0
    
    def isdisjoint(self, other):
        return self.__and__(other).__len__() == 0
    
    def issubset(self, other):
        return self.__sub__(other).__len__() == 0
    
    def issuperset(self, other):
        return other.__sub__(self).__len__() == 0

    def __contains__(self, elem):
        return elem in self._a
    
    def __repr__(self):
        if len(self) == 0:
            return 'set()'
        return '{'+ ', '.join([repr(i) for i in self._a.keys()]) + '}'
    
    def __iter__(self):
        return self._a.keys()
)";

const char* kRandomCode = R"(
def shuffle(L):
    for i in range(len(L)):
        j = randint(i, len(L) - 1)
        L[i], L[j] = L[j], L[i]

def choice(L):
    return L[randint(0, len(L) - 1)]
)";

const char* kFuncToolsCode = R"(
def cache(f):
    def wrapper(*args):
        if not hasattr(f, 'cache'):
            f.cache = {}
        key = args
        if key not in f.cache:
            f.cache[key] = f(*args)
        return f.cache[key]
    return wrapper
)";