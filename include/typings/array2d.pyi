from typing import Callable, Any, Generic, TypeVar, Literal, overload, Iterator
from linalg import vec2i

T = TypeVar('T')

Neighborhood = Literal['Moore', 'von Neumann']

class array2d(Generic[T]):
    @property
    def n_cols(self) -> int: ...
    @property
    def n_rows(self) -> int: ...
    @property
    def width(self) -> int: ...
    @property
    def height(self) -> int: ...
    @property
    def numel(self) -> int: ...

    def __new__(cls, n_cols: int, n_rows: int, default=None): ...
    def __len__(self) -> int: ...
    def __repr__(self) -> str: ...
    def __iter__(self) -> Iterator[tuple[int, int, T]]: ...

    @overload
    def is_valid(self, col: int, row: int) -> bool: ...
    @overload
    def is_valid(self, pos: vec2i) -> bool: ...

    def get(self, col: int, row: int, default=None) -> T | None:
        """Returns the value at the given position or the default value if out of bounds."""
    def unsafe_get(self, col: int, row: int) -> T:
        """Returns the value at the given position without bounds checking."""
    def unsafe_set(self, col: int, row: int, value: T):
        """Sets the value at the given position without bounds checking."""

    @overload
    def __getitem__(self, index: tuple[int, int]) -> T: ...
    @overload
    def __getitem__(self, index: vec2i) -> T: ...
    @overload
    def __getitem__(self, index: tuple[slice, slice]) -> 'array2d[T]': ...
    @overload
    def __setitem__(self, index: tuple[int, int], value: T): ...
    @overload
    def __setitem__(self, index: vec2i, value: T): ...
    @overload
    def __setitem__(self, index: tuple[slice, slice], value: int | float | str | bool | None | 'array2d[T]'): ...

    def map(self, f: Callable[[T], Any]) -> 'array2d': ...
    def copy(self) -> 'array2d[T]': ...

    def fill_(self, value: T) -> None: ...
    def apply_(self, f: Callable[[T], T]) -> None: ...
    def copy_(self, other: 'array2d[T] | list[T]') -> None: ...

    def tolist(self) -> list[list[T]]: ...
    def render(self) -> str: ...

    # algorithms
    def count(self, value: T) -> int:
        """Counts the number of cells with the given value."""

    def count_neighbors(self, value: T, neighborhood: Neighborhood) -> 'array2d[int]':
        """Counts the number of neighbors with the given value for each cell."""

    def find_bounding_rect(self, value: T) -> tuple[int, int, int, int]:
        """Finds the bounding rectangle of the given value.
        
        Returns a tuple `(x, y, width, height)` or raise `ValueError` if the value is not found.
        """

    def find_one(self, condition: Callable[[T], bool]) -> vec2i:
        """Finds the position of the first cell that satisfies the condition.
        
        Returns a `vec2i` or raise `ValueError` if no cell satisfies the condition.
        """

    def convolve(self: array2d[int], kernel: 'array2d[int]', padding: int) -> 'array2d[int]':
        """Convolves the array with the given kernel."""
