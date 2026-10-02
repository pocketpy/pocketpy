---
title: Modules and Host Callbacks
icon: dot
order: 8
---

# Modules and host callbacks

A module holds Python globals. Native modules created with `py_newmodule()`
can be imported by scripts in the current VM, just like source modules.

## Create or look up a module

This fragment runs after initialization:

```c
py_GlobalRef settings = py_newmodule("settings");
py_newint(py_emplacedict(settings, py_name("screen_width")), 1280);

bool ok = py_exec("import settings\nprint(settings.screen_width)",
                  "app.py", EXEC_MODE, NULL);
if(!ok) py_printexc();
```

Create a module only once per VM: recreating an existing name is an error at the
native API level. `py_getmodule("settings")` looks up an already registered
module and returns `NULL` if it is absent; it does not perform an import.

`py_import(name)` attempts to load a module and returns:

| Return | Meaning |
| --- | --- |
| `1` | Success; the module is in `py_retval()`. |
| `0` | The module was not found; the caller decides how to report this. |
| `-1` | Loading failed with a Python exception. |

Do not treat `py_import()` as a boolean: `-1` is true in a C condition.

## Where imports come from

The importer first checks registered modules, then the host's optional
`lazyimport` callback, then bundled Python modules. For a file-based module
such as `game.level`, it asks `importfile` for these paths in order:

1. `game/level.py`
2. `game/level.pyc`
3. `game/level/__init__.py`
4. `game/level/__init__.pyc`

Path separators follow the platform. Parent packages are loaded first.
Supported desktop builds may then try a native dynamic module if enabled.

The default file callback opens paths relative to the **process working
directory**. Running `main scripts/app.py` does not automatically add
`scripts/` to an import path. Start the host in the intended script directory
or provide your own loader. pocketpy does not provide CPython's `sys.path`
search mechanism.

When both source and bytecode exist, source is preferred.
See [bytecode deployment](../features/deploy.md) for the distribution format.

## Load a module from memory

Replace `py_callbacks()->importfile` to load scripts from a game asset bundle,
a virtual filesystem, or embedded strings. This complete example provides one
module without reading a file:

```c
#include "pocketpy.h"
#include <string.h>

static char* import_source(const char* path, int* data_size) {
    if(strcmp(path, "settings.py") != 0) return NULL;

    const char* source = "screen_width = 1280\n";
    size_t size = strlen(source);
    char* buffer = py_malloc(size + 1);
    memcpy(buffer, source, size + 1);
    if(data_size != NULL) *data_size = (int)size;
    return buffer;
}

int main(void) {
    py_initialize();
    py_callbacks()->importfile = import_source;

    bool ok = py_exec("import settings\nassert settings.screen_width == 1280",
                      "app.py", EXEC_MODE, NULL);
    if(!ok) py_printexc();

    py_finalize();
    return ok ? 0 : 1;
}
```

The loader contract matters:

- Return `NULL` when the requested path is unavailable.
- Allocate a fresh buffer with `py_malloc()`; the interpreter owns and frees it.
  Do not return a string literal, stack buffer, or shared asset pointer.
- NUL-terminate source text. For bytecode, report the exact byte length.
- Check whether `data_size` is `NULL`: source reloads can omit this output.
- Handle every path variant that your application supports.

Callbacks belong to the current VM. Configure each VM that needs a custom
loader, and configure it again after a reset.

## Redirect script output

`py_callbacks()->print` receives NUL-terminated UTF-8 text.
A single Python `print()` can call it more than once; do not assume each
callback contains a whole line. Set `flush` if your output backend buffers
data. `py_printexc()` also uses this output callback.

The `getchr` callback supplies input for `input()`. More specialized hooks,
including `gc_mark` and `displayhook`, are declared in
[the public header](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/pocketpy.h).
