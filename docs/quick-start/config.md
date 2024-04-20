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

You can take a look at `include/pocketpy/config.h` to see all the available configuration macros.


