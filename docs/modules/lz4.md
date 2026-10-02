---
icon: package
label: lz4
---

# lz4

Fast compression of byte strings.

!!!info Optional module
Enable `PK_BUILD_MODULE_LZ4=ON`; see [build configuration](../build.md).
!!!

| Function | Result |
| --- | --- |
| `compress(data: bytes)` | Compressed bytes with a stored uncompressed size. |
| `decompress(data: bytes)` | Restored bytes. |

```python
import lz4

original = ('tile-data-' * 100).encode()
packed = lz4.compress(original)
assert lz4.decompress(packed) == original
```

The format is an LZ4 **block** preceded by a four-byte uncompressed-size field.
It is not an LZ4 frame or a `.lz4` command-line file.
Use matching block-format APIs when exchanging data with another library.

Decompression allocates from the stored size. Keep compressed data within your
application's size limits; do not treat the size prefix as trustworthy for
untrusted files. Serialize objects before compression, for example using
[msgpack](msgpack.md) when its supported types fit.

Implementation: [lz4.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/lz4.c).
