import gc
from pkpy import setup_gc_debug_callback

setup_gc_debug_callback(print)

gc.collect()
def create_garbage():
    a = [(1,2) for i in range(20000)]
    return a

create_garbage()
gc.collect()

create_garbage()
create_garbage()
create_garbage()