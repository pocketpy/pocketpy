import gc

def create_garbage():
    a = [(1,2) for i in range(10000)]
    return a

create_garbage()
gc.collect()

create_garbage()
create_garbage()
create_garbage()