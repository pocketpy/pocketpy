"""Python bindings for the new per-function dmath corpus.

Run from the repository root: main tests/930_dmath.py [function]
C-only functions (exp2, exp10, sincos, isnormal, fmin, fmax) have their own
groups in test_dmath. The same frozen cases are used here, without decimal
parsing or signed-integer overflow during test setup.
"""
import math
import stdc
import sys


_byte_order_probe = stdc.UInt(1)
_little_endian = stdc.read_bytes(stdc.addressof(_byte_order_probe), 4)[0] == 1
_hex_digits = '0123456789abcdef'


def from_bits(text):
    raw = bytes([int(text[i:i + 2], 16) for i in range(0, 16, 2)])
    if _little_endian:
        raw = raw[::-1]
    cell = stdc.Double(0.0)
    stdc.memcpy(stdc.addressof(cell), raw, 8)
    return cell.value


def to_bits(x):
    cell = stdc.Double(x)
    raw = stdc.read_bytes(stdc.addressof(cell), 8)
    if _little_endian:
        raw = raw[::-1]
    return ''.join([_hex_digits[b >> 4] + _hex_digits[b & 15] for b in raw])


def check_result(actual, expected, context):
    if expected == 'nan':
        assert math.isnan(actual), context
    elif expected == '+inf' or expected == '-inf':
        assert math.isinf(actual), context
        assert (actual > 0.0) == (expected == '+inf'), context
    else:
        assert to_bits(actual) == expected, (context, to_bits(actual), expected)


def raises_type_error(function, *args):
    try:
        function(*args)
    except TypeError:
        return
    raise AssertionError('expected TypeError')


def check_cases(name, function, arity=1, kind='float'):
    with open('tests/dmath/cases/' + name + '.txt', 'rt') as handle:
        lines = handle.read().split('\n')
    count = 0
    for line in lines:
        if not line or line.startswith('#'):
            continue
        fields = line.split()
        label, x_word, y_word, expected, auxiliary = fields[:5]
        assert expected != 'PENDING', (name, label, 'unreviewed case')
        x, y = from_bits(x_word), from_bits(y_word)
        result = function(x) if arity == 1 else function(x, y)
        context = (name, label, x_word, y_word)
        if kind == 'bool':
            assert type(result) is bool, context
            assert result == bool(int(expected, 16)), context
        elif kind == 'rounding':
            want = from_bits(expected) if len(expected) == 16 else math.nan
            if want >= from_bits('c3e0000000000000') and want < from_bits('43e0000000000000'):
                assert type(result) is int, context
                assert result == int(want), context
            else:
                assert type(result) is float, context
                check_result(result, expected, context)
        elif kind == 'pair':
            assert type(result) is tuple and len(result) == 2, context
            check_result(result[0], expected, context)
            check_result(result[1], auxiliary, context)
        else:
            assert type(result) is float, context
            check_result(result, expected, context)
        count += 1
    assert count > 0, name
    raises_type_error(function)
    raises_type_error(function, 'not a number')
    raises_type_error(function, 1.0, 2.0, 3.0)
    if arity == 2:
        raises_type_error(function, 1.0, 'not a number')
    print('PASS math.' + name + ': ' + str(count) + ' named cases')


# Classification.
def test_isfinite():
    check_cases('isfinite', math.isfinite, kind='bool')


def test_isinf():
    check_cases('isinf', math.isinf, kind='bool')


def test_isnan():
    check_cases('isnan', math.isnan, kind='bool')


# Sign operations require exact finite results; NaNs are checked by classification.
def test_fabs():
    check_cases('fabs', math.fabs)


def test_copysign():
    check_cases('copysign', math.copysign, 2)


# Integral return types and exact int64 arguments belong to the binding layer.
def check_integer_arguments(function):
    for x in [9007199254741027, -9007199254741027, 9223372036854775793,
              -9223372036854775807 - 1]:
        assert type(function(x)) is int
        assert function(x) == x


def test_ceil():
    check_cases('ceil', math.ceil, kind='rounding')
    check_integer_arguments(math.ceil)


def test_floor():
    check_cases('floor', math.floor, kind='rounding')
    check_integer_arguments(math.floor)


def test_trunc():
    check_cases('trunc', math.trunc, kind='rounding')
    check_integer_arguments(math.trunc)


def test_modf():
    check_cases('modf', math.modf, kind='pair')
    assert math.modf(83) == (0.0, 83.0)


def test_fmod():
    check_cases('fmod', math.fmod, 2)


# Roots.
def test_sqrt():
    check_cases('sqrt', math.sqrt)


def test_cbrt():
    check_cases('cbrt', math.cbrt)


# Exponential and logarithmic functions.
def test_exp():
    check_cases('exp', math.exp)


def test_pow():
    check_cases('pow', math.pow, 2)


def test_log():
    check_cases('log', math.log)


def test_log2():
    check_cases('log2', math.log2)


def test_log10():
    check_cases('log10', math.log10)


def test_log_base():
    check_cases('log_base', math.log, 2)


# Trigonometry.
def test_sin():
    check_cases('sin', math.sin)


def test_cos():
    check_cases('cos', math.cos)


def test_tan():
    check_cases('tan', math.tan)


# Inverse trigonometry.
def test_asin():
    check_cases('asin', math.asin)


def test_acos():
    check_cases('acos', math.acos)


def test_atan():
    check_cases('atan', math.atan)


def test_atan2():
    check_cases('atan2', math.atan2, 2)


groups = [
    ('classification', [test_isfinite, test_isinf, test_isnan]),
    ('sign', [test_fabs, test_copysign]),
    ('rounding_and_remainder', [test_ceil, test_floor, test_trunc, test_modf, test_fmod]),
    ('roots', [test_sqrt, test_cbrt]),
    ('exponentials', [test_exp, test_pow]),
    ('logarithms', [test_log, test_log2, test_log10, test_log_base]),
    ('trigonometry', [test_sin, test_cos, test_tan]),
    ('inverse_trigonometry', [test_asin, test_acos, test_atan, test_atan2]),
]
selected = sys.argv[1] if len(sys.argv) > 1 else None
ran = 0
for category, functions in groups:
    for function in functions:
        if selected is None or function.__name__ == 'test_' + selected:
            function()
            ran += 1
assert ran > 0, selected
