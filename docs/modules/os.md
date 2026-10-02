---
icon: package
label: os and io
---

# os and io

Host filesystem access, available when `PK_ENABLE_OS=1`.

## Files

`open(path, mode)` constructs an `io.FileIO`. Both arguments are required.
Use modes such as `'r'`, `'w'`, `'a'`, `'rb'`, and `'wb'`.
Binary modes ending in `b` read/write bytes; text modes read/write strings.

```python
import os

file = open('example.txt', 'w')
try:
    assert file.write('hello') == 5
except Exception:
    file.close()
    raise
file.close()

file = open('example.txt', 'r')
try:
    text = file.read()
except Exception:
    file.close()
    raise
file.close()

assert text == 'hello'
assert os.path.exists('example.txt')
os.remove('example.txt')
```

| Method | Behavior |
| --- | --- |
| `read(size=-1)` | Read up to a byte count, or the remaining contents when omitted/negative. |
| `write(data)` | Write a string or bytes according to the mode; return the byte count written. |
| `tell()` | Return the current file position. |
| `seek(offset, whence)` | Seek using both arguments; return the C `fseek` status. |
| `flush()`, `close()` | Flush buffered output or close the handle. |

`io.SEEK_SET`, `io.SEEK_CUR`, and `io.SEEK_END` select the origin for
`seek`. There are no encoding/error-policy arguments, `StringIO`,
`BytesIO`, or full CPython text-stream interfaces.

Close files explicitly on both normal and exceptional paths.
The [context manager limitations](../features/differences.md#context-managers)
also apply to file objects.

## OS functions

| API | Behavior |
| --- | --- |
| `getcwd()` | Current process working directory. |
| `chdir(path)` | Change that directory; this affects relative imports and file paths. |
| `remove(path)` | Remove a file through the host C library. |
| `path.exists(path)` | Test whether a path exists on supported targets. |
| `system(command)` | Execute a host command on desktop platforms and return its status. |
| `environ` | An initially empty dictionary, not a live view of the process environment. |

Availability of individual OS operations depends on the target.
The module does not provide CPython's full path utilities or directory APIs.
For virtual filesystems or packaged scripts, configure
[host import callbacks](../C-API/modules.md).

Implementation: [os.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/os.c).
