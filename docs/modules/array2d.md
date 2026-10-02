---
icon: package
label: array2d
---

# array2d

A rectangular grid for tile maps, images, cellular simulations, and other 2D
data. Coordinates are **column first, row second**: `grid[x, y]`.

## Create and address a grid

`array2d(width, height, default=None)` requires positive dimensions.
A callable default receives a `vmath.vec2i` position for each cell.

```python
from array2d import array2d
from vmath import vec2i

grid = array2d(3, 2, default=0)
grid[1, 0] = 7
assert grid.shape == vec2i(3, 2)
assert grid.numel == 6
assert grid.tolist() == [[0, 7, 0], [0, 0, 0]]
assert grid.get(99, 0, -1) == -1

cells = array2d(2, 2, default=lambda pos: [])
cells[0, 0].append('player')
assert cells[1, 0] == []
```

A non-callable default is copied by reference into every cell. Use a factory,
as above, for independent mutable objects.

`array2d.fromlist(rows)` accepts a nonempty rectangular list of rows.
`width`/`n_cols`, `height`/`n_rows`, `shape`, and `numel` describe
the dimensions. Integer cell coordinates must be nonnegative and in bounds.
Use `is_valid(x, y)` or `get(x, y, default)` for boundary-aware access.

## Views, copies, and masks

Slicing creates a view sharing the original grid. Call `copy()` for a
separate grid; contained Python objects are still shallow-copied.

```python
from array2d import array2d

grid = array2d(3, 2, default=0)
view = grid[1:3, :]
view[:, :] = 5
assert grid.tolist() == [[0, 5, 5], [0, 5, 5]]

mask = grid == 5
assert mask.any()
assert not mask.all()
assert grid[mask] == [5, 5, 5, 5]
grid[mask] = 9
assert grid.count(9) == 4
```

Comparisons produce boolean grids, not a single boolean.
Use `(a == b).all()` for whole-grid equality.
Arithmetic and bitwise operations accept scalars or compatible grids.

## Transform and analyze

| Method | Use |
| --- | --- |
| `map(function)` | Return a new grid of transformed values. |
| `apply(function)` | Transform cells in place. |
| `zip_with(other, function)` | Combine two grids cell by cell. |
| `index(value)`, `count(value)` | Find the first position or count matching cells. |
| `get_bounding_rect(value)` | `(x, y, width, height)`; raise `ValueError` if absent. |
| `count_neighbors(value, neighborhood)` | Count matching neighbors of each cell. |
| `get_connected_components(value, neighborhood)` | Return a label grid and component count; label 0 is unvisited. |
| `convolve(kernel, padding)` | Integer-grid convolution with a padding value. |
| `render()`, `render_with_color(fg, bg)` | Produce text for display. |

Neighborhood names are `'Moore'` (eight neighbors) and `'von Neumann'`
(four neighbors). Iteration yields `(position, value)` pairs.

## Chunked grids

`chunked_array2d(chunk_size, default=None, auto_add_chunk=True)` stores
chunks addressed with `vec2i` world coordinates. It provides chunk creation,
removal, movement, context data, and rectangular/chunk views.
Use `world_to_chunk(position)` to obtain the chunk coordinate and local cell
coordinate.

The [API declarations](https://github.com/pocketpy/pocketpy/blob/main/include/typings/array2d.pyi)
list the full grid, view, and chunk interfaces.
`array2d` is not supported by [pickle](pickle.md); `tolist()` can provide a
serializable representation when the cell values support it.
