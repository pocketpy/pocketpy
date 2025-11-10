---
icon: package
label: msgpack
---

!!!
This module is optional. Set option `PK_BUILD_MODULE_MSGPACK` to `ON` in your `CMakeLists.txt` to enable it.
!!!

### `msgpack.loads(data: bytes)`

Decode a msgpack bytes into a python object.

### `msgpack.dumps(obj) -> bytes`

Encode a python object into a msgpack bytes.
