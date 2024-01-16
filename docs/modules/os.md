---
icon: package-dependencies
label: os
---

!!!
This module is optional. Set `PK_ENABLE_OS` to `1` to enable it.
!!!

### `os.getcwd()`

Returns the current working directory.

### `os.chdir(path: str)`

Changes the current working directory to the given path.

### `os.listdir(path: str)`

Returns a list of files and directories in the given path.

### `os.remove(path: str)`

Removes the file at the given path.

### `os.mkdir(path: str)`

Creates a directory at the given path.

### `os.rmdir(path: str)`

Removes the directory at the given path.

### `os.path.join(*paths: str)`

Joins the given paths together.

### `os.path.exists(path: str)`

Check if the given path exists.

### `os.path.basename(path: str)`

Returns the basename of the given path.
