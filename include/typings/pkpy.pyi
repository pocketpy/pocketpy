from typing import Self, Literal
from vmath import vec2, vec2i

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

def enable_full_buffering_mode() -> None:
    """Enable full buffering mode for ASCII drawings."""

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

    def submit_exec(self, source: str) -> None:
        """Submit a job to execute some source code."""

    def submit_eval(self, source: str) -> None:
        """Submit a job to evaluate some source code."""

    def submit_call(self, eval_src: str, *args, **kwargs) -> None:
        """Submit a job to call a function with arguments."""

    def exec(self, source: str) -> None:
        """Directly execute some source code."""

    def eval(self, source: str):
        """Directly evaluate some source code."""
