---
icon: package
label: stdc
---

# stdc

Typed native storage and raw-memory operations for C bindings.

## Typed storage

```python
from stdc import Int32, addressof, sizeof

value = Int32(42)
assert value.value == 42
Int32.write(addressof(value), 0, 100)
assert value.value == 100

items = Int32.array(3)
items[0] = 10
items[1] = 20
items[2] = 30
assert Int32.read(addressof(items), 1) == 20
assert sizeof(Int32) == 4
```

`Type(value)` creates scalar storage; `Type.array(length)` creates an array.
The wrapper keeps that storage alive. Initialize array elements before reading
them. `read(pointer, offset)` and `write(pointer, offset, value)` use offsets
in **elements of the selected type**, not bytes.

| Family | Types |
| --- | --- |
| C integers | `Char`, `UChar`, `Short`, `UShort`, `Int`, `UInt`, `Long`, `ULong`, `LongLong`, `ULongLong` |
| Fixed widths | `Int8/16/32/64`, `UInt8/16/32/64` |
| Other C values | `Float`, `Double`, `Bool`, `Pointer` |
| Platform sizes | `SizeT`, `IntPtrT`, `UIntPtrT` |

Use `sizeof(type)` to query the native size rather than assuming a platform's
`long` or pointer width.

## Raw memory

The module exposes `malloc`, `free`, `memcpy`, `memset`, `memcmp`,
`read_cstr`, `write_cstr`, `read_bytes`, and `write_bytes`.
Pointer values are integers.

These operations do not provide Python's memory safety. Validate addresses,
sizes, and ownership; retain the wrapper while its address is in use.
Allocate enough space for string terminators, and free manually allocated
memory exactly once. Do not call `free()` on memory owned by a typed wrapper.

Full signatures:
[stdc.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/stdc.pyi).
Runtime type names follow
[stdc.c](https://github.com/pocketpy/pocketpy/blob/main/src/modules/stdc.c).
