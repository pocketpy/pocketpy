from typing import Callable, Any, Generic, TypeVar

T = TypeVar('T')

class array2d(Generic[T]):
    data: list[T]       # not available in native module

    def __init__(self, n_cols: int, n_rows: int, default=None):
        self.n_cols = n_cols
        self.n_rows = n_rows
        if callable(default):
            self.data = [default() for _ in range(n_cols * n_rows)]
        else:
            self.data = [default] * n_cols * n_rows
    
    @property
    def width(self) -> int:
        return self.n_cols
    
    @property
    def height(self) -> int:
        return self.n_rows
    
    @property
    def numel(self) -> int:
        return self.n_cols * self.n_rows

    def is_valid(self, col: int, row: int) -> bool:
        return 0 <= col < self.n_cols and 0 <= row < self.n_rows

    def get(self, col: int, row: int, default=None):
        if not self.is_valid(col, row):
            return default
        return self.data[row * self.n_cols + col]

    def __getitem__(self, index: tuple[int, int]):
        col, row = index
        if not self.is_valid(col, row):
            raise IndexError(f'({col}, {row}) is not a valid index for {self!r}')
        return self.data[row * self.n_cols + col]

    def __setitem__(self, index: tuple[int, int], value: T):
        col, row = index
        if not self.is_valid(col, row):
            raise IndexError(f'({col}, {row}) is not a valid index for {self!r}')
        self.data[row * self.n_cols + col] = value

    def __iter__(self) -> list[list['T']]:
        for row in range(self.n_rows):
            yield [self[col, row] for col in range(self.n_cols)]
    
    def __len__(self):
        return self.n_rows
    
    def __eq__(self, other: 'array2d') -> bool:
        if not isinstance(other, array2d):
            return NotImplemented
        for i in range(self.numel):
            if self.data[i] != other.data[i]:
                return False
        return True
    
    def __ne__(self, other: 'array2d') -> bool:
        return not self.__eq__(other)

    def __repr__(self):
        return f'array2d({self.n_cols}, {self.n_rows})'

    def map(self, f: Callable[[T], Any]) -> 'array2d':
        new_a: array2d = array2d(self.n_cols, self.n_rows)
        for i in range(self.n_cols * self.n_rows):
            new_a.data[i] = f(self.data[i])
        return new_a
    
    def copy(self) -> 'array2d[T]':
        new_a: array2d[T] = array2d(self.n_cols, self.n_rows)
        new_a.data = self.data.copy()
        return new_a

    def fill_(self, value: T) -> None:
        for i in range(self.numel):
            self.data[i] = value

    def apply_(self, f: Callable[[T], T]) -> None:
        for i in range(self.numel):
            self.data[i] = f(self.data[i])

    def copy_(self, other: 'array2d[T] | list[T]') -> None:
        if isinstance(other, list):
            assert len(other) == self.numel
            self.data = other.copy()
            return
        self.n_cols = other.n_cols
        self.n_rows = other.n_rows
        self.data = other.data.copy()

    # for cellular automata
    def count_neighbors(self, value) -> 'array2d[int]':
        new_a = array2d(self.n_cols, self.n_rows)
        for j in range(self.n_rows):
            for i in range(self.n_cols):
                count = 0
                count += int(self.is_valid(i-1, j-1) and self[i-1, j-1] == value)
                count += int(self.is_valid(i, j-1) and self[i, j-1] == value)
                count += int(self.is_valid(i+1, j-1) and self[i+1, j-1] == value)
                count += int(self.is_valid(i-1, j) and self[i-1, j] == value)
                count += int(self.is_valid(i+1, j) and self[i+1, j] == value)
                count += int(self.is_valid(i-1, j+1) and self[i-1, j+1] == value)
                count += int(self.is_valid(i, j+1) and self[i, j+1] == value)
                count += int(self.is_valid(i+1, j+1) and self[i+1, j+1] == value)
                new_a[i, j] = count
        return new_a
