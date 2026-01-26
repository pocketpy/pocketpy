assert __package__ == 'test.a.g'
assert __name__ == 'test.a.g'

import os
os.environ['STDOUT'] += 'test.a.g init!!\n'


