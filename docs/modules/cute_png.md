---
icon: package
label: cute_png
---

# cute_png

PNG decoding and encoding backed by the bundled cute_png library.

!!!info Optional module
Enable `PK_BUILD_MODULE_CUTE_PNG=ON`; see [build configuration](../build.md).
!!!

## Encode and decode in memory

```python
from cute_png import Image
from vmath import color32

image = Image(2, 2)
image.clear(color32(0, 0, 0, 255))
image.setpixel(1, 0, color32(255, 0, 0, 255))

data = image.to_png_bytes()
restored = Image.from_bytes(data)
assert restored.width == 2
assert restored.height == 2
assert restored.getpixel(1, 0) == color32(255, 0, 0, 255)
```

| API | Purpose |
| --- | --- |
| `Image(width, height)` | Allocate an image; initialize pixels with `clear()` or `setpixel()`. |
| `Image.from_bytes(data)`, `Image.from_file(path)` | Decode a PNG. |
| `getpixel(x, y)`, `setpixel(x, y, color)` | Read/write a `color32` pixel. |
| `clear(color)` | Fill the image. |
| `to_png_bytes()`, `to_png_file(path)` | Encode the image. |
| `to_rgb565_file(path)` | Write RGB565 pixel data. |
| `paste(...)` | Copy a region with foreground/background coloring. |

Coordinates use `(x, y)`, and must be within the image.
File operations require an accessible host filesystem.
For scripts using [array2d](array2d.md), `loads(data)` returns an
`array2d[color32]` and `dumps(grid)` returns PNG bytes.

Full signatures, including `paste`:
[cute_png.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/cute_png.pyi).
