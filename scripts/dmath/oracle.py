"""Independent binary64 references. No host libm and no calls to dmath.

Integer/Fraction arithmetic handles exact operations. Decimal handles roots,
exponentials and logs; Machin's formula and convergent series handle angles.
The generator requires identical rounded results at two different precisions.
"""

from decimal import Decimal as D, getcontext, ROUND_HALF_EVEN
from fractions import Fraction
from functools import lru_cache
import struct

SIGN = 1 << 63
INF = 0x7FF0000000000000
NAN = 0x7FF8000000000000
MASK = SIGN - 1


def bits(x):
    return struct.unpack('>Q', struct.pack('>d', x))[0]


def value(u):
    return struct.unpack('>d', struct.pack('>Q', u))[0]


def rounded(x):
    if isinstance(x, D) and x.is_nan():
        return NAN
    try:
        return bits(float(x))
    except OverflowError:
        return INF | (SIGN if x < 0 else 0)


def atan_series(x):
    term = total = x
    square = -x * x
    n = 1
    while True:
        term *= square
        after = total + term / (2 * n + 1)
        if after == total:
            return total
        total = after
        n += 1


@lru_cache(None)
def pi(precision):
    assert precision == getcontext().prec
    return 16 * atan_series(D(1) / 5) - 4 * atan_series(D(1) / 239)


def atan(x):
    if x.is_zero():
        return x
    if x.is_signed():
        return -atan(-x)
    if x > 1:
        return pi(getcontext().prec) / 2 - atan(1 / x)
    scale = 1
    while x > D('0.03125'):
        x = x / (1 + (1 + x * x).sqrt())
        scale *= 2
    return scale * atan_series(x)


def atan2(y, x):
    p = pi(getcontext().prec)
    if y.is_nan() or x.is_nan():
        return D('NaN')
    if y.is_zero():
        return p.copy_sign(y) if x.is_signed() else y
    if x.is_zero():
        return (p / 2).copy_sign(y)
    if y.is_infinite():
        angle = p / 4 if x.is_infinite() else p / 2
        if x.is_infinite() and x.is_signed():
            angle = 3 * p / 4
        return angle.copy_sign(y)
    if x.is_infinite():
        return (p if x.is_signed() else D(0)).copy_sign(y)
    angle = atan(abs(y / x))
    if x.is_signed():
        angle = p - angle
    return angle.copy_sign(y)


def sincos(x):
    if x.is_zero():
        return x, D(1)
    half_pi = pi(getcontext().prec) / 2
    quadrant = (x / half_pi).to_integral_value(rounding=ROUND_HALF_EVEN)
    r = x - quadrant * half_pi
    sin_term = sine = r
    cos_term = cosine = D(1)
    square = -r * r
    n = 1
    while True:
        sin_term = sin_term * square / ((2 * n) * (2 * n + 1))
        cos_term = cos_term * square / ((2 * n - 1) * (2 * n))
        s, c = sine + sin_term, cosine + cos_term
        if s == sine and c == cosine:
            break
        sine, cosine = s, c
        n += 1
    return [(sine, cosine), (cosine, -sine), (-sine, -cosine),
            (-cosine, sine)][int(quadrant) % 4]


def exponential(x):
    # These bounds are outside the finite/nonzero binary64 result range.
    if x > 1500:
        return D('Infinity')
    if x < -1500:
        return D(0)
    return x.exp()


def reference(name, a, b=0):
    """Return output words (two for modf/sincos, one otherwise)."""
    x, y = value(a), value(b)
    ax, ay = a & MASK, b & MASK
    nx, ny = ax > INF, ay > INF
    if name == 'isnan':
        return (int(nx),)
    if name == 'isinf':
        return (int(ax == INF),)
    if name == 'isfinite':
        return (int(ax < INF),)
    if name == 'isnormal':
        return (int(0x0010000000000000 <= ax < INF),)
    if name == 'fabs':
        return (ax,)
    if name == 'copysign':
        return (ax | (b & SIGN),)
    if name in ('fmin', 'fmax'):
        if nx:
            return (NAN if ny else b,)
        if ny:
            return (a,)
        if ax == 0 and ay == 0:
            return ((a | b) if name == 'fmin' else (a & b),)
        return (a if (x < y if name == 'fmin' else x > y) else b,)
    if name == 'pow':
        if y == 0 or x == 1:
            return (bits(1.0),)
        if nx or ny:
            return (NAN,)
        if ay == INF:
            if abs(x) == 1:
                return (bits(1.0),)
            return (INF if (abs(x) > 1) == (y > 0) else 0,)
        odd = y.is_integer() and abs(y) < 2**53 and int(y) % 2 != 0
        sign = SIGN if (a & SIGN) and odd else 0
        if ax == 0:
            return (sign | (INF if y < 0 else 0),)
        if ax == INF:
            return (sign | (INF if y > 0 else 0),)
        if x < 0 and not y.is_integer():
            return (NAN,)
        if y.is_integer() and abs(y) <= 4097:
            return (rounded(Fraction(x) ** int(y)),)
        z = exponential(D.from_float(y) * D.from_float(abs(x)).ln())
        return (rounded(z) | sign,)
    if nx or (name in ('atan2', 'fmod', 'log_base') and ny):
        return (NAN, NAN) if name in ('modf', 'sincos') else (NAN,)
    if name in ('ceil', 'floor', 'trunc', 'modf'):
        if ax == INF or ax == 0:
            return (a & SIGN, a) if name == 'modf' else (a,)
        exact = Fraction(x)
        integer = int(exact)
        if name == 'ceil':
            integer = -((-exact.numerator) // exact.denominator)
        if name == 'floor':
            integer = exact.numerator // exact.denominator
        integral = bits(float(integer)) if integer else a & SIGN
        if name == 'modf':
            fraction = exact - integer
            return (rounded(fraction) if fraction else a & SIGN, integral)
        return (integral,)
    if name == 'fmod':
        if ay == 0 or ax == INF:
            return (NAN,)
        if ay == INF or ax == 0:
            return (a,)
        q = int(Fraction(x) / Fraction(y))
        remainder = Fraction(x) - q * Fraction(y)
        return (rounded(remainder) if remainder else a & SIGN,)

    dx, dy = D.from_float(x), D.from_float(y)
    if name == 'sqrt':
        return (rounded(dx.sqrt()) if x >= 0 else NAN,)
    if name == 'cbrt':
        if ax == 0 or ax == INF:
            return (a,)
        return (rounded(exponential(abs(dx).ln() / 3)) | (a & SIGN),)
    if name in ('exp', 'exp2', 'exp10'):
        # Resolve exact halfway/subnormal cases using rationals, rather than
        # allowing the last Decimal rounding error to decide a binary64 tie.
        if name == 'exp2' and ax < INF and x.is_integer() and abs(x) <= 1100:
            return (rounded(Fraction(2) ** int(x)),)
        if name == 'exp10' and ax < INF and x.is_integer() and abs(x) <= 500:
            return (rounded(Fraction(10) ** int(x)),)
        factor = {'exp': D(1), 'exp2': D(2).ln(), 'exp10': D(10).ln()}[name]
        return (rounded(exponential(dx * factor)),)
    if name in ('log', 'log2', 'log10', 'log_base'):
        logarithm = dx.ln()
        divisor = {'log': D(1), 'log2': D(2).ln(), 'log10': D(10).ln()}
        denominator = dy.ln() if name == 'log_base' else divisor[name]
        return (rounded(logarithm / denominator),)
    if name in ('sin', 'cos', 'tan', 'sincos'):
        if ax == INF:
            return (NAN, NAN) if name == 'sincos' else (NAN,)
        s, c = sincos(dx)
        if name == 'sincos':
            return rounded(s), rounded(c)
        return (rounded({'sin': s, 'cos': c, 'tan': s / c}[name]),)
    if name == 'atan':
        return (rounded(atan(dx)),)
    if name == 'atan2':
        return (rounded(atan2(dx, dy)),)  # arguments are (y, x)
    if name in ('asin', 'acos'):
        if abs(dx) > 1:
            return (NAN,)
        root = (1 - dx * dx).sqrt()
        return (rounded(atan2(dx, root) if name == 'asin' else atan2(root, dx)),)
    raise ValueError(name)
