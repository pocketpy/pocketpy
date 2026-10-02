---
icon: dot
title: Unsupported Operations
order: 98
---

# Unsupported operations

The following operations violate runtime assumptions. They may crash the
interpreter or produce incorrect results instead of a Python exception.

| Operation | Example or explanation |
| --- | --- |
| Modify or delete an attribute of a built-in type | `int.__add__ = custom_add` or `del int.__add__`. Optimized paths assume the original methods. |
| Call an unbound method with an invalid `self` | `int.__add__('1', 2)`. Use ordinary operations on valid instances. |
| Return an unrelated object from `T.__new__` | The result must be an instance of `T`. |
| Pass an invalid class argument to `__new__` | Constructors expect a type object and the appropriate inheritance relationship. |

These restrictions are distinct from the documented
[language differences](differences.md), such as unsupported syntax.

At the C API boundary, respect the [reference lifetimes](../C-API/introduction.md):
do not use a stale item reference, an object from another VM, or a reference
after reset/finalization. Check types before low-level casts and do not access
one VM concurrently from multiple threads.

Use Debug builds during binding development. They include checks for common
native callback errors, such as an unbalanced stack or a missing return value.
