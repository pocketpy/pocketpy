#pragma once

const char* __BUILTINS_CODE = R"(
def len(x):
    return x.__len__()

str.__mul__ = lambda self, n: ''.join([self for _ in range(n)])

def __str4split(self, sep):
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
str.split = __str4split
del __str4split

def __str4index(self, sub):
    for i in range(len(self) - len(sub) + 1):
        if self[i:i+len(sub)] == sub:
            return i
    return -1
str.index = __str4index
del __str4index

list.__repr__ = lambda self: '[' + ', '.join([repr(i) for i in self]) + ']'
tuple.__repr__ = lambda self: '(' + ', '.join([repr(i) for i in self]) + ')'
list.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'
tuple.__json__ = lambda self: '[' + ', '.join([i.__json__() for i in self]) + ']'

def __list4extend(self, other):
    for i in other:
        self.append(i)
list.extend = __list4extend
del __list4extend

def __list4remove(self, value):
    for i in range(len(self)):
        if self[i] == value:
            del self[i]
            return True
    return False
list.remove = __list4remove
del __list4remove

def __list4index(self, value):
    for i in range(len(self)):
        if self[i] == value:
            return i
    return -1
list.index = __list4index
del __list4index

def __list4__mul__(self, n):
    a = []
    for i in range(n):
        a.extend(self)
    return a
list.__mul__ = __list4__mul__
del __list4__mul__

def __iterable4__eq__(self, other):
    if len(self) != len(other):
        return False
    for i in range(len(self)):
        if self[i] != other[i]:
            return False
    return True
list.__eq__ = __iterable4__eq__
tuple.__eq__ = __iterable4__eq__
del __iterable4__eq__

def __iterable4count(self, x):
    res = 0
    for i in self:
        if i == x:
            res += 1
    return res
list.count = __iterable4count
tuple.count = __iterable4count
del __iterable4count

def __iterable4__contains__(self, item):
    for i in self:
        if i == item:
            return True
    return False
list.__contains__ = __iterable4__contains__
tuple.__contains__ = __iterable4__contains__
del __iterable4__contains__

list.__new__ = lambda obj: [i for i in obj]

# https://github.com/python/cpython/blob/main/Objects/dictobject.c
class dict:
    def __init__(self):
        self._capacity = 8
        self._a = [None] * self._capacity
        self._len = 0
        
    def __len__(self):
        return self._len

    def __probe(self, key):
        i = hash(key) % self._capacity
        while self._a[i] is not None:
            if self._a[i][0] == key:
                return True, i
            i = ((5*i) + 1) % self._capacity
        return False, i

    def __getitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(key)
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
            if self._len > self._capacity * 0.6:
                self.__resize_2x()

    def __delitem__(self, key):
        ok, i = self.__probe(key)
        if not ok:
            raise KeyError(key)
        self._a[i] = None
        self._len -= 1

    def __resize_2x(self):
        old_a = self._a
        self._capacity *= 2
        self._a = [None] * self._capacity
        self._len = 0
        for kv in old_a:
            if kv is not None:
                self[kv[0]] = kv[1]

    def keys(self):
        return [kv[0] for kv in self._a if kv is not None]

    def values(self):
        return [kv[1] for kv in self._a if kv is not None]

    def items(self):
        return [kv for kv in self._a if kv is not None]

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

def round(x):
    if x >= 0:
        return int(x + 0.5)
    else:
        return int(x - 0.5)

def max(a, b):
    if a > b:
        return a
    return b

def min(a, b):
    if a < b:
        return a
    return b

def sum(iterable):
    res = 0
    for i in iterable:
        res += i
    return res

def map(f, iterable):
    return [f(i) for i in iterable]

def zip(a, b):
    return [(a[i], b[i]) for i in range(min(len(a), len(b)))]

def sorted(iterable, key=None, reverse=False):
    if key is None:
        key = lambda x: x
    a = [key(i) for i in iterable]
    b = list(iterable)
    for i in range(len(a)):
        for j in range(i+1, len(a)):
            if (a[i] > a[j]) ^ reverse:
                a[i], a[j] = a[j], a[i]
                b[i], b[j] = b[j], b[i]
    return b

class FileIO:
  def __init__(self, path, mode):
    assert type(path) is str
    assert type(mode) is str
    assert mode in ['r', 'w']
    self.path = path
    self.mode = mode
    self.fp = jsonrpc({"method": "fopen", "params": [path, mode]})

  def read(self):
    assert self.mode == 'r'
    return jsonrpc({"method": "fread", "params": [self.fp]})

  def write(self, s):
    assert self.mode == 'w'
    assert type(s) is str
    jsonrpc({"method": "fwrite", "params": [self.fp, s]})

  def close(self):
    jsonrpc({"method": "fclose", "params": [self.fp]})

  def __enter__(self):
    pass

  def __exit__(self):
    self.close()

def open(path, mode='r'):
    return FileIO(path, mode)
)";

const char* __RANDOM_CODE = R"(
import time as _time

__all__ = ['Random', 'seed', 'random', 'randint', 'uniform']

def _int32(x):
	return int(0xffffffff & x)

class Random:
	def __init__(self, seed=None):
		if seed is None:
			seed = int(_time.time() * 1000000)
		seed = _int32(seed)
		self.mt = [0] * 624
		self.mt[0] = seed
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
	
	def extract_number(self):
		if self.mti == 0:
			self.twist()
		y = self.mt[self.mti]
		y = y ^ y >> 11
		y = y ^ y << 7 & 2636928640
		y = y ^ y << 15 & 4022730752
		y = y ^ y >> 18
		self.mti = (self.mti + 1) % 624
		return _int32(y)
	
	def twist(self):
		for i in range(0, 624):
			y = _int32((self.mt[i] & 0x80000000) + (self.mt[(i + 1) % 624] & 0x7fffffff))
			self.mt[i] = (y >> 1) ^ self.mt[(i + 397) % 624]
			
			if y % 2 != 0:
				self.mt[i] = self.mt[i] ^ 0x9908b0df
				
	def seed(self, x):
		assert type(x) is int
		self.mt = [0] * 624
		self.mt[0] = _int32(x)
		self.mti = 0
		for i in range(1, 624):
			self.mt[i] = _int32(1812433253 * (self.mt[i - 1] ^ self.mt[i - 1] >> 30) + i)
			
	def random(self):
		return self.extract_number() / 2 ** 32
		
	def randint(self, a, b):
		assert type(a) is int and type(b) is int
		assert a <= b
		return int(self.random() * (b - a + 1)) + a
		
	def uniform(self, a, b):
        assert type(a) is int or type(a) is float
        assert type(b) is int or type(b) is float
		if a > b:
			a, b = b, a
		return self.random() * (b - a) + a

    def shuffle(self, L):
        for i in range(len(L)):
            j = self.randint(i, len(L) - 1)
            L[i], L[j] = L[j], L[i]

    def choice(self, L):
        return L[self.randint(0, len(L) - 1)]
		
_inst = Random()
seed = _inst.seed
random = _inst.random
randint = _inst.randint
uniform = _inst.uniform
shuffle = _inst.shuffle
choice = _inst.choice

)";