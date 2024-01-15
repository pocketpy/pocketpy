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