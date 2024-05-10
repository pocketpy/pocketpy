---
icon: package
label: operator
---

The operator module exports a set of efficient functions corresponding to the intrinsic operators of Python. For example, `operator.add(x, y)` is equivalent to the expression `x+y`. Many function names are those used for special methods, without the double underscores.

## Mapping Operators to Functions

| Operation | Syntax | Function |
| --- | --- | --- |
| Ordering | `a <= b` | `le(a, b)` |
| Ordering | `a < b` | `lt(a, b)` |
| Ordering | `a >= b` | `ge(a, b)` |
| Ordering | `a > b` | `gt(a, b)` |
| Equality | `a == b` | `eq(a, b)` |
| Equality | `a != b` | `ne(a, b)` |
| Bitwise AND | `a & b` | `and_(a, b)` |
| Bitwise OR | `a | b` | `or_(a, b)` |
| Bitwise XOR | `a ^ b` | `xor(a, b)` |
| Bitwise Inversion | `~a` | `invert(a)` |
| Left Shift | `a << b` | `lshift(a, b)` |
| Right Shift | `a >> b` | `rshift(a, b)` |
| Identity | `a is b` | `is_(a, b)` |
| Identity | `a is not b` | `is_not(a, b)` |
| Negation (Logical) | `not a` | `not_(a)` |
| Negation (Arithmetic) | `-a` | `neg(a)` |
| Truth Test | `bool(a)` | `truth(a)` |
| Containment Test | `b in a` | `contains(a, b)` |
| Addition | `a + b` | `add(a, b)` |
| Subtraction | `a - b` | `sub(a, b)` |
| Multiplication | `a * b` | `mul(a, b)` |
| Division | `a / b` | `truediv(a, b)` |
| Division | `a // b` | `floordiv(a, b)` |
| Modulo | `a % b` | `mod(a, b)` |
| Exponentiation | `a ** b` | `pow(a, b)` |
| Matrix Multiplication | `a @ b` | `matmul(a, b)` |
| Indexing | `a[b]` | `getitem(a, b)` |
| Index Assignment | `a[b] = c` | `setitem(a, b, c)` |
| Index Deletion | `del a[b]` | `delitem(a, b)` |


## In-place Operators
| Operation | Syntax | Function |
| --- | --- | --- |
| Addition | `a += b` | `iadd(a, b)` |
| Subtraction | `a -= b` | `isub(a, b)` |
| Multiplication | `a *= b` | `imul(a, b)` |
| Division | `a /= b` | `itruediv(a, b)` |
| Division | `a //= b` | `ifloordiv(a, b)` |
| Modulo | `a %= b` | `imod(a, b)` |
| Bitwise AND | `a &= b` | `iand(a, b)` |
| Bitwise OR | `a |= b` | `ior(a, b)` |
| Bitwise XOR | `a ^= b` | `ixor(a, b)` |
| Left Shift | `a <<= b` | `ilshift(a, b)` |
| Right Shift | `a >>= b` | `irshift(a, b)` |
