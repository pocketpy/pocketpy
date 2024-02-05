---
icon: dot
label: 'Advanced Config'
order: -2
---

### Enable os-related features

If you want to enable os-related features, you can do this before including `pocketpy.h`.

```cpp
#define PK_ENABLE_OS 1
#include <pocketpy.h>
```

### Working with multiple threads

pkpy does not support multi-threading. But you can create multiple `VM` instances and run them in different threads.
You can do the following to ensure thread safety for `VM` instances:

```cpp
#define PK_ENABLE_THREAD 1
#include <pocketpy.h>
```

### Full config

You can create a `user_config.h` in the same directory as `pocketpy.h` to override some default settings.

1. Copy [src/config.h](https://github.com/pocketpy/pocketpy/blob/main/include/pocketpy/config.h) and rename it to `user_config.h`.
2. Define a macro `PK_USER_CONFIG_H` in `user_config.h`. This invalidates the default `config.h` and enables your `user_config.h`.
3. Edit `user_config.h` to override default settings.
