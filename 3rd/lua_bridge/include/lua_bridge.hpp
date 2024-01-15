#pragma once

#include "pocketpy.h"

extern "C"{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

namespace pkpy{

    void initialize_lua_bridge(VM* vm, lua_State* newL);

}   // namespace pkpy