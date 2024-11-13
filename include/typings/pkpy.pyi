from typing import Self

class TValue[T]:
    def __new__(cls, value: T) -> Self: ...
    
    @property
    def value(self) -> T: ...
