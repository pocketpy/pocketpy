---
icon: package-dependencies
label: io
---

!!!
This module is optional. Set `PK_ENABLE_OS` to `1` to enable it.
!!!

### `io.FileIO.read(size=-1) -> bytes | str`

Read up to `size` bytes from the file. If `size` is negative or omitted, read until EOF.

### `io.FileIO.write(data: bytes | str)`

Write the given data to the file.

### `io.FileIO.seek(offset, whence=0) -> int`

Change the file position to the given offset. The `whence` argument is optional and defaults to `0` (absolute file positioning); other values are `1` (seek relative to the current position) and `2` (seek relative to the file's end).

### `io.FileIO.tell() -> int`

Return the current file position.

### `io.FileIO.close()`

Close the file.


### `io.SEEK_SET`

Seek from the beginning of the file.

### `io.SEEK_CUR`

Seek from the current position.

### `io.SEEK_END`

Seek from the end of the file.

