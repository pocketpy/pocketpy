---
icon: package
label: periphery
---

# periphery

Low-level bindings to the bundled c-periphery library for Linux GPIO, SPI,
I2C, serial, PWM, LED, and memory-mapped I/O.

!!!info Optional Linux module
Enable `PK_BUILD_MODULE_PERIPHERY=ON` and initialize its Git submodule.
See [build configuration](../build.md).
!!!

To confirm that a build includes it, run on the target Linux system:

```python
import periphery

print(periphery.periphery_version())
```

This module follows the **C API**, using function names such as `gpio_new`,
`gpio_open`, `gpio_read`, `gpio_close`, and `gpio_free`.
It does not provide the high-level class API of the separate
`python-periphery` package.

Handles and pointer arguments are integer addresses. Use [stdc](stdc.md)
to prepare typed storage for output parameters. Follow each function's return
code contract and keep buffers alive throughout native calls.
Close opened devices and free allocated handles on both success and failure;
pocketpy's `with` does not provide general exception cleanup.

Device paths, permissions, pin numbering, and electrical settings depend on
the board. Choose them from your hardware configuration rather than copying
a generic output example.

Full binding signatures:
[periphery.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/periphery.pyi).
The bundled
[c-periphery documentation](https://github.com/vsergeev/c-periphery/tree/master/docs)
describes native arguments and return codes.
