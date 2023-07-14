---
title: Specials
icon: dot
order: 6
---

+ `void pkpy_free(void* p)`

    Wraps `free(p)` in C++.

+ `pkpy_CString pkpy_string(const char*)`

    Construct a `pkpy_CString` from a null-terminated C string.

+ `pkpy_CName pkpy_name(const char*)`

    Construct a `pkpy_CName` from a null-terminated C string. You should cache the result of this function if you are going to use it multiple times.