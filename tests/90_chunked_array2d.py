import array2d
from vmath import vec2i


def on_builder(a:vec2i):
    return str(a)
    pass

default = 0
a = array2d.chunked_array2d(16, default,on_builder)
assert a.chunk_size == 16

a[vec2i(16, 16)] = 16
a[vec2i(15, 16)] = 15
assert a[vec2i(16, 16)] == 16
assert a[vec2i(15, 16)] == 15
assert a[vec2i(16, 15)] == default

a1,a2=a.world_to_chunk(vec2i(15,16))

assert a.remove_chunk(a1)== True
assert a[vec2i(15, 16)] == default

assert a.get_context(vec2i(1,1))==on_builder(vec2i(1,1))

assert a.view().tolist()==[
    [16 if i==0 and j==0 else 0 for j in range(16)] for i in range(16)
]
assert a.view_rect(vec2i(15,15),4,4).tolist()==[
    [0,0,0,0],
    [0,16,0,0],
    [0,0,0,0],
    [0,0,0,0]
]
a[vec2i(15, 16)] = 15
assert a.view_chunk(a1).tolist()==[
    [15 if i==0 and j==15 else 0 for j in range(16)] for i in range(16)
]
a.clear()

assert a[vec2i(16, 16)] == default
assert a[vec2i(15, 16)] == default
assert a[vec2i(16, 15)] == default

from typing import Any

a = array2d.chunked_array2d[int, Any](4, default=0, context_builder=lambda x: 1)
assert a.chunk_size == 4

assert a.add_chunk(vec2i(0, 1)) == 1
assert a.get_context(vec2i(0, 1)) == 1

assert a.move_chunk(vec2i(2, 1), vec2i(1, 1)) == False
assert a.move_chunk(vec2i(0, 1), vec2i(1, 1)) == True

assert a.get_context(vec2i(1, 1)) == 1
assert a.get_context(vec2i(0, 1)) == None

b = a.copy()
assert a is not b
assert a.chunk_size == b.chunk_size
assert a.default == b.default
assert a.context_builder == b.context_builder
assert (a.view() == b.view()).all()

for pos, ctx in a:
    assert b.get_context(pos) == ctx
