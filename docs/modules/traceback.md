---
icon: package
label: traceback
---

### `traceback.print_exc() -> None`

Print the exception currently being handled and its traceback. This also works
from helper functions called by an exception handler. If no exception is being
handled, print nothing.

### `traceback.format_exc() -> str | None`

Return the exception currently being handled and its traceback as a string.
This uses the same exception as `print_exc()`, including inside helper functions
and nested `try` blocks. Return `None` if no exception is being handled.
