import gc

gc.setup_debug_callback(print)

gc.collect()
def create_garbage():
    a = [(1,2) for i in range(20000)]
    return a

create_garbage()
gc.collect()

create_garbage()
create_garbage()
c = create_garbage()

assert gc.is_tracked(c) == True
gc.untrack(c)
assert gc.is_tracked(c) == False
gc.track(c)
assert gc.is_tracked(c) == True

# an error in the debug callback is printed; the collection continues
calls = []
def bad_callback(phase, info):
    calls.append(phase)
    raise ValueError(phase)

gc.setup_debug_callback(bad_callback)
gc.collect()
gc.setup_debug_callback(None)
assert calls == ['start', 'stop'], calls
