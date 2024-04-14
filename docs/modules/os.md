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

### `os.path.isdir(path: str)`

Check if the given path is a directory.

### `os.path.isfile(path: str)`

Check if the given path is a file.

### `os.path.abspath(path: str)`

Returns the absolute path of the given path.


## Other functions

You can add other functions to `os` module via normal binding if you need them.
For example, add `os.system`:

```cpp
PyObject* mod = vm->_modules["os"];

vm->bind(mod, "system(cmd: str) -> int", [](VM* vm, ArgsView args){
    const char* cmd = py_cast<CString>(vm, args[0]);
    int code = system(cmd);
    return py_var(vm, code);
});
```
