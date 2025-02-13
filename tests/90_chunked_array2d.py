from array2d import chunked_array2d
from linalg import vec2i

a = chunked_array2d(4, default=0)

a[vec2i.ONE] = 1

print(a.view().render())
