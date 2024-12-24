// generated by prebuild.py
#include "pocketpy/common/_generated.h"
#include <string.h>
const char kPythonLibs_bisect[] = "\"\"\"Bisection algorithms.\"\"\"\n\ndef insort_right(a, x, lo=0, hi=None):\n    \"\"\"Insert item x in list a, and keep it sorted assuming a is sorted.\n\n    If x is already in a, insert it to the right of the rightmost x.\n\n    Optional args lo (default 0) and hi (default len(a)) bound the\n    slice of a to be searched.\n    \"\"\"\n\n    lo = bisect_right(a, x, lo, hi)\n    a.insert(lo, x)\n\ndef bisect_right(a, x, lo=0, hi=None):\n    \"\"\"Return the index where to insert item x in list a, assuming a is sorted.\n\n    The return value i is such that all e in a[:i] have e <= x, and all e in\n    a[i:] have e > x.  So if x already appears in the list, a.insert(x) will\n    insert just after the rightmost x already there.\n\n    Optional args lo (default 0) and hi (default len(a)) bound the\n    slice of a to be searched.\n    \"\"\"\n\n    if lo < 0:\n        raise ValueError('lo must be non-negative')\n    if hi is None:\n        hi = len(a)\n    while lo < hi:\n        mid = (lo+hi)//2\n        if x < a[mid]: hi = mid\n        else: lo = mid+1\n    return lo\n\ndef insort_left(a, x, lo=0, hi=None):\n    \"\"\"Insert item x in list a, and keep it sorted assuming a is sorted.\n\n    If x is already in a, insert it to the left of the leftmost x.\n\n    Optional args lo (default 0) and hi (default len(a)) bound the\n    slice of a to be searched.\n    \"\"\"\n\n    lo = bisect_left(a, x, lo, hi)\n    a.insert(lo, x)\n\n\ndef bisect_left(a, x, lo=0, hi=None):\n    \"\"\"Return the index where to insert item x in list a, assuming a is sorted.\n\n    The return value i is such that all e in a[:i] have e < x, and all e in\n    a[i:] have e >= x.  So if x already appears in the list, a.insert(x) will\n    insert just before the leftmost x already there.\n\n    Optional args lo (default 0) and hi (default len(a)) bound the\n    slice of a to be searched.\n    \"\"\"\n\n    if lo < 0:\n        raise ValueError('lo must be non-negative')\n    if hi is None:\n        hi = len(a)\n    while lo < hi:\n        mid = (lo+hi)//2\n        if a[mid] < x: lo = mid+1\n        else: hi = mid\n    return lo\n\n# Create aliases\nbisect = bisect_right\ninsort = insort_right\n";
const char kPythonLibs_builtins[] = "def all(iterable):\n    for i in iterable:\n        if not i:\n            return False\n    return True\n\ndef any(iterable):\n    for i in iterable:\n        if i:\n            return True\n    return False\n\ndef enumerate(iterable, start=0):\n    n = start\n    for elem in iterable:\n        yield n, elem\n        n += 1\n\ndef __minmax_reduce(op, args):\n    if len(args) == 2:  # min(1, 2)\n        return args[0] if op(args[0], args[1]) else args[1]\n    if len(args) == 0:  # min()\n        raise TypeError('expected 1 arguments, got 0')\n    if len(args) == 1:  # min([1, 2, 3, 4]) -> min(1, 2, 3, 4)\n        args = args[0]\n    args = iter(args)\n    try:\n        res = next(args)\n    except StopIteration:\n        raise ValueError('args is an empty sequence')\n    while True:\n        try:\n            i = next(args)\n        except StopIteration:\n            break\n        if op(i, res):\n            res = i\n    return res\n\ndef min(*args, key=None):\n    key = key or (lambda x: x)\n    return __minmax_reduce(lambda x,y: key(x)<key(y), args)\n\ndef max(*args, key=None):\n    key = key or (lambda x: x)\n    return __minmax_reduce(lambda x,y: key(x)>key(y), args)\n\ndef sum(iterable):\n    res = 0\n    for i in iterable:\n        res += i\n    return res\n\ndef map(f, iterable):\n    for i in iterable:\n        yield f(i)\n\ndef filter(f, iterable):\n    for i in iterable:\n        if f(i):\n            yield i\n\ndef zip(a, b):\n    a = iter(a)\n    b = iter(b)\n    while True:\n        try:\n            ai = next(a)\n            bi = next(b)\n        except StopIteration:\n            break\n        yield ai, bi\n\ndef reversed(iterable):\n    a = list(iterable)\n    a.reverse()\n    return a\n\ndef sorted(iterable, key=None, reverse=False):\n    a = list(iterable)\n    a.sort(key=key, reverse=reverse)\n    return a\n\n##### str #####\ndef __format_string(self: str, *args, **kwargs) -> str:\n    def tokenizeString(s: str):\n        tokens = []\n        L, R = 0,0\n        \n        mode = None\n        curArg = 0\n        # lookingForKword = False\n        \n        while(R<len(s)):\n            curChar = s[R]\n            nextChar = s[R+1] if R+1<len(s) else ''\n            \n            # Invalid case 1: stray '}' encountered, example: \"ABCD EFGH {name} IJKL}\", \"Hello {vv}}\", \"HELLO {0} WORLD}\"\n            if curChar == '}' and nextChar != '}':\n                raise ValueError(\"Single '}' encountered in format string\")        \n            \n            # Valid Case 1: Escaping case, we escape \"{{ or \"}}\" to be \"{\" or \"}\", example: \"{{}}\", \"{{My Name is {0}}}\"\n            if (curChar == '{' and nextChar == '{') or (curChar == '}' and nextChar == '}'):\n                \n                if (L<R): # Valid Case 1.1: make sure we are not adding empty string\n                    tokens.append(s[L:R]) # add the string before the escape\n                \n                \n                tokens.append(curChar) # Valid Case 1.2: add the escape char\n                L = R+2 # move the left pointer to the next char\n                R = R+2 # move the right pointer to the next char\n                continue\n            \n            # Valid Case 2: Regular command line arg case: example:  \"ABCD EFGH {} IJKL\", \"{}\", \"HELLO {} WORLD\"\n            elif curChar == '{' and nextChar == '}':\n                if mode is not None and mode != 'auto':\n                    # Invalid case 2: mixing automatic and manual field specifications -- example: \"ABCD EFGH {name} IJKL {}\", \"Hello {vv} {}\", \"HELLO {0} WORLD {}\" \n                    raise ValueError(\"Cannot switch from manual field numbering to automatic field specification\")\n                \n                mode = 'auto'\n                if(L<R): # Valid Case 2.1: make sure we are not adding empty string\n                    tokens.append(s[L:R]) # add the string before the special marker for the arg\n                \n                tokens.append(\"{\"+str(curArg)+\"}\") # Valid Case 2.2: add the special marker for the arg\n                curArg+=1 # increment the arg position, this will be used for referencing the arg later\n                \n                L = R+2 # move the left pointer to the next char\n                R = R+2 # move the right pointer to the next char\n                continue\n            \n            # Valid Case 3: Key-word arg case: example: \"ABCD EFGH {name} IJKL\", \"Hello {vv}\", \"HELLO {name} WORLD\"\n            elif (curChar == '{'):\n                \n                if mode is not None and mode != 'manual':\n                    # # Invalid case 2: mixing automatic and manual field specifications -- example: \"ABCD EFGH {} IJKL {name}\", \"Hello {} {1}\", \"HELLO {} WORLD {name}\"\n                    raise ValueError(\"Cannot switch from automatic field specification to manual field numbering\")\n                \n                mode = 'manual'\n                \n                if(L<R): # Valid case 3.1: make sure we are not adding empty string\n                    tokens.append(s[L:R]) # add the string before the special marker for the arg\n                \n                # We look for the end of the keyword          \n                kwL = R # Keyword left pointer\n                kwR = R+1 # Keyword right pointer\n                while(kwR<len(s) and s[kwR]!='}'):\n                    if s[kwR] == '{': # Invalid case 3: stray '{' encountered, example: \"ABCD EFGH {n{ame} IJKL {\", \"Hello {vv{}}\", \"HELLO {0} WOR{LD}\"\n                        raise ValueError(\"Unexpected '{' in field name\")\n                    kwR += 1\n                \n                # Valid case 3.2: We have successfully found the end of the keyword\n                if kwR<len(s) and s[kwR] == '}':\n                    tokens.append(s[kwL:kwR+1]) # add the special marker for the arg\n                    L = kwR+1\n                    R = kwR+1\n                    \n                # Invalid case 4: We didn't find the end of the keyword, throw error\n                else:\n                    raise ValueError(\"Expected '}' before end of string\")\n                continue\n            \n            R = R+1\n        \n        \n        # Valid case 4: We have reached the end of the string, add the remaining string to the tokens \n        if L<R:\n            tokens.append(s[L:R])\n                \n        # print(tokens)\n        return tokens\n\n    tokens = tokenizeString(self)\n    argMap = {}\n    for i, a in enumerate(args):\n        argMap[str(i)] = a\n    final_tokens = []\n    for t in tokens:\n        if t[0] == '{' and t[-1] == '}':\n            key = t[1:-1]\n            argMapVal = argMap.get(key, None)\n            kwargsVal = kwargs.get(key, None)\n                                    \n            if argMapVal is None and kwargsVal is None:\n                raise ValueError(\"No arg found for token: \"+t)\n            elif argMapVal is not None:\n                final_tokens.append(str(argMapVal))\n            else:\n                final_tokens.append(str(kwargsVal))\n        else:\n            final_tokens.append(t)\n    \n    return ''.join(final_tokens)\n\nstr.format = __format_string\ndel __format_string\n\n\ndef help(obj):\n    if hasattr(obj, '__func__'):\n        obj = obj.__func__\n    # print(obj.__signature__)\n    if obj.__doc__:\n        print(obj.__doc__)\n\ndef complex(real, imag=0):\n    import cmath\n    return cmath.complex(real, imag)\n\n\nclass set:\n    def __init__(self, iterable=None):\n        iterable = iterable or []\n        self._a = {}\n        self.update(iterable)\n\n    def add(self, elem):\n        self._a[elem] = None\n        \n    def discard(self, elem):\n        self._a.pop(elem, None)\n\n    def remove(self, elem):\n        del self._a[elem]\n        \n    def clear(self):\n        self._a.clear()\n\n    def update(self, other):\n        for elem in other:\n            self.add(elem)\n\n    def __len__(self):\n        return len(self._a)\n    \n    def copy(self):\n        return set(self._a.keys())\n    \n    def __and__(self, other):\n        return {elem for elem in self if elem in other}\n\n    def __sub__(self, other):\n        return {elem for elem in self if elem not in other}\n    \n    def __or__(self, other):\n        ret = self.copy()\n        ret.update(other)\n        return ret\n\n    def __xor__(self, other): \n        _0 = self - other\n        _1 = other - self\n        return _0 | _1\n\n    def union(self, other):\n        return self | other\n\n    def intersection(self, other):\n        return self & other\n\n    def difference(self, other):\n        return self - other\n\n    def symmetric_difference(self, other):      \n        return self ^ other\n    \n    def __eq__(self, other):\n        if not isinstance(other, set):\n            return NotImplemented\n        return len(self ^ other) == 0\n    \n    def __ne__(self, other):\n        if not isinstance(other, set):\n            return NotImplemented\n        return len(self ^ other) != 0\n\n    def isdisjoint(self, other):\n        return len(self & other) == 0\n    \n    def issubset(self, other):\n        return len(self - other) == 0\n    \n    def issuperset(self, other):\n        return len(other - self) == 0\n\n    def __contains__(self, elem):\n        return elem in self._a\n    \n    def __repr__(self):\n        if len(self) == 0:\n            return 'set()'\n        return '{'+ ', '.join([repr(i) for i in self._a.keys()]) + '}'\n    \n    def __iter__(self):\n        return iter(self._a.keys())";
const char kPythonLibs_cmath[] = "import math\n\nclass complex:\n    def __init__(self, real, imag=0):\n        self._real = float(real)\n        self._imag = float(imag)\n\n    @property\n    def real(self):\n        return self._real\n    \n    @property\n    def imag(self):\n        return self._imag\n\n    def conjugate(self):\n        return complex(self.real, -self.imag)\n    \n    def __repr__(self):\n        s = ['(', str(self.real)]\n        s.append('-' if self.imag < 0 else '+')\n        s.append(str(abs(self.imag)))\n        s.append('j)')\n        return ''.join(s)\n    \n    def __eq__(self, other):\n        if type(other) is complex:\n            return self.real == other.real and self.imag == other.imag\n        if type(other) in (int, float):\n            return self.real == other and self.imag == 0\n        return NotImplemented\n    \n    def __ne__(self, other):\n        res = self == other\n        if res is NotImplemented:\n            return res\n        return not res\n    \n    def __add__(self, other):\n        if type(other) is complex:\n            return complex(self.real + other.real, self.imag + other.imag)\n        if type(other) in (int, float):\n            return complex(self.real + other, self.imag)\n        return NotImplemented\n        \n    def __radd__(self, other):\n        return self.__add__(other)\n    \n    def __sub__(self, other):\n        if type(other) is complex:\n            return complex(self.real - other.real, self.imag - other.imag)\n        if type(other) in (int, float):\n            return complex(self.real - other, self.imag)\n        return NotImplemented\n    \n    def __rsub__(self, other):\n        if type(other) is complex:\n            return complex(other.real - self.real, other.imag - self.imag)\n        if type(other) in (int, float):\n            return complex(other - self.real, -self.imag)\n        return NotImplemented\n    \n    def __mul__(self, other):\n        if type(other) is complex:\n            return complex(self.real * other.real - self.imag * other.imag,\n                           self.real * other.imag + self.imag * other.real)\n        if type(other) in (int, float):\n            return complex(self.real * other, self.imag * other)\n        return NotImplemented\n    \n    def __rmul__(self, other):\n        return self.__mul__(other)\n    \n    def __truediv__(self, other):\n        if type(other) is complex:\n            denominator = other.real ** 2 + other.imag ** 2\n            real_part = (self.real * other.real + self.imag * other.imag) / denominator\n            imag_part = (self.imag * other.real - self.real * other.imag) / denominator\n            return complex(real_part, imag_part)\n        if type(other) in (int, float):\n            return complex(self.real / other, self.imag / other)\n        return NotImplemented\n    \n    def __pow__(self, other: int | float):\n        if type(other) in (int, float):\n            return complex(self.__abs__() ** other * math.cos(other * phase(self)),\n                           self.__abs__() ** other * math.sin(other * phase(self)))\n        return NotImplemented\n    \n    def __abs__(self) -> float:\n        return math.sqrt(self.real ** 2 + self.imag ** 2)\n\n    def __neg__(self):\n        return complex(-self.real, -self.imag)\n    \n    def __hash__(self):\n        return hash((self.real, self.imag))\n\n\n# Conversions to and from polar coordinates\n\ndef phase(z: complex):\n    return math.atan2(z.imag, z.real)\n\ndef polar(z: complex):\n    return z.__abs__(), phase(z)\n\ndef rect(r: float, phi: float):\n    return r * math.cos(phi) + r * math.sin(phi) * 1j\n\n# Power and logarithmic functions\n\ndef exp(z: complex):\n    return math.exp(z.real) * rect(1, z.imag)\n\ndef log(z: complex, base=2.718281828459045):\n    return math.log(z.__abs__(), base) + phase(z) * 1j\n\ndef log10(z: complex):\n    return log(z, 10)\n\ndef sqrt(z: complex):\n    return z ** 0.5\n\n# Trigonometric functions\n\ndef acos(z: complex):\n    return -1j * log(z + sqrt(z * z - 1))\n\ndef asin(z: complex):\n    return -1j * log(1j * z + sqrt(1 - z * z))\n\ndef atan(z: complex):\n    return 1j / 2 * log((1 - 1j * z) / (1 + 1j * z))\n\ndef cos(z: complex):\n    return (exp(z) + exp(-z)) / 2\n\ndef sin(z: complex):\n    return (exp(z) - exp(-z)) / (2 * 1j)\n\ndef tan(z: complex):\n    return sin(z) / cos(z)\n\n# Hyperbolic functions\n\ndef acosh(z: complex):\n    return log(z + sqrt(z * z - 1))\n\ndef asinh(z: complex):\n    return log(z + sqrt(z * z + 1))\n\ndef atanh(z: complex):\n    return 1 / 2 * log((1 + z) / (1 - z))\n\ndef cosh(z: complex):\n    return (exp(z) + exp(-z)) / 2\n\ndef sinh(z: complex):\n    return (exp(z) - exp(-z)) / 2\n\ndef tanh(z: complex):\n    return sinh(z) / cosh(z)\n\n# Classification functions\n\ndef isfinite(z: complex):\n    return math.isfinite(z.real) and math.isfinite(z.imag)\n\ndef isinf(z: complex):\n    return math.isinf(z.real) or math.isinf(z.imag)\n\ndef isnan(z: complex):\n    return math.isnan(z.real) or math.isnan(z.imag)\n\ndef isclose(a: complex, b: complex):\n    return math.isclose(a.real, b.real) and math.isclose(a.imag, b.imag)\n\n# Constants\n\npi = math.pi\ne = math.e\ntau = 2 * pi\ninf = math.inf\ninfj = complex(0, inf)\nnan = math.nan\nnanj = complex(0, nan)\n";
const char kPythonLibs_collections[] = "from typing import Generic, TypeVar, Iterable\n\nT = TypeVar('T')\n\ndef Counter(iterable: Iterable[T]):\n    a: dict[T, int] = {}\n    for x in iterable:\n        if x in a:\n            a[x] += 1\n        else:\n            a[x] = 1\n    return a\n\n\nclass defaultdict(dict):\n    def __init__(self, default_factory, *args):\n        super().__init__(*args)\n        self.default_factory = default_factory\n\n    def __missing__(self, key):\n        self[key] = self.default_factory()\n        return self[key]\n\n    def __repr__(self) -> str:\n        return f\"defaultdict({self.default_factory}, {super().__repr__()})\"\n\n    def copy(self):\n        return defaultdict(self.default_factory, self)\n\n\nclass deque(Generic[T]):\n    _data: list[T]\n    _head: int\n    _tail: int\n    _capacity: int\n\n    def __init__(self, iterable: Iterable[T] = None):\n        self._data = [None] * 8 # type: ignore\n        self._head = 0\n        self._tail = 0\n        self._capacity = len(self._data)\n\n        if iterable is not None:\n            self.extend(iterable)\n\n    def __resize_2x(self):\n        backup = list(self)\n        self._capacity *= 2\n        self._head = 0\n        self._tail = len(backup)\n        self._data.clear()\n        self._data.extend(backup)\n        self._data.extend([None] * (self._capacity - len(backup)))\n\n    def append(self, x: T):\n        self._data[self._tail] = x\n        self._tail = (self._tail + 1) % self._capacity\n        if (self._tail + 1) % self._capacity == self._head:\n            self.__resize_2x()\n\n    def appendleft(self, x: T):\n        self._head = (self._head - 1 + self._capacity) % self._capacity\n        self._data[self._head] = x\n        if (self._tail + 1) % self._capacity == self._head:\n            self.__resize_2x()\n\n    def copy(self):\n        return deque(self)\n    \n    def count(self, x: T) -> int:\n        n = 0\n        for item in self:\n            if item == x:\n                n += 1\n        return n\n    \n    def extend(self, iterable: Iterable[T]):\n        for x in iterable:\n            self.append(x)\n\n    def extendleft(self, iterable: Iterable[T]):\n        for x in iterable:\n            self.appendleft(x)\n    \n    def pop(self) -> T:\n        if self._head == self._tail:\n            raise IndexError(\"pop from an empty deque\")\n        self._tail = (self._tail - 1 + self._capacity) % self._capacity\n        return self._data[self._tail]\n    \n    def popleft(self) -> T:\n        if self._head == self._tail:\n            raise IndexError(\"pop from an empty deque\")\n        x = self._data[self._head]\n        self._head = (self._head + 1) % self._capacity\n        return x\n    \n    def clear(self):\n        i = self._head\n        while i != self._tail:\n            self._data[i] = None # type: ignore\n            i = (i + 1) % self._capacity\n        self._head = 0\n        self._tail = 0\n\n    def rotate(self, n: int = 1):\n        if len(self) == 0:\n            return\n        if n > 0:\n            n = n % len(self)\n            for _ in range(n):\n                self.appendleft(self.pop())\n        elif n < 0:\n            n = -n % len(self)\n            for _ in range(n):\n                self.append(self.popleft())\n\n    def __len__(self) -> int:\n        return (self._tail - self._head + self._capacity) % self._capacity\n\n    def __contains__(self, x: object) -> bool:\n        for item in self:\n            if item == x:\n                return True\n        return False\n    \n    def __iter__(self):\n        i = self._head\n        while i != self._tail:\n            yield self._data[i]\n            i = (i + 1) % self._capacity\n\n    def __eq__(self, other: object) -> bool:\n        if not isinstance(other, deque):\n            return NotImplemented\n        if len(self) != len(other):\n            return False\n        for x, y in zip(self, other):\n            if x != y:\n                return False\n        return True\n    \n    def __ne__(self, other: object) -> bool:\n        if not isinstance(other, deque):\n            return NotImplemented\n        return not self == other\n    \n    def __repr__(self) -> str:\n        return f\"deque({list(self)!r})\"\n\n";
const char kPythonLibs_dataclasses[] = "def _get_annotations(cls: type):\n    inherits = []\n    while cls is not object:\n        inherits.append(cls)\n        cls = cls.__base__\n    inherits.reverse()\n    res = {}\n    for cls in inherits:\n        res.update(cls.__annotations__)\n    return res.keys()\n\ndef _wrapped__init__(self, *args, **kwargs):\n    cls = type(self)\n    cls_d = cls.__dict__\n    fields = _get_annotations(cls)\n    i = 0   # index into args\n    for field in fields:\n        if field in kwargs:\n            setattr(self, field, kwargs.pop(field))\n        else:\n            if i < len(args):\n                setattr(self, field, args[i])\n                i += 1\n            elif field in cls_d:    # has default value\n                setattr(self, field, cls_d[field])\n            else:\n                raise TypeError(f\"{cls.__name__} missing required argument {field!r}\")\n    if len(args) > i:\n        raise TypeError(f\"{cls.__name__} takes {len(fields)} positional arguments but {len(args)} were given\")\n    if len(kwargs) > 0:\n        raise TypeError(f\"{cls.__name__} got an unexpected keyword argument {next(iter(kwargs))!r}\")\n\ndef _wrapped__repr__(self):\n    fields = _get_annotations(type(self))\n    obj_d = self.__dict__\n    args: list = [f\"{field}={obj_d[field]!r}\" for field in fields]\n    return f\"{type(self).__name__}({', '.join(args)})\"\n\ndef _wrapped__eq__(self, other):\n    if type(self) is not type(other):\n        return False\n    fields = _get_annotations(type(self))\n    for field in fields:\n        if getattr(self, field) != getattr(other, field):\n            return False\n    return True\n\ndef _wrapped__ne__(self, other):\n    return not self.__eq__(other)\n\ndef dataclass(cls: type):\n    assert type(cls) is type\n    cls_d = cls.__dict__\n    if '__init__' not in cls_d:\n        cls.__init__ = _wrapped__init__\n    if '__repr__' not in cls_d:\n        cls.__repr__ = _wrapped__repr__\n    if '__eq__' not in cls_d:\n        cls.__eq__ = _wrapped__eq__\n    if '__ne__' not in cls_d:\n        cls.__ne__ = _wrapped__ne__\n    fields = _get_annotations(cls)\n    has_default = False\n    for field in fields:\n        if field in cls_d:\n            has_default = True\n        else:\n            if has_default:\n                raise TypeError(f\"non-default argument {field!r} follows default argument\")\n    return cls\n\ndef asdict(obj) -> dict:\n    fields = _get_annotations(type(obj))\n    obj_d = obj.__dict__\n    return {field: obj_d[field] for field in fields}";
const char kPythonLibs_datetime[] = "from time import localtime\nimport operator\n\nclass timedelta:\n    def __init__(self, days=0, seconds=0):\n        self.days = days\n        self.seconds = seconds\n\n    def __repr__(self):\n        return f\"datetime.timedelta(days={self.days}, seconds={self.seconds})\"\n\n    def __eq__(self, other) -> bool:\n        if not isinstance(other, timedelta):\n            return NotImplemented\n        return (self.days, self.seconds) == (other.days, other.seconds)\n\n    def __ne__(self, other) -> bool:\n        if not isinstance(other, timedelta):\n            return NotImplemented\n        return (self.days, self.seconds) != (other.days, other.seconds)\n\n\nclass date:\n    def __init__(self, year: int, month: int, day: int):\n        self.year = year\n        self.month = month\n        self.day = day\n\n    @staticmethod\n    def today():\n        t = localtime()\n        return date(t.tm_year, t.tm_mon, t.tm_mday)\n    \n    def __cmp(self, other, op):\n        if not isinstance(other, date):\n            return NotImplemented\n        if self.year != other.year:\n            return op(self.year, other.year)\n        if self.month != other.month:\n            return op(self.month, other.month)\n        return op(self.day, other.day)\n\n    def __eq__(self, other) -> bool:\n        return self.__cmp(other, operator.eq)\n    \n    def __ne__(self, other) -> bool:\n        return self.__cmp(other, operator.ne)\n\n    def __lt__(self, other: 'date') -> bool:\n        return self.__cmp(other, operator.lt)\n\n    def __le__(self, other: 'date') -> bool:\n        return self.__cmp(other, operator.le)\n\n    def __gt__(self, other: 'date') -> bool:\n        return self.__cmp(other, operator.gt)\n\n    def __ge__(self, other: 'date') -> bool:\n        return self.__cmp(other, operator.ge)\n\n    def __str__(self):\n        return f\"{self.year}-{self.month:02}-{self.day:02}\"\n\n    def __repr__(self):\n        return f\"datetime.date({self.year}, {self.month}, {self.day})\"\n\n\nclass datetime(date):\n    def __init__(self, year: int, month: int, day: int, hour: int, minute: int, second: int):\n        super().__init__(year, month, day)\n        # Validate and set hour, minute, and second\n        if not 0 <= hour <= 23:\n            raise ValueError(\"Hour must be between 0 and 23\")\n        self.hour = hour\n        if not 0 <= minute <= 59:\n            raise ValueError(\"Minute must be between 0 and 59\")\n        self.minute = minute\n        if not 0 <= second <= 59:\n            raise ValueError(\"Second must be between 0 and 59\")\n        self.second = second\n\n    def date(self) -> date:\n        return date(self.year, self.month, self.day)\n\n    @staticmethod\n    def now():\n        t = localtime()\n        tm_sec = t.tm_sec\n        if tm_sec == 60:\n            tm_sec = 59\n        return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, tm_sec)\n\n    def __str__(self):\n        return f\"{self.year}-{self.month:02}-{self.day:02} {self.hour:02}:{self.minute:02}:{self.second:02}\"\n\n    def __repr__(self):\n        return f\"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})\"\n\n    def __cmp(self, other, op):\n        if not isinstance(other, datetime):\n            return NotImplemented\n        if self.year != other.year:\n            return op(self.year, other.year)\n        if self.month != other.month:\n            return op(self.month, other.month)\n        if self.day != other.day:\n            return op(self.day, other.day)\n        if self.hour != other.hour:\n            return op(self.hour, other.hour)\n        if self.minute != other.minute:\n            return op(self.minute, other.minute)\n        return op(self.second, other.second)\n\n    def __eq__(self, other) -> bool:\n        return self.__cmp(other, operator.eq)\n    \n    def __ne__(self, other) -> bool:\n        return self.__cmp(other, operator.ne)\n    \n    def __lt__(self, other) -> bool:\n        return self.__cmp(other, operator.lt)\n    \n    def __le__(self, other) -> bool:\n        return self.__cmp(other, operator.le)\n    \n    def __gt__(self, other) -> bool:\n        return self.__cmp(other, operator.gt)\n    \n    def __ge__(self, other) -> bool:\n        return self.__cmp(other, operator.ge)\n\n\n";
const char kPythonLibs_functools[] = "class cache:\n    def __init__(self, f):\n        self.f = f\n        self.cache = {}\n\n    def __call__(self, *args):\n        if args not in self.cache:\n            self.cache[args] = self.f(*args)\n        return self.cache[args]\n    \ndef reduce(function, sequence, initial=...):\n    it = iter(sequence)\n    if initial is ...:\n        try:\n            value = next(it)\n        except StopIteration:\n            raise TypeError(\"reduce() of empty sequence with no initial value\")\n    else:\n        value = initial\n    for element in it:\n        value = function(value, element)\n    return value\n\nclass partial:\n    def __init__(self, f, *args, **kwargs):\n        self.f = f\n        if not callable(f):\n            raise TypeError(\"the first argument must be callable\")\n        self.args = args\n        self.kwargs = kwargs\n\n    def __call__(self, *args, **kwargs):\n        kwargs.update(self.kwargs)\n        return self.f(*self.args, *args, **kwargs)\n\n";
const char kPythonLibs_heapq[] = "# Heap queue algorithm (a.k.a. priority queue)\ndef heappush(heap, item):\n    \"\"\"Push item onto heap, maintaining the heap invariant.\"\"\"\n    heap.append(item)\n    _siftdown(heap, 0, len(heap)-1)\n\ndef heappop(heap):\n    \"\"\"Pop the smallest item off the heap, maintaining the heap invariant.\"\"\"\n    lastelt = heap.pop()    # raises appropriate IndexError if heap is empty\n    if heap:\n        returnitem = heap[0]\n        heap[0] = lastelt\n        _siftup(heap, 0)\n        return returnitem\n    return lastelt\n\ndef heapreplace(heap, item):\n    \"\"\"Pop and return the current smallest value, and add the new item.\n\n    This is more efficient than heappop() followed by heappush(), and can be\n    more appropriate when using a fixed-size heap.  Note that the value\n    returned may be larger than item!  That constrains reasonable uses of\n    this routine unless written as part of a conditional replacement:\n\n        if item > heap[0]:\n            item = heapreplace(heap, item)\n    \"\"\"\n    returnitem = heap[0]    # raises appropriate IndexError if heap is empty\n    heap[0] = item\n    _siftup(heap, 0)\n    return returnitem\n\ndef heappushpop(heap, item):\n    \"\"\"Fast version of a heappush followed by a heappop.\"\"\"\n    if heap and heap[0] < item:\n        item, heap[0] = heap[0], item\n        _siftup(heap, 0)\n    return item\n\ndef heapify(x):\n    \"\"\"Transform list into a heap, in-place, in O(len(x)) time.\"\"\"\n    n = len(x)\n    # Transform bottom-up.  The largest index there's any point to looking at\n    # is the largest with a child index in-range, so must have 2*i + 1 < n,\n    # or i < (n-1)/2.  If n is even = 2*j, this is (2*j-1)/2 = j-1/2 so\n    # j-1 is the largest, which is n//2 - 1.  If n is odd = 2*j+1, this is\n    # (2*j+1-1)/2 = j so j-1 is the largest, and that's again n//2-1.\n    for i in reversed(range(n//2)):\n        _siftup(x, i)\n\n# 'heap' is a heap at all indices >= startpos, except possibly for pos.  pos\n# is the index of a leaf with a possibly out-of-order value.  Restore the\n# heap invariant.\ndef _siftdown(heap, startpos, pos):\n    newitem = heap[pos]\n    # Follow the path to the root, moving parents down until finding a place\n    # newitem fits.\n    while pos > startpos:\n        parentpos = (pos - 1) >> 1\n        parent = heap[parentpos]\n        if newitem < parent:\n            heap[pos] = parent\n            pos = parentpos\n            continue\n        break\n    heap[pos] = newitem\n\ndef _siftup(heap, pos):\n    endpos = len(heap)\n    startpos = pos\n    newitem = heap[pos]\n    # Bubble up the smaller child until hitting a leaf.\n    childpos = 2*pos + 1    # leftmost child position\n    while childpos < endpos:\n        # Set childpos to index of smaller child.\n        rightpos = childpos + 1\n        if rightpos < endpos and not heap[childpos] < heap[rightpos]:\n            childpos = rightpos\n        # Move the smaller child up.\n        heap[pos] = heap[childpos]\n        pos = childpos\n        childpos = 2*pos + 1\n    # The leaf at pos is empty now.  Put newitem there, and bubble it up\n    # to its final resting place (by sifting its parents down).\n    heap[pos] = newitem\n    _siftdown(heap, startpos, pos)";
const char kPythonLibs_itertools[] = "class chain:\n    def __init__(self, *iterable):\n        self.iterable = iterable\n\n    @classmethod\n    def from_iterable(cls, iterable):\n        for it in iterable:\n            yield from it\n\n    def __iter__(self):\n        for it in self.iterable:\n            yield from it\n";
const char kPythonLibs_operator[] = "# https://docs.python.org/3/library/operator.html#mapping-operators-to-functions\n\ndef le(a, b): return a <= b\ndef lt(a, b): return a < b\ndef ge(a, b): return a >= b\ndef gt(a, b): return a > b\ndef eq(a, b): return a == b\ndef ne(a, b): return a != b\n\ndef and_(a, b): return a & b\ndef or_(a, b): return a | b\ndef xor(a, b): return a ^ b\ndef invert(a): return ~a\ndef lshift(a, b): return a << b\ndef rshift(a, b): return a >> b\n\ndef is_(a, b): return a is b\ndef is_not(a, b): return a is not b\ndef not_(a): return not a\ndef truth(a): return bool(a)\ndef contains(a, b): return b in a\n\ndef add(a, b): return a + b\ndef sub(a, b): return a - b\ndef mul(a, b): return a * b\ndef truediv(a, b): return a / b\ndef floordiv(a, b): return a // b\ndef mod(a, b): return a % b\ndef pow(a, b): return a ** b\ndef neg(a): return -a\ndef matmul(a, b): return a @ b\n\ndef getitem(a, b): return a[b]\ndef setitem(a, b, c): a[b] = c\ndef delitem(a, b): del a[b]\n\ndef iadd(a, b): a += b; return a\ndef isub(a, b): a -= b; return a\ndef imul(a, b): a *= b; return a\ndef itruediv(a, b): a /= b; return a\ndef ifloordiv(a, b): a //= b; return a\ndef imod(a, b): a %= b; return a\n# def ipow(a, b): a **= b; return a\n# def imatmul(a, b): a @= b; return a\ndef iand(a, b): a &= b; return a\ndef ior(a, b): a |= b; return a\ndef ixor(a, b): a ^= b; return a\ndef ilshift(a, b): a <<= b; return a\ndef irshift(a, b): a >>= b; return a\n";
const char kPythonLibs_this[] = "print(\"\"\"The Zen of Python, by Tim Peters\n\nBeautiful is better than ugly.\nExplicit is better than implicit.\nSimple is better than complex.\nComplex is better than complicated.\nFlat is better than nested.\nSparse is better than dense.\nReadability counts.\nSpecial cases aren't special enough to break the rules.\nAlthough practicality beats purity.\nErrors should never pass silently.\nUnless explicitly silenced.\nIn the face of ambiguity, refuse the temptation to guess.\nThere should be one-- and preferably only one --obvious way to do it.\nAlthough that way may not be obvious at first unless you're Dutch.\nNow is better than never.\nAlthough never is often better than *right* now.\nIf the implementation is hard to explain, it's a bad idea.\nIf the implementation is easy to explain, it may be a good idea.\nNamespaces are one honking great idea -- let's do more of those!\"\"\")";
const char kPythonLibs_typing[] = "class _Placeholder:\n    def __init__(self, *args, **kwargs):\n        pass\n    def __getitem__(self, *args):\n        return self\n    def __call__(self, *args, **kwargs):\n        return self\n    def __and__(self, other):\n        return self\n    def __or__(self, other):\n        return self\n    def __xor__(self, other):\n        return self\n\n\n_PLACEHOLDER = _Placeholder()\n\nList = _PLACEHOLDER\nDict = _PLACEHOLDER\nTuple = _PLACEHOLDER\nSet = _PLACEHOLDER\nAny = _PLACEHOLDER\nUnion = _PLACEHOLDER\nOptional = _PLACEHOLDER\nCallable = _PLACEHOLDER\nType = _PLACEHOLDER\n\nLiteral = _PLACEHOLDER\nLiteralString = _PLACEHOLDER\n\nIterable = _PLACEHOLDER\nGenerator = _PLACEHOLDER\nIterator = _PLACEHOLDER\n\nHashable = _PLACEHOLDER\n\nTypeVar = _PLACEHOLDER\nSelf = _PLACEHOLDER\n\nProtocol = object\nGeneric = object\n\nTYPE_CHECKING = False\n\n# decorators\noverload = lambda x: x\nfinal = lambda x: x\n";

const char* load_kPythonLib(const char* name) {
    if (strchr(name, '.') != NULL) return NULL;
    if (strcmp(name, "bisect") == 0) return kPythonLibs_bisect;
    if (strcmp(name, "builtins") == 0) return kPythonLibs_builtins;
    if (strcmp(name, "cmath") == 0) return kPythonLibs_cmath;
    if (strcmp(name, "collections") == 0) return kPythonLibs_collections;
    if (strcmp(name, "dataclasses") == 0) return kPythonLibs_dataclasses;
    if (strcmp(name, "datetime") == 0) return kPythonLibs_datetime;
    if (strcmp(name, "functools") == 0) return kPythonLibs_functools;
    if (strcmp(name, "heapq") == 0) return kPythonLibs_heapq;
    if (strcmp(name, "itertools") == 0) return kPythonLibs_itertools;
    if (strcmp(name, "operator") == 0) return kPythonLibs_operator;
    if (strcmp(name, "this") == 0) return kPythonLibs_this;
    if (strcmp(name, "typing") == 0) return kPythonLibs_typing;
    return NULL;
}
