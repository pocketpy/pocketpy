---
title: General
icon: dot
order: 7
---
#### `void pkpy_delete(void* p)`

Delete a pointer allocated by `pkpy_xxx_xxx`.
It can be `VM*`, `REPL*`, `char*`, etc.

!!!
If the pointer is not allocated by `pkpy_xxx_xxx`, the behavior is undefined.
!!!
