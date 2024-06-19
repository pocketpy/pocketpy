---
icon: log
title: 'Migration Guide'
order: 22
---

## Overview

v2.0 branch is a complete refactor of pocketpy in C11,
which enables users to run pocketpy on platforms that do not support C++.
Also we redesign the core interpreter to be more efficient and maintainable
by using modern C11 language features.

> v2.0 will be released on 2024/08.

## API compatibility

|  name | v1.x | v2.0 |
| --- | --- | --- |
| legacy C++ API (v1.x only) | ✅ | ❌ |
| legacy C API (v1.x only) | ✅ | ❌ |
| C11 API (v2.0 only) | ❌ | ✅ (work-in-progress) |
| pybind11 API (both) | ✅ | ✅ (work-in-progress) |

## Suggestions

- If you are a C++ user
    - Use **pybind11 API** if you want to upgrade to v2.0 in the future
    - Use **legacy C++ API** if you want to stay in v1.x
- If you are a C user
    - Use **v2.0's C11 API** (will be available soon)
    - Use **legacy C API** if you want to stay in v1.x
