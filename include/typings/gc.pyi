from typing import Callable, Literal

def isenabled() -> bool:
    """Check if automatic garbage collection is enabled."""

def enable() -> None:
    """Enable automatic garbage collection."""

def disable() -> None:
    """Disable automatic garbage collection."""

def collect() -> int:
    """Run a full collection immediately.

    Returns an integer indicating the number of unreachable objects found.
    """

def collect_hint() -> None:
    """Hint the garbage collector to run a collection.

    The typical usage scenario for this function is in frame-driven games,
    where `gc.disable()` is called at the start of the game,
    and `gc.collect_hint()` is called at the end of each frame.
    """

def setup_debug_callback(cb: Callable[[Literal['start', 'stop'], str], None] | None) -> None:
    """Setup a callback that will be triggered at the end of each collection."""
