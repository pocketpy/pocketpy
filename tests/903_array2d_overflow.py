from array2d import array2d
import array2d as array2d_module
from vmath import vec2i


def expect_value_error(fn):
    try:
        fn()
    except ValueError:
        return
    raise AssertionError("expected ValueError")


# Dimensions that fit in int individually but whose product exceeds INT_MAX
# must be rejected before the allocation size is calculated.
expect_value_error(lambda: array2d(65536, 65537))

# Python integers outside the C int range must not be truncated during
# conversion to the dimensions stored by array2d.
expect_value_error(lambda: array2d(2**32 + 5, 1))

# Normal construction and indexing must remain unaffected.
a = array2d(2, 3, default=0)
a[1, 2] = 42
assert a.width == 2
assert a.height == 3
assert a.numel == 6
assert a[1, 2] == 42

# Views must reject dimensions whose element count exceeds INT_MAX too.
# Otherwise operations such as copy() or map() could reach an invalid allocation.
view_source = array2d_module.chunked_array2d(4)
expect_value_error(lambda: view_source.view_rect(vec2i(0, 0), 65536, 65537))

