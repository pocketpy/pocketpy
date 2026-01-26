assert __package__ == 'test'
assert __name__ == 'test'

import os
os.environ['STDOUT'] += 'test init!!\n'