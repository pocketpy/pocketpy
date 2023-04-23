---
icon: dot
---

# ternary op

Ternary operator is a short hand `if...else`.
pkpy supports both c and python style ternary.


## Syntax

```
<condition> ? <true_expr> : <false_expr>
```

```
<true_expr> if <condition> else <false_expr>
```

## Example

```python
a = 1
s = a == 1 ? "a is 1" : "a is not 1"
print(s)    # a is 1

# which equals to
s = "a is 1" if a == 1 else "a is not 1"
```