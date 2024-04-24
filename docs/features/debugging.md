---
icon: dot
title: Debugging
---

!!!
This feature is available in `v1.4.5` or higher.
!!!

You can invoke `breakpoint()` in your python code to start a PDB-like session.

The following commands are supported:

+ `h, help`: show this help message
+ `q, quit`: exit the debugger
+ `n, next`: execute next line
+ `s, step`: step into
+ `w, where`: show current stack frame
+ `c, continue`: continue execution
+ `a, args`: show local variables
+ `l, list`: show lines around current line
+ `ll, longlist`: show all lines
+ `p, print <expr>`: evaluate expression
+ `!, execute statement`: execute statement
