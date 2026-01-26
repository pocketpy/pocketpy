assert __package__ == 'test.a.g'
assert __name__ == 'test.a.g.q'

import os
os.environ['STDOUT'] += 'test.a.g.q init!!\n'