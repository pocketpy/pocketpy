from typing import Self
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
