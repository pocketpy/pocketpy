try:
    import os
except ImportError:
    exit(0)

os.chdir('tests')

import test1

assert test1.add(1, 2) == 13

from test2.a.g import get_value
assert get_value() == '123'

import test2
assert test2.a.g.get_value() == '123'

from test2.utils import get_value_2
assert get_value_2() == '123'

from test3.a.b import value
assert value == 1

def f():
    import math as m
    assert m.pi > 3

    from test3.a.b import value
    assert value == 1

f()

from math import *
assert pi > 3
