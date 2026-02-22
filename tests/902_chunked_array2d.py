import array2d
from vmath import vec2i


default = 0
a = array2d.chunked_array2d(16, default, auto_add_chunk=False)
assert a.chunk_size == 16

a.add_chunk(vec2i(1, 1), 5.0)
a[vec2i(16, 16)] = 16
a[vec2i(17, 16)] = 15
assert a[vec2i(16, 16)] == 16
assert a[vec2i(17, 16)] == 15
assert a[vec2i(17, 20)] == default

a1, _ = a.world_to_chunk(vec2i(16, 16))

assert a.get_context(vec2i(1,1)) == 5.0
assert a.remove_chunk(a1)
