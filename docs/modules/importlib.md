---
icon: package
label: importlib
---

# importlib

`reload(module)` re-executes an already imported source module and returns
the same module object. It reloads `.py` source through the current VM's
[import callback](../C-API/modules.md), not compiled `.pyc` or native modules.

For example, save this as `settings.py`:

```python
difficulty = 3
```

Run the following from the directory containing that file:

```python
import importlib
import settings

original = settings
reloaded = importlib.reload(settings)
assert reloaded is original
print(settings.difficulty)  # 3, or the new value after editing settings.py
```

In a long-running host, edit the source and call `reload()` again to pick up
the new contents.

Reloading preserves the module dictionary and attempts to reuse existing
class types. Names removed from the source can remain in the dictionary.
References obtained through `from settings import difficulty` do not update
automatically; prefer `settings.difficulty` for values intended to change.

Reloading executes module-level side effects again and does not roll back a
partially failed reload. Use it for controlled development workflows rather
than assuming it resets all application state.
