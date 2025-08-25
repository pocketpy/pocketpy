a=[]
import gc
gc.collect()

# a.append(a)
print(list(globals().items()))
del a
print(list(globals().items()))
gc.collect()
