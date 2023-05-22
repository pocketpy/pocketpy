---
icon: dot
title: Goto Statement
---

pkpy supports `goto` and `label` just like C. You are allowed to change the control flow unconditionally.

## Syntax

```
$goto <identifier>
$label <identifier>
```

## Example

```python
for i in range(10):
  for j in range(10):
    for k in range(10):
      $goto exit

$label exit
```
