---
icon: package
label: json
---

JSON serialization and deserialization module.

This module is not safe. You may not want to use it with untrusted data.
If you need a safe alternative, consider a 3rd-party library like `cjson`.

You can override the json functions with:
```c
py_GlobalRef mod = py_getmodule("json");
py_bindfunc(mod, "loads", _safe_json_loads);
py_bindfunc(mod, "dumps", _safe_json_dumps);
```

#### Source code

:::code source="../../include/typings/json.pyi" :::

