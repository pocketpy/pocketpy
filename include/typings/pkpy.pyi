from typing import Self, Literal
from linalg import vec2, vec2i

class TValue[T]:
    def __new__(cls, value: T) -> Self: ...
    
    @property
    def value(self) -> T: ...

# TValue_int = TValue[int]
# TValue_float = TValue[float]
# TValue_vec2i = TValue[vec2i]
# TValue_vec2 = TValue[vec2]

def memory_usage() -> str:
    """Return a summary of the memory usage."""

def is_user_defined_type(t: type) -> bool:
    """Check if a type is user-defined. This means the type was created by executing python `class` statement."""

def currentvm() -> int:
    """Return the current VM index."""


class ComputeThread:
    def __init__(self, vm_index: Literal[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]): ...

    @property
    def is_done(self) -> bool:
        """Check if the current job is done."""

    def wait_for_done(self) -> None:
        """Wait for the current job to finish."""

    def last_error(self) -> str | None: ...
    def last_retval(self): ...

    def exec(self, source: str) -> None: ...
    def eval(self, source: str) -> None: ...
    def call(self, eval_src: str, *args, **kwargs) -> None: ...
