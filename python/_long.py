from c import sizeof

# https://www.cnblogs.com/liuchanglc/p/14203783.html
if sizeof('void_p') == 4:
    PyLong_SHIFT = 28//2 - 1
    PyLong_NTT_P = 12289
    PyLong_NTT_PR = 11
elif sizeof('void_p') == 8:
    PyLong_SHIFT = 60//2 - 1
    # 998244353 can not be compiled in 32-bit platform (even it is not used)
    PyLong_NTT_P = 998244353   # PyLong_NTT_P**2 should not overflow
    PyLong_NTT_PR = 3
else:
    raise NotImplementedError

PyLong_BASE = 2 ** PyLong_SHIFT
PyLong_MASK = PyLong_BASE - 1
PyLong_DECIMAL_SHIFT = 4
PyLong_DECIMAL_BASE = 10 ** PyLong_DECIMAL_SHIFT

assert PyLong_NTT_P > PyLong_BASE

#----------------------------------------------------------------------------#
#                                                                            #
#                         Number Theoretic Transform                         #
#                                                                            #
#----------------------------------------------------------------------------#

def ibin(n, bits):
    assert type(bits) is int and bits >= 0
    return bin(n)[2:].rjust(bits, "0")

def _number_theoretic_transform(a: list, p, pr, inverse):
    n = len(a)
    assert n&(n - 1) == 0

    a = [x % p for x in a]
    b = n.bit_length() - 1

    for i in range(1, n):
        j = int(ibin(i, b)[::-1], 2)
        if i < j:
            a[i], a[j] = a[j], a[i]

    rt = pow(pr, (p - 1) // n, p)
    if inverse:
        rt = pow(rt, p - 2, p)

    w = [1]*(n // 2)
    for i in range(1, n // 2):
        w[i] = w[i - 1]*rt % p

    h = 2
    while h <= n:
        hf, ut = h // 2, n // h
        for i in range(0, n, h):
            for j in range(hf):
                u = a[i + j]
                v = a[i + j + hf] * w[ut * j] % p
                a[i + j] = (u + v) % p
                a[i + j + hf] = (u - v + p) % p
        h *= 2

    if inverse:
        rv = pow(n, p - 2, p)
        a = [x*rv % p for x in a]

    return a


def ntt(a, p, pr):
    return _number_theoretic_transform(a, p, pr, False)

def intt(a, p, pr):
    return _number_theoretic_transform(a, p, pr, True)

##############################################################

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
    N = len(a) + len(b)
    if False:
        # use grade-school multiplication
        res = [0] * N
        for i in range(len(a)):
            carry = 0
            for j in range(len(b)):
                carry += res[i+j] + a[i] * b[j]
                res[i+j] = carry & PyLong_MASK
                carry >>= PyLong_SHIFT
            res[i+len(b)] = carry
        ulong_unpad_(res)
        return res
    else:
        # use fast number-theoretic transform
        limit = 1
        while limit < N:
            limit <<= 1
        a += [0]*(limit - len(a))
        b += [0]*(limit - len(b))
        # print(a, b)
        a = ntt(a, PyLong_NTT_P, PyLong_NTT_PR)
        b = ntt(b, PyLong_NTT_P, PyLong_NTT_PR)
        # print(a, b)
        c = [0] * limit
        for i in range(limit):
            c[i] = (a[i] * b[i]) % PyLong_NTT_P

        # print(c)
        c = intt(c, PyLong_NTT_P, PyLong_NTT_PR)
        # print(c)

        # handle carry
        carry = 0
        for i in range(limit-1):
            carry += c[i]
            c[i] = carry & PyLong_MASK
            carry >>= PyLong_SHIFT
        if carry > 0:
            c[limit-1] = carry
        # print(c)
        ulong_unpad_(c)     # should we use this?
        # print(c)
        return c

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
    if s[-1] == 'L':
        s = s[:-1]
    res, base = [0], [1]
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
        elif type(x) is float:
            self.digits, self.sign = ulong_fromint(int(x))
        elif type(x) is str:
            self.digits, self.sign = ulong_fromstr(x)
        elif type(x) is long:
            self.digits, self.sign = x.digits.copy(), x.sign
        else:
            raise TypeError('expected int or str')

    def __add__(self, other):
        if type(other) is int:
            other = long(other)
        elif type(other) is not long:
            return NotImplemented
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
        elif type(other) is not long:
            return NotImplemented
        if self.sign != other.sign:
            return long((ulong_add(self.digits, other.digits), self.sign))
        cmp = ulong_cmp(self.digits, other.digits)
        if cmp == 0:
            return long(0)
        if cmp > 0:
            return long((ulong_sub(self.digits, other.digits), self.sign))
        else:
            return long((ulong_sub(other.digits, self.digits), -other.sign))
            
    def __rsub__(self, other):
        if type(other) is int:
            other = long(other)
        elif type(other) is not long:
            return NotImplemented
        return other.__sub__(self)
    
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
        return NotImplemented
    
    def __rmul__(self, other):
        return self.__mul__(other)
    
    #######################################################
    def __divmod__(self, other):
        if type(other) is int:
            assert type(other) is int and other > 0
            assert self.sign == 1
            q, r = ulong_divmodi(self.digits, other)
            return long((q, 1)), r
        raise NotImplementedError

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
        assert type(other) is int and other >= 0
        x = self.digits.copy()
        q, r = divmod(other, PyLong_SHIFT)
        x = [0]*q + x
        for _ in range(r): x = ulong_muli(x, 2)
        return long((x, self.sign))
    
    def __rshift__(self, other: int):
        assert type(other) is int and other >= 0
        x = self.digits.copy()
        q, r = divmod(other, PyLong_SHIFT)
        x = x[q:]
        if not x: return long(0)
        for _ in range(r): x = ulong_floordivi(x, 2)
        return long((x, self.sign))
    
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