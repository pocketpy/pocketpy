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

os.environ['STDOUT'] = ''

import test.a.g.q

assert os.environ['STDOUT'] == 'test init!!\ntest.a init!!\ntest.a.g init!!\ntest.a.g.q init!!\n'

import test

assert test.__package__ == 'test'
assert test.__name__ == 'test'
