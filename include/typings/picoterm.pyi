def enable_full_buffering_mode() -> None:
    """Enable full buffering mode for ASCII drawings (32KB)."""

def split_ansi_escaped_string(s: str) -> list[str]:
    """Perform split on ANSI escaped string."""

def wcwidth(c: int) -> int: ...
def wcswidth(s: str) -> int: ...

def sscanf(s: str, fmt: str, out_list: list) -> bool: ...