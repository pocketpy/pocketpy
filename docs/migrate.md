---
icon: log
title: 'Migration Guide'
order: 22
---

## Overview

This document describes the changes from v1.x to v2.0.

## API compatibility

|  name | v1.x | v2.0 |
| --- | --- | --- |
| legacy C++ API (v1.x only) | ✅ | ❌ |
| legacy C API (v1.x only) | ✅ | ❌ |
| C11 API (v2.0 only) | ❌ | ✅ |
| pybind11 API (both) | ✅ | ✅ |

## Suggestions

- If you are a C++ user
    - Use **pybind11 API** if you want to upgrade to v2.0 in the future
    - Use **legacy C++ API** if you want to stay in v1.x
- If you are a C user
    - Use **v2.0's C11 API** (will be available soon)
    - Use **legacy C API** if you want to stay in v1.x