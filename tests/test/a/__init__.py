assert __package__ == 'test.a'
assert __name__ == 'test.a'

import os
os.environ['STDOUT'] += 'test.a init!!\n'