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
def tokenize(s:str) -> list:
    tokens = []
    L, R = 0,0
    
    mode = None
    curPos = 0
    lookingForKword = False
    
    while(R<len(s) and not lookingForKword):
        curChar = s[R]
        nextChar = s[R+1] if R+1<len(s) else ''
        
        # Escaping case
        if (curChar == '{' and nextChar == '{') or (curChar == '}' and nextChar == '}'):
            tokens.append(curChar)
            L = R+2
            R = R+2
            continue
        
        # Regular command line arg case
        if curChar == '{' and nextChar == '}':
            
            if mode is not None and mode != 'auto':
                raise ValueError("Cannot switch from manual field numbering to automatic field specification")
            
            mode = 'auto'
            # best case {}, just for normal args
            if(L<R):
                tokens.append(s[L:R])
            tokens.append("{"+str(curPos)+"}")
            curPos+=1
            L = R+2
            R = R+2
            continue
        
        # Kwarg case
        elif (curChar == '{'):
            
            if mode is not None and mode != 'manual':
                raise ValueError("Cannot switch from automatic field specification to manual field numbering")
            
            mode = 'manual'
            
            if(L<R):
                tokens.append(s[L:R])
            
            lookingForKword = True
            kwL = R+1
            kwR = R+1
            while(kwR<len(s) and s[kwR]!='}'):
                kwR += 1
            tokens.append(s[R:kwR+1])
            
            if kwR<len(s) and s[kwR] == '}':
                lookingForKword = False
                L = kwR+1
                R = kwR+1
            continue
        
        R = R+1
    
    if lookingForKword:
        raise ValueError("Expected '}' before end of string")
    
    if(not lookingForKword and L<R):
        tokens.append(s[L:R])

    # print("Looking for kword: ", lookingForKword)
    return tokens
def __f(self:str, *args, **kwargs) -> str:
    tokens = tokenize(self)
    argMap = {}
    for i, a in enumerate(args):
        argMap[str(i)] = a
    final_tokens = []
    for t in tokens:
        if t[0] == '{' and t[-1] == '}':
            key = t[1:-1]
            argMapVal = argMap.get(key, None)
            kwargsVal = kwargs.get(key, None)
            
            if argMapVal is None and kwargsVal is None:
                raise ValueError("No arg found for token: "+t)
            elif argMapVal is not None:
                final_tokens.append(str(argMapVal))
            else:
                final_tokens.append(str(kwargsVal))
        else:
            final_tokens.append(t)
    
    return ''.join(final_tokens)
 
    # if '{}' in self:
    #     for i in range(len(args)):
    #         self = self.replace('{}', str(args[i]), 1)
    # else:
    #     # Positional arguments will be followed by keyword arguments
    #     # 1. Replace the positional arguments
    #     for i,a in enumerate(args):
    #         self = self.replace('{'+str(i)+'}', str(a))
        
    #     # 2. Replace the keyword arguments
    #     for k,v in kwargs.items():
    #         self = self.replace('{'+k+'}', str(v))
    
    # return self
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