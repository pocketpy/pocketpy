try:
    import os
except ImportError:
    exit(0)

import importlib

os.chdir('tests')
assert os.getcwd().endswith('tests')

# test
os.environ['TEST_RELOAD_VALUE'] = '123'
os.environ['SET_X'] = '1'
os.environ['SET_Y'] = '0'

from testreload import MyClass, a

objid = id(MyClass)
funcid = id(MyClass.some_func)
getxyid = id(MyClass.get_xy)

assert MyClass.value == '123'
assert MyClass.get_xy() == (1, 0)

inst = MyClass()
assert inst.some_func() == '123'

# reload
os.environ['TEST_RELOAD_VALUE'] = '456'
os.environ['SET_X'] = '0'
os.environ['SET_Y'] = '1'

importlib.reload(a)

assert id(MyClass) == objid
assert id(MyClass.some_func) != funcid
assert id(MyClass.get_xy) != getxyid

assert MyClass.value == '456'
assert inst.some_func() == '456'
assert (MyClass.get_xy() == (1, 1)), MyClass.get_xy()


