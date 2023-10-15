---
icon: package
label: json
---

pkpy has two JSON modules.
1. The built-in JSON module is always available and can be imported via `import json`.
2. After `v1.2.7`, you can set `PK_USE_CJSON` to `ON` in CMakeLists.txt to enable an alternative JSON module `cjson`.

Their interfaces are the same, `cjson` is faster while the built-in `json` is more stable since it was developed earlier.

### `json.loads(s)`

Decode a JSON string into a python object.

### `json.dumps(obj)`

Encode a python object into a JSON string.

