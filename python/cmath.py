import math

class complex:
    def __init__(self, real, imag=0):
        self._real = float(real)
        self._imag = float(imag)

    @property
    def real(self):
        return self._real
    
    @property
    def imag(self):
        return self._imag

    def conjugate(self):
        return complex(self.real, -self.imag)
    
    def __repr__(self):
        s = ['(', str(self.real)]
        s.append('-' if self.imag < 0 else '+')
        s.append(str(abs(self.imag)))
        s.append('j)')
        return ''.join(s)
    
    def __eq__(self, other):
        if type(other) is complex:
            return self.real == other.real and self.imag == other.imag
        if type(other) in (int, float):
            return self.real == other and self.imag == 0
        return NotImplemented
    
    def __add__(self, other):
        if type(other) is complex:
            return complex(self.real + other.real, self.imag + other.imag)
        if type(other) in (int, float):
            return complex(self.real + other, self.imag)
        return NotImplemented
        
    def __radd__(self, other):
        return self.__add__(other)
    
    def __sub__(self, other):
        if type(other) is complex:
            return complex(self.real - other.real, self.imag - other.imag)
        if type(other) in (int, float):
            return complex(self.real - other, self.imag)
        return NotImplemented
    
    def __rsub__(self, other):
        if type(other) is complex:
            return complex(other.real - self.real, other.imag - self.imag)
        if type(other) in (int, float):
            return complex(other - self.real, -self.imag)
        return NotImplemented
    
    def __mul__(self, other):
        if type(other) is complex:
            return complex(self.real * other.real - self.imag * other.imag,
                           self.real * other.imag + self.imag * other.real)
        if type(other) in (int, float):
            return complex(self.real * other, self.imag * other)
        return NotImplemented
    
    def __rmul__(self, other):
        return self.__mul__(other)
    
    def __truediv__(self, other):
        if type(other) is complex:
            denominator = other.real ** 2 + other.imag ** 2
            real_part = (self.real * other.real + self.imag * other.imag) / denominator
            imag_part = (self.imag * other.real - self.real * other.imag) / denominator
            return complex(real_part, imag_part)
        if type(other) in (int, float):
            return complex(self.real / other, self.imag / other)
        return NotImplemented
    
    def __pow__(self, other: int | float):
        if type(other) in (int, float):
            return complex(self.__abs__() ** other * math.cos(other * phase(self)),
                           self.__abs__() ** other * math.sin(other * phase(self)))
        return NotImplemented
    
    def __abs__(self) -> float:
        return math.sqrt(self.real ** 2 + self.imag ** 2)

    def __neg__(self):
        return complex(-self.real, -self.imag)
    
    def __hash__(self):
        return hash((self.real, self.imag))


# Conversions to and from polar coordinates

def phase(z: complex):
    return math.atan2(z.imag, z.real)

def polar(z: complex):
    return z.__abs__(), phase(z)

def rect(r: float, phi: float):
    return r * math.cos(phi) + r * math.sin(phi) * 1j

# Power and logarithmic functions

def exp(z: complex):
    return math.exp(z.real) * rect(1, z.imag)

def log(z: complex, base=2.718281828459045):
    return math.log(z.__abs__(), base) + phase(z) * 1j

def log10(z: complex):
    return log(z, 10)

def sqrt(z: complex):
    return z ** 0.5

# Trigonometric functions

def acos(z: complex):
    return -1j * log(z + sqrt(z * z - 1))

def asin(z: complex):
    return -1j * log(1j * z + sqrt(1 - z * z))

def atan(z: complex):
    return 1j / 2 * log((1 - 1j * z) / (1 + 1j * z))

def cos(z: complex):
    return (exp(z) + exp(-z)) / 2

def sin(z: complex):
    return (exp(z) - exp(-z)) / (2 * 1j)

def tan(z: complex):
    return sin(z) / cos(z)

# Hyperbolic functions

def acosh(z: complex):
    return log(z + sqrt(z * z - 1))

def asinh(z: complex):
    return log(z + sqrt(z * z + 1))

def atanh(z: complex):
    return 1 / 2 * log((1 + z) / (1 - z))

def cosh(z: complex):
    return (exp(z) + exp(-z)) / 2

def sinh(z: complex):
    return (exp(z) - exp(-z)) / 2

def tanh(z: complex):
    return sinh(z) / cosh(z)

# Classification functions

def isfinite(z: complex):
    return math.isfinite(z.real) and math.isfinite(z.imag)

def isinf(z: complex):
    return math.isinf(z.real) or math.isinf(z.imag)

def isnan(z: complex):
    return math.isnan(z.real) or math.isnan(z.imag)

def isclose(a: complex, b: complex):
    return math.isclose(a.real, b.real) and math.isclose(a.imag, b.imag)

# Constants

pi = math.pi
e = math.e
tau = 2 * pi
inf = math.inf
infj = complex(0, inf)
nan = math.nan
nanj = complex(0, nan)
