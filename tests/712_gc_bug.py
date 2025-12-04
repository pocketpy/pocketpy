# a=[]
# import gc
# gc.collect()

# # a.append(a)
# print(globals().items())
# del a
# print(list(globals().items()))
# print(globals()['gc'])
# gc.collect()

d = object()
d.__name__ = '__main__'
d.__package__ = ''
d.__path__ = '__main__'
d.a = []
d.gc = 1

assert d.gc == 1
del d.a

assert not hasattr(d, 'a')
assert d.gc == 1

# [0, 1, 6, 7, 4, 5, 2, 3]

# 0 __name__      [0]
# 1 __package__   [1]
# 2 nil
# 3 nil
# 4 gc            [4]
# 5 nil
# 6 __path__      [2]
# 7 a             [3]

import gc
gc.collect()

a = []
del a
assert gc.collect() == 1

a = []
a.append(a)
del a
assert gc.collect() == 1
