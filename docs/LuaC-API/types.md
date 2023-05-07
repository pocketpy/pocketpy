---
title: Type Checking
icon: dot
order: 7
---

#### `bool pkpy_is_int(pkpy_vm*, int index)`

Return true if the value at the given index is an integer.

#### `bool pkpy_is_float(pkpy_vm*, int index)`

Return true if the value at the given index is a float.

#### `bool pkpy_is_bool(pkpy_vm*, int index)`

Return true if the value at the given index is a boolean.

#### `bool pkpy_is_string(pkpy_vm*, int index)`

Return true if the value at the given index is a string.

#### `bool pkpy_is_voidp(pkpy_vm*, int index)`

Return true if the value at the given index is a void pointer.

#### `bool pkpy_is_none(pkpy_vm*, int index)`

Return true if the value at the given index is `None`.