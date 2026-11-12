from typing import Literal

configmacros: dict[str, int]

def memory_usage_info() -> str: ...
def memory_usage() -> int: ...


def currentvm() -> int:
    """Return the current VM index."""


def watchdog_begin(timeout: int):
    """Begin the watchdog with `timeout` in milliseconds of process CPU time.

    `PK_ENABLE_WATCHDOG` must be defined to `1` to use this feature.
    You need to call `watchdog_end()` later.
    If `timeout` is reached, `TimeoutError` will be raised.
    """
def watchdog_end() -> None:
    """End the watchdog after a call to `watchdog_begin()`."""

def profiler_begin() -> None: ...
def profiler_end() -> None: ...
def profiler_reset() -> None: ...
def profiler_report() -> dict[str, list[list]]: ...

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
