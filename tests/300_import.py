try:
    import os
except ImportError:
    exit(0)

import sys
is_pyc = sys.argv[0].endswith('.pyc')

if is_pyc:
    os.chdir('tmp/tests')
else:
    os.chdir('tests')

assert os.getcwd().endswith('tests')

import test1

assert test1.add(1, 2) == 13

from test2.a.g import get_value, A
assert get_value() == '123'
assert (A.__module__ == 'test2.a.g'), A.__module__

import test2
assert test2.a.g.get_value() == '123'

from test2.utils import get_value_2
assert get_value_2() == '123'

from test3.a.b import value
assert value == 1

from test2.utils import r
assert r.__name__ == 'r'
assert r.__package__ == 'test2.utils'
assert r.__path__ == 'test2.utils.r'

def f():
    import math as m
    assert m.pi > 3

    from test3.a.b import value
    assert value == 1

f()

from math import *
assert pi > 3

from math import (pi, pow, sin, cos)

from math import (
    pi,
    pow,
    sin,
    cos
)

assert __import__('math').pi > 3

# test reload (dummy)
if not is_pyc:
    import importlib
    importlib.reload(test2.a)

    
