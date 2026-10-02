---
icon: package
label: picoterm and conio
---

# picoterm and conio

Terminal helpers for text-based interfaces.

## picoterm

| Function | Use |
| --- | --- |
| `wcwidth(codepoint)` | Width estimate for a Unicode code point given as an integer. |
| `wcswidth(text)` | Width estimate for text, excluding recognized ANSI escapes. |
| `split_ansi_escaped_string(text)` | Split plain text, newlines, and recognized escape sequences into tokens. |
| `sscanf(text, format, output)` | Parse decimal `%d`/`%i` fields into a list; return whether the format matched. |
| `enable_full_buffering_mode()` | Enable a 32 KB buffer on native standard output. |

```python
import picoterm

assert picoterm.wcswidth('hello') == 5
assert picoterm.wcswidth('\x1b[31mhello\x1b[0m') == 5

coordinates = []
assert picoterm.sscanf('12,34', '%d,%d', coordinates)
assert coordinates == [12, 34]
```

Width estimates use the module's character rules, not a complete grapheme or
terminal-emulator model. The parser supports integer fields and exact literal
characters, not the complete C `scanf` format language. It clears the output
list before parsing and can leave partial results after a mismatch.

## conio

On desktop targets with `PK_ENABLE_OS=1`, `conio._kbhit()` reports keyboard
input availability and `conio._getch()` reads a character code without ordinary
line-buffered input. These functions require an appropriate terminal and are
not a portable browser/mobile input API.

For an embedded UI, prefer the application's own input events and
[output callbacks](../C-API/modules.md).

API declarations:
[picoterm.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/picoterm.pyi),
[conio.pyi](https://github.com/pocketpy/pocketpy/blob/main/include/typings/conio.pyi).
