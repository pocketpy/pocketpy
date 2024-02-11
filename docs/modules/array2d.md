---
icon: package
label: array2d
---

Efficient general-purpose 2D array.

https://github.com/pocketpy/pocketpy/blob/main/include/typings/array2d.pyi

## Example

```python
from array2d import array2d

a = array2d(3, 4, default=0)

a[1, 2] = 5
print(a[1, 2]) # 5
```