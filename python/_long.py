from c import sizeof

if sizeof('void_p') == 4:
    PyLong_SHIFT = 28//2
elif sizeof('void_p') == 8:
    PyLong_SHIFT = 60//2
else:
    raise NotImplementedError

PyLong_BASE = 2 ** PyLong_SHIFT
PyLong_MASK = PyLong_BASE - 1
PyLong_DECIMAL_SHIFT = 4
PyLong_DECIMAL_BASE = 10 ** PyLong_DECIMAL_SHIFT

def ulong_fromint(x: int):
    # return a list of digits and sign
    if x == 0: return [0], 1
    sign = 1 if x > 0 else -1
    if sign < 0: x = -x
    res = []
    while x:
        res.append(x & PyLong_MASK)
        x >>= PyLong_SHIFT
    return res, sign

def ulong_cmp(a: list, b: list) -> int:
    # return 1 if a>b, -1 if a<b, 0 if a==b
    if len(a) > len(b): return 1
    if len(a) < len(b): return -1
    for i in range(len(a)-1, -1, -1):
        if a[i] > b[i]: return 1
        if a[i] < b[i]: return -1
    return 0

def ulong_pad_(a: list, size: int):
    # pad leading zeros to have `size` digits
    delta = size - len(a)
    if delta > 0:
        a.extend([0] * delta)

def ulong_unpad_(a: list):
    # remove leading zeros
    while len(a)>1 and a[-1]==0:
        a.pop()

def ulong_add(a: list, b: list) -> list:
    res = [0] * max(len(a), len(b))
    ulong_pad_(a, len(res))
    ulong_pad_(b, len(res))
    carry = 0
    for i in range(len(res)):
        carry += a[i] + b[i]
        res[i] = carry & PyLong_MASK
        carry >>= PyLong_SHIFT
    if carry > 0:
        res.append(carry)
    return res

def ulong_sub(a: list, b: list) -> list:
    # a >= b
    res = []
    borrow = 0
    for i in range(len(b)):
        tmp = a[i] - b[i] - borrow
        if tmp < 0:
            tmp += PyLong_BASE
            borrow = 1
        else:
            borrow = 0
        res.append(tmp)
    for i in range(len(b), len(a)):
        tmp = a[i] - borrow
        if tmp < 0:
            tmp += PyLong_BASE
            borrow = 1
        else:
            borrow = 0
        res.append(tmp)
    ulong_unpad_(res)
    return res

def ulong_divmodi(a: list, b: int):
    # b > 0
    res = []
    carry = 0
    for i in range(len(a)-1, -1, -1):
        carry <<= PyLong_SHIFT
        carry += a[i]
        res.append(carry // b)
        carry %= b
    res.reverse()
    ulong_unpad_(res)
    return res, carry

def ulong_floordivi(a: list, b: int):
    # b > 0
    return ulong_divmodi(a, b)[0]

def ulong_muli(a: list, b: int):
    # b >= 0
    res = [0] * len(a)
    carry = 0
    for i in range(len(a)):
        carry += a[i] * b
        res[i] = carry & PyLong_MASK
        carry >>= PyLong_SHIFT
    if carry > 0:
        res.append(carry)
    return res

def ulong_mul(a: list, b: list):
    res = [0] * (len(a) + len(b))
    for i in range(len(a)):
        carry = 0
        for j in range(len(b)):
            carry += res[i+j] + a[i] * b[j]
            res[i+j] = carry & PyLong_MASK
            carry >>= PyLong_SHIFT
        res[i+len(b)] = carry
    ulong_unpad_(res)
    return res

def ulong_powi(a: list, b: int):
    # b >= 0
    if b == 0: return [1]
    res = [1]
    while b:
        if b & 1:
            res = ulong_mul(res, a)
        a = ulong_mul(a, a)
        b >>= 1
    return res

def ulong_repr(x: list) -> str:
    res = []
    while len(x)>1 or x[0]>0:   # non-zero
        x, r = ulong_divmodi(x, PyLong_DECIMAL_BASE)
        res.append(str(r).zfill(PyLong_DECIMAL_SHIFT))
    res.reverse()
    s = ''.join(res)
    if len(s) == 0: return '0'
    if len(s) > 1: s = s.lstrip('0')
    return s

def ulong_fromstr(s: str):
    res = [0]
    base = [1]
    if s[0] == '-':
        sign = -1
        s = s[1:]
    else:
        sign = 1
    s = s[::-1]
    for c in s:
        c = ord(c) - 48
        assert 0 <= c <= 9
        res = ulong_add(res, ulong_muli(base, c))
        base = ulong_muli(base, 10)
    return res, sign

class long:
    def __init__(self, x):
        if type(x) is tuple:
            self.digits, self.sign = x
        elif type(x) is int:
            self.digits, self.sign = ulong_fromint(x)
        elif type(x) is str:
            self.digits, self.sign = ulong_fromstr(x)
        else:
            raise TypeError('expected int or str')

    def __add__(self, other):
        if type(other) is int:
            other = long(other)
        else:
            assert type(other) is long
        if self.sign == other.sign:
            return long((ulong_add(self.digits, other.digits), self.sign))
        else:
            cmp = ulong_cmp(self.digits, other.digits)
            if cmp == 0:
                return long(0)
            if cmp > 0:
                return long((ulong_sub(self.digits, other.digits), self.sign))
            else:
                return long((ulong_sub(other.digits, self.digits), other.sign))
            
    def __radd__(self, other):
        return self.__add__(other)
    
    def __sub__(self, other):
        if type(other) is int:
            other = long(other)
        else:
            assert type(other) is long
        if self.sign != other.sign:
            return long((ulong_add(self.digits, other.digits), self.sign))
        else:
            cmp = ulong_cmp(self.digits, other.digits)
            if cmp == 0:
                return long(0)
            if cmp > 0:
                return long((ulong_sub(self.digits, other.digits), self.sign))
            else:
                return long((ulong_sub(other.digits, self.digits), -other.sign))
            
    def __rsub__(self, other):
        return self.__sub__(other)
    
    def __mul__(self, other):
        if type(other) is int:
            return long((
                ulong_muli(self.digits, abs(other)),
                self.sign * (1 if other >= 0 else -1)
            ))
        elif type(other) is long:
            return long((
                ulong_mul(self.digits, other.digits),
                self.sign * other.sign
            ))
        raise TypeError('unsupported operand type(s) for *')
    
    def __rmul__(self, other):
        return self.__mul__(other)
    
    #######################################################
    def __divmod__(self, other: int):
        assert type(other) is int and other > 0
        assert self.sign == 1
        q, r = ulong_divmodi(self.digits, other)
        return long((q, 1)), r

    def __floordiv__(self, other: int):
        return self.__divmod__(other)[0]

    def __mod__(self, other: int):
        return self.__divmod__(other)[1]

    def __pow__(self, other: int):
        assert type(other) is int and other >= 0
        if self.sign == -1 and other & 1:
            sign = -1
        else:
            sign = 1
        return long((ulong_powi(self.digits, other), sign))
    
    def __lshift__(self, other: int):
        # TODO: optimize
        assert type(other) is int and other >= 0
        x = self.digits.copy()
        for _ in range(other):
            x = ulong_muli(x, 2)
        return long((x, self.sign))
    
    def __rshift__(self, other: int):
        # TODO: optimize
        assert type(other) is int and other >= 0
        x = self.digits.copy()
        for _ in range(other):
            x = ulong_floordivi(x, 2)
        return long((x, self.sign))
    
    def __and__(self, other):
        raise NotImplementedError
    
    def __or__(self, other):
        raise NotImplementedError
    
    def __xor__(self, other):
        raise NotImplementedError
    
    def __neg__(self):
        return long((self.digits, -self.sign))
    
    def __cmp__(self, other):
        if type(other) is int:
            other = long(other)
        else:
            assert type(other) is long
        if self.sign > other.sign:
            return 1
        elif self.sign < other.sign:
            return -1
        else:
            return ulong_cmp(self.digits, other.digits)
        
    def __eq__(self, other):
        return self.__cmp__(other) == 0
    def __lt__(self, other):
        return self.__cmp__(other) < 0
    def __le__(self, other):
        return self.__cmp__(other) <= 0
    def __gt__(self, other):
        return self.__cmp__(other) > 0
    def __ge__(self, other):
        return self.__cmp__(other) >= 0
            
    def __repr__(self):
        prefix = '-' if self.sign < 0 else ''
        return prefix + ulong_repr(self.digits) + 'L'