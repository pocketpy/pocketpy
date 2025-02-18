from array2d import array2d
from linalg import vec2i

def exit_on_error():
    raise KeyboardInterrupt

# test error args for __init__
try:
    a = array2d(0, 0)
    exit_on_error()
except ValueError:
    pass

# test callable constructor
a = array2d[int](2, 4, lambda pos: (pos.x, pos.y))

assert a.width == a.n_cols == 2
assert a.height == a.n_rows == 4
assert a.shape == vec2i(2, 4)
assert a.numel == 8
assert a.tolist() == [
    [(0, 0), (1, 0)],
    [(0, 1), (1, 1)],
    [(0, 2), (1, 2)],
    [(0, 3), (1, 3)]]

assert a[0, :].tolist() == [[(0, 0)], [(0, 1)], [(0, 2)], [(0, 3)]]
assert a[1, :].tolist() == [[(1, 0)], [(1, 1)], [(1, 2)], [(1, 3)]]
assert a[:, 0].tolist() == [[(0, 0), (1, 0)]]
assert a[:, -1].tolist() == [[(0, 3), (1, 3)]]

# test is_valid
assert a.is_valid(0, 0) and a.is_valid(vec2i(0, 0))
assert a.is_valid(1, 3) and a.is_valid(vec2i(1, 3))
assert not a.is_valid(2, 0) and not a.is_valid(vec2i(2, 0))
assert not a.is_valid(0, 4) and not a.is_valid(vec2i(0, 4))
assert not a.is_valid(-1, 0) and not a.is_valid(vec2i(-1, 0))
assert not a.is_valid(0, -1) and not a.is_valid(vec2i(0, -1))

# test get
assert a.get(0, 0, -1) == (0, 0)
assert a.get(1, 3) == (1, 3)
assert a.get(2, 0) is None
assert a.get(0, 4, 'S') == 'S'

# test __getitem__
assert a[0, 0] == (0, 0)
assert a[1, 3] == (1, 3)
try:
    a[2, 0]
    exit_on_error()
except IndexError:
    pass

# test __setitem__
a = array2d[int](2, 4, default=0)
a[0, 0] = 5
assert a[0, 0] == 5
a[1, 3] = 6
assert a[1, 3] == 6
try:
    a[0, -1] = 7
    exit_on_error()
except IndexError:
    pass

# test tolist
a_list = [[5, 0], [0, 0], [0, 0], [0, 6]]
assert a_list == a.tolist()

# test __eq__
x = array2d(2, 4, default=0)
b = array2d(2, 4, default=0)
assert (x == b).all()

b[0, 0] = 1
assert (x != b).any()

# test __repr__
assert repr(a) == f'array2d(2, 4)'

# test map
c = a.map(lambda x: x + 1)
assert c.tolist() == [[6, 1], [1, 1], [1, 1], [1, 7]]
assert a.tolist() == [[5, 0], [0, 0], [0, 0], [0, 6]]
assert c.width == c.n_cols == 2
assert c.height == c.n_rows == 4
assert c.numel == 8

# test copy
d = c.copy()
assert (d == c).all() and d is not c

# test fill_
d[:, :] = -3    # d.fill_(-3)
assert (d == array2d(2, 4, default=-3)).all()

# test apply
d.apply(lambda x: x + 3)
assert (d == array2d(2, 4, default=0)).all()

# test copy_
a[:, :] = d
assert (a == d).all() and a is not d
x = array2d(2, 4, default=0)
x[:, :] = d
assert (x == d).all() and x is not d

# test alive_neighbors
a = array2d[int](3, 3, default=0)
a[1, 1] = 1
"""     Moore    von Neumann
0 0 0   1 1 1    0 1 0
0 1 0   1 0 1    1 0 1
0 0 0   1 1 1    0 1 0
"""
moore_result = array2d(3, 3, default=1)
moore_result[1, 1] = 0

von_neumann_result = array2d(3, 3, default=0)
von_neumann_result[0, 1] = von_neumann_result[1, 0] = von_neumann_result[1, 2] = von_neumann_result[2, 1] = 1

_0 = a.count_neighbors(1, 'Moore')
assert _0 == moore_result
_1 = a.count_neighbors(1, 'von Neumann')
assert _1 == von_neumann_result

MOORE_KERNEL = array2d[int].fromlist([[1, 1, 1], [1, 0, 1], [1, 1, 1]])
VON_NEUMANN_KERNEL = array2d.fromlist([[0, 1, 0], [1, 0, 1], [0, 1, 0]])
moore_conv_result = a.convolve(MOORE_KERNEL, 0)
assert (moore_conv_result == moore_result).all()
von_neumann_conv_result = a.convolve(VON_NEUMANN_KERNEL, 0)
assert (von_neumann_conv_result == von_neumann_result).all()

# test slice get
a = array2d(5, 5, default=0)
b = array2d(3, 2, default=1)

assert a[1:4, 1:4] == array2d(3, 3, default=0)
assert a[1:4, 1:3] == array2d(3, 2, default=0)
assert (a[1:4, 1:3] != b).any()
a[1:4, 1:3] = b
assert (a[1:4, 1:3] == b).all()
"""
0 0 0 0 0
0 1 1 1 0
0 1 1 1 0
0 0 0 0 0
0 0 0 0 0
"""
assert a.count(1) == 3*2

assert a.get_bounding_rect(1) == (1, 1, 3, 2)
assert a.get_bounding_rect(0) == (0, 0, 5, 5)

try:
    a.get_bounding_rect(2)
    exit_on_error()
except ValueError:
    pass

a = array2d(3, 2, default='?')
# int/float/str/bool/None

for value in [0, 0.0, '0', False, None]:
    a[0:2, 0:1] = value
    assert a[2, 1] == '?'
    assert a[0, 0] == value

a[:, :] = 3
assert a == array2d(3, 2, default=3)

try:
    a[:, :] = array2d(1, 1)
    exit_on_error()
except ValueError:
    pass

# test __iter__
a = array2d(3, 4, default=1)
for xy, val in a:
    assert a[xy] == x

# test convolve
a = array2d[int].fromlist([[1, 0, 2, 4, 0], [3, 1, 0, 5, 1]])
"""
1 0 2 4 0
3 1 0 5 1
"""
assert a.tolist() == [[1, 0, 2, 4, 0], [3, 1, 0, 5, 1]]

kernel = array2d[int](3, 3, default=1)
res = a.convolve(kernel, -1)
"""
0 4 9 9 5
0 4 9 9 5
"""
assert res.tolist() == [[0, 4, 9, 9, 5], [0, 4, 9, 9, 5]]

mask = res == 9
assert mask.tolist() == [
    [False, False, True, True, False],
    [False, False, True, True, False]
    ]
assert res[mask] == [9, 9, 9, 9]

mask = res != 9
assert mask.tolist() == [
    [True, True, False, False, True],
    [True, True, False, False, True]
    ]
assert res[mask] == [0, 4, 5, 0, 4, 5]
res[mask] = -1
assert res.tolist() == [[-1, -1, 9, 9, -1], [-1, -1, 9, 9, -1]]

# test get_connected_components
a = array2d[int].fromlist([
    [1, 1, 0, 1],
    [0, 2, 2, 1],
    [0, 1, 1, 1],
    [1, 0, 0, 0],
])
vis, cnt = a.get_connected_components(1, 'von Neumann')
assert vis == [
    [1, 1, 0, 2],
    [0, 0, 0, 2],
    [0, 2, 2, 2],
    [3, 0, 0, 0]
    ]
assert cnt == 3
vis, cnt = a.get_connected_components(1, 'Moore')
assert vis == [
    [1, 1, 0, 2],
    [0, 0, 0, 2],
    [0, 2, 2, 2],
    [2, 0, 0, 0]
    ]
assert cnt == 2
vis, cnt = a.get_connected_components(2, 'von Neumann')
assert cnt == 1
vis, cnt = a.get_connected_components(0, 'Moore')
assert cnt == 2

# test zip_with
a = array2d[int].fromlist([[1, 2], [3, 4]])
b = array2d[int].fromlist([[5, 6], [7, 8]])
c = a.zip_with(b, lambda x, y: x + y)
assert c.tolist() == [[6, 8], [10, 12]]

# test magic op
a = array2d[int].fromlist([[1, 2], [3, 4]])
assert (a <= 2).tolist() == [[True, True], [False, False]]
assert (a < 2).tolist() == [[True, False], [False, False]]
assert (a >= 2).tolist() == [[False, True], [True, True]]
assert (a > 2).tolist() == [[False, False], [True, True]]
assert (a == 2).tolist() == [[False, True], [False, False]]
assert (a != 2).tolist() == [[True, False], [True, True]]
assert (a + 1).tolist() == [[2, 3], [4, 5]]
assert (a - 1).tolist() == [[0, 1], [2, 3]]
assert (a * 2).tolist() == [[2, 4], [6, 8]]
assert (a / 1).tolist() == [[1.0, 2.0], [3.0, 4.0]]
assert (a // 2).tolist() == [[0, 1], [1, 2]]
assert (a % 2).tolist() == [[1, 0], [1, 0]]
assert (a ** 2).tolist() == [[1, 4], [9, 16]]

a = array2d[bool].fromlist([[True, False], [False, True]])
assert (a & True).tolist() == [[True, False], [False, True]]
assert (a | True).tolist() == [[True, True], [True, True]]
assert (a ^ True).tolist() == [[False, True], [True, False]]

b = array2d[bool].fromlist([[True, True], [False, False]])
assert (a & b).tolist() == [[True, False], [False, False]]
assert (a | b).tolist() == [[True, True], [False, True]]
assert (a ^ b).tolist() == [[False, True], [False, True]]
assert (~a).tolist() == [[False, True], [True, False]]
assert (~b).tolist() == [[False, False], [True, True]]

# stackoverflow bug due to recursive mark-and-sweep
# class Cell:
#     neighbors: list['Cell']

# cells: array2d[Cell] = array2d(192, 108, default=Cell)
# OutOfBounds = Cell()
# for x, y, cell in cells:
#     cell.neighbors = [
#         cells.get(x-1, y-1, OutOfBounds),
#         cells.get(x  , y-1, OutOfBounds),
#         cells.get(x+1, y-1, OutOfBounds),
#         cells.get(x-1, y  , OutOfBounds),
#         cells.get(x+1, y  , OutOfBounds),
#         cells.get(x  , y+1, OutOfBounds),
#         cells.get(x+1, y+1, OutOfBounds),
#     ]

# import gc
# gc.collect()
