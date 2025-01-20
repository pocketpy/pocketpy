---
icon: package
label: libhv
---

!!!
This module is optional. Set option `PK_BUILD_MODULE_LIBHV` to `ON` in your `CMakeLists.txt` to enable it.
!!!

`libhv` is a git submodule located at `3rd/libhv/libhv`. If you cannot find it, please run the following command to initialize the submodule:

```bash
git submodule update --init --recursive
```

Simple bindings for [libhv](https://github.com/ithewei/libhv), which provides cross platform implementation of the following:
+ HTTP server
+ HTTP client
+ WebSocket server
+ WebSocket client

#### Source code

:::code source="../../include/typings/libhv.pyi" :::
