---
icon: dot
label: 'Advanced config'
order: -2
---

You can create a `user_config.h` in the same directory as `pocketpy.h` to override some default settings.

1. Copy [src/config.h](https://github.com/blueloveTH/pocketpy/blob/main/src/config.h) and rename it to `user_config.h`.
2. Define a macro `PK_USER_CONFIG_H` in `user_config.h`. This invalidates the default `config.h` and enables your `user_config.h`.
3. Edit `user_config.h` to override default settings.