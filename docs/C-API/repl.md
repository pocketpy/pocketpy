---
title: REPL
icon: dot
order: 8
---
#### `REPL* pkpy_new_repl(VM* vm)`

Create a REPL, using the given virtual machine as the backend.

#### `bool pkpy_repl_input(REPL* r, const char* line)`

Input a source line to an interactive console. Return true if need more lines.