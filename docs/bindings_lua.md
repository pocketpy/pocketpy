---
icon: cpu
title: Reuse Lua Bindings
order: 17
---

!!!
This feature is available in `v1.4.0` or higher.
!!!

pkpy provides a lua bridge to reuse lua bindings.
It allows you to run lua code and call lua functions in python
by embedding a lua virtual machine.

Add `lua_bridge.hpp` and `lua_bridge.cpp` in [3rd/lua_bridge](https://github.com/pocketpy/pocketpy/tree/main/3rd/lua_bridge) to your project.
Make sure `lua.h`, `lualib.h` and `lauxlib.h` are in your include path
because `lua_bridge.hpp` needs them.

The lua bridge is based on lua 5.1.5 for maximum compatibility.
lua 5.2 or higher should also work.

### Setup

Use `initialize_lua_bridge(VM*, lua_State*)` to initialize the lua bridge.
This creates a new module `lua` in your python virtual machine.

You can use `lua.dostring` to execute lua code and get the result.
And use `lua.Table()` to create a lua table.
A `lua.Table` instance in python is a dict-like object which provides a bunch of
magic methods to access the underlying lua table.

```python
class Table:
    def keys(self) -> list:
        """Return a list of keys in the table."""

    def values(self) -> list:
        """Return a list of values in the table."""

    def items(self) -> list[tuple]:
        """Return a list of (key, value) pairs in the table."""

    def __len__(self) -> int:
        """Return the length of the table."""

    def __contains__(self, key) -> bool:
        """Return True if the table contains the key."""

    def __getitem__(self, key): ...
    def __setitem__(self, key, value): ...
    def __delitem__(self, key): ...
    def __getattr__(self, key): ...
    def __setattr__(self, key, value): ...
    def __delattr__(self, key): ...
```

Only basic types can be passed between python and lua.
The following table shows the type mapping.
If you pass an unsupported type, an exception will be raised.

| Python type   | Lua type  | Allow create in Python? | Reference? |
| -----------   | --------  | ---------------------- |  --------- |
| `None`        | `nil`     | YES                    |  NO        |
| `bool`        | `boolean` | YES                    |  NO        |
| `int`         | `number`  | YES                    |  NO        |
| `float`       | `number`  | YES                    |  NO        |
| `str`         | `string`  | YES                    |  NO        |
| `tuple`       | `table`   | YES                    |  NO        |
| `list`        | `table`   | YES                    |  NO        |
| `dict`        | `table`   | YES                    |  NO        |
| `lua.Table`   | `table`   | YES                    |  YES       |
| `lua.Function`| `function`| NO                     |  YES       |

### Example
```cpp
#include "lua_bridge.hpp"

using namespace pkpy;

int main(){
    VM* vm = new VM();

    // create lua state
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // initialize lua bridge
    initialize_lua_bridge(vm, L);

    // dostring to get _G
    vm->exec("import lua");
    vm->exec("g = lua.dostring('return _G')");

    // create a table
    vm->exec("t = lua.Table()");
    vm->exec("t.a = 1");
    vm->exec("t.b = 2");

    // call lua function
    vm->exec("g.print(t.a + t.b)");     // 3
    
    return 0;
}
```