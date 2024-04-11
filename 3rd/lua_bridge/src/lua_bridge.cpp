#include "lua_bridge.hpp"

namespace pkpy{

static lua_State* _L;
static void lua_push_from_python(VM*, PyObject*);
static PyObject* lua_popx_to_python(VM*);

template<typename T>
static void table_apply(VM* vm, T f){
    PK_ASSERT(lua_istable(_L, -1));
    lua_pushnil(_L);                 // [key]
    while(lua_next(_L, -2) != 0){    // [key, val]
        lua_pushvalue(_L, -2);       // [key, val, key]
        PyObject* key = lua_popx_to_python(vm);
        PyObject* val = lua_popx_to_python(vm);
        f(key, val);                // [key]
    }
    lua_pop(_L, 1);                  // []
}

struct LuaExceptionGuard{
    int base_size;
    LuaExceptionGuard(){ base_size = lua_gettop(_L); }
    ~LuaExceptionGuard(){
        int delta = lua_gettop(_L) - base_size;
        if(delta > 0) lua_pop(_L, delta);
    }
};

#define LUA_PROTECTED(__B) { LuaExceptionGuard __guard; __B; }

struct PyLuaObject{
    PK_ALWAYS_PASS_BY_POINTER(PyLuaObject)
    int r;
    PyLuaObject(){ r = luaL_ref(_L, LUA_REGISTRYINDEX); }
    ~PyLuaObject(){ luaL_unref(_L, LUA_REGISTRYINDEX, r); }
};

struct PyLuaTable: PyLuaObject{
    PY_CLASS(PyLuaTable, lua, Table)

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        Type t = PK_OBJ_GET(Type, type);
        PyTypeInfo* ti = &vm->_all_types[t];
        ti->subclass_enabled = false;
        ti->m__getattr__ = [](VM* vm, PyObject* obj, StrName name){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                std::string_view name_sv = name.sv();
                lua_pushlstring(_L, name_sv.data(), name_sv.size());
                lua_gettable(_L, -2);
                PyObject* ret = lua_popx_to_python(vm);
                lua_pop(_L, 1);
                return ret;
            )
        };

        ti->m__setattr__ = [](VM* vm, PyObject* obj, StrName name, PyObject* val){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                std::string_view name_sv = name.sv();
                lua_pushlstring(_L, name_sv.data(), name_sv.size());
                lua_push_from_python(vm, val);
                lua_settable(_L, -3);
                lua_pop(_L, 1);
            )
        };

        ti->m__delattr__ = [](VM* vm, PyObject* obj, StrName name){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                std::string_view name_sv = name.sv();
                lua_pushlstring(_L, name_sv.data(), name_sv.size());
                lua_pushnil(_L);
                lua_settable(_L, -3);
                lua_pop(_L, 1);
            )
            return true;
        };

        vm->bind_constructor<1>(type, [](VM* vm, ArgsView args){
            lua_newtable(_L);    // push an empty table onto the stack
            PyObject* obj = vm->heap.gcnew<PyLuaTable>(PK_OBJ_GET(Type, args[0]));
            return obj;
        });

        vm->bind__len__(t, [](VM* vm, PyObject* obj){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
            i64 len = 0;
            lua_pushnil(_L);
            while(lua_next(_L, -2) != 0){ len += 1; lua_pop(_L, 1); }
            lua_pop(_L, 1);
            return len;
        });

        vm->bind__getitem__(t, [](VM* vm, PyObject* obj, PyObject* key){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                lua_push_from_python(vm, key);
                lua_gettable(_L, -2);
                PyObject* ret = lua_popx_to_python(vm);
                lua_pop(_L, 1);
                return ret;
            )
        });

        vm->bind__setitem__(t, [](VM* vm, PyObject* obj, PyObject* key, PyObject* val){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                lua_push_from_python(vm, key);
                lua_push_from_python(vm, val);
                lua_settable(_L, -3);
                lua_pop(_L, 1);
            )
        });

        vm->bind__delitem__(t, [](VM* vm, PyObject* obj, PyObject* key){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                lua_push_from_python(vm, key);
                lua_pushnil(_L);
                lua_settable(_L, -3);
                lua_pop(_L, 1);
            )
        });

        vm->bind__contains__(t, [](VM* vm, PyObject* obj, PyObject* key){
            const PyLuaTable& self = _CAST(PyLuaTable&, obj);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                lua_push_from_python(vm, key);
                lua_gettable(_L, -2);
                bool ret = lua_isnil(_L, -1) == 0;
                lua_pop(_L, 2);
                return ret ? vm->True : vm->False;
            )
        });

        vm->bind(type, "keys(self) -> list", [](VM* vm, ArgsView args){
            const PyLuaTable& self = _CAST(PyLuaTable&, args[0]);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                List ret;
                table_apply(vm, [&](PyObject* key, PyObject* val){ ret.push_back(key); });
                lua_pop(_L, 1);
                return VAR(std::move(ret));
            )
        });

        vm->bind(type, "values(self) -> list", [](VM* vm, ArgsView args){
            const PyLuaTable& self = _CAST(PyLuaTable&, args[0]);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                List ret;
                table_apply(vm, [&](PyObject* key, PyObject* val){ ret.push_back(val); });
                lua_pop(_L, 1);
                return VAR(std::move(ret));
            )
        });

        vm->bind(type, "items(self) -> list[tuple]", [](VM* vm, ArgsView args){
            const PyLuaTable& self = _CAST(PyLuaTable&, args[0]);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                List ret;
                table_apply(vm, [&](PyObject* key, PyObject* val){
                    PyObject* item = VAR(Tuple(key, val));
                    ret.push_back(item);
                });
                lua_pop(_L, 1);
                return VAR(std::move(ret));
            )
        });
    }
};

static PyObject* lua_popx_multi_to_python(VM* vm, int count){
    if(count == 0){
        return vm->None;
    }else if(count == 1){
        return lua_popx_to_python(vm);
    }else if(count > 1){
        Tuple ret(count);
        for(int i=0; i<count; i++){
            ret[i] = lua_popx_to_python(vm);
        }
        return VAR(std::move(ret));
    }
    PK_FATAL_ERROR()
}

struct PyLuaFunction: PyLuaObject{
    PY_CLASS(PyLuaFunction, lua, Function)

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<PyLuaFunction>(type);
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;

        vm->bind_method<-1>(type, "__call__", [](VM* vm, ArgsView args){
            if(args.size() < 1) vm->TypeError("__call__ takes at least 1 argument");
            const PyLuaFunction& self = _CAST(PyLuaFunction&, args[0]);
            int base_size = lua_gettop(_L);
            LUA_PROTECTED(
                lua_rawgeti(_L, LUA_REGISTRYINDEX, self.r);
                for(int i=1; i<args.size(); i++){
                    lua_push_from_python(vm, args[i]);
                }
                if(lua_pcall(_L, args.size()-1, LUA_MULTRET, 0)){
                    const char* error = lua_tostring(_L, -1);
                    lua_pop(_L, 1);
                    vm->RuntimeError(error);
                } 
                return lua_popx_multi_to_python(vm, lua_gettop(_L) - base_size);
            )
        });
    }
};

void lua_push_from_python(VM* vm, PyObject* val){
    if(val == vm->None){
        lua_pushnil(_L);
        return;
    }
    Type t = vm->_tp(val);
    switch(t.index){
        case VM::tp_bool.index:
            lua_pushboolean(_L, val == vm->True);
            return;
        case VM::tp_int.index:
            lua_pushinteger(_L, _CAST(i64, val));
            return;
        case VM::tp_float.index:
            lua_pushnumber(_L, _CAST(f64, val));
            return;
        case VM::tp_str.index: {
            std::string_view sv = _CAST(Str, val).sv();
            lua_pushlstring(_L, sv.data(), sv.size());
            return;
        }
        case VM::tp_tuple.index: {
            lua_newtable(_L);
            int i = 1;
            for(PyObject* obj: PK_OBJ_GET(Tuple, val)){
                lua_push_from_python(vm, obj);
                lua_rawseti(_L, -2, i++);
            }
            return;
        }
        case VM::tp_list.index: {
            lua_newtable(_L);
            int i = 1;
            for(PyObject* obj: PK_OBJ_GET(List, val)){
                lua_push_from_python(vm, obj);
                lua_rawseti(_L, -2, i++);
            }
            return;
        }
        case VM::tp_dict.index: {
            lua_newtable(_L);
            PK_OBJ_GET(Dict, val).apply([&](PyObject* key, PyObject* val){
                lua_push_from_python(vm, key);
                lua_push_from_python(vm, val);
                lua_settable(_L, -3);
            });
            return;
        }
    }

    if(is_type(val, PyLuaTable::_type(vm))){
        const PyLuaTable& table = _CAST(PyLuaTable&, val);
        lua_rawgeti(_L, LUA_REGISTRYINDEX, table.r);
        return;
    }

    if(is_type(val, PyLuaFunction::_type(vm))){
        const PyLuaFunction& func = _CAST(PyLuaFunction&, val);
        lua_rawgeti(_L, LUA_REGISTRYINDEX, func.r);
        return;
    }
    vm->RuntimeError(_S("unsupported python type: ", _type_name(vm, t).escape()));
}

PyObject* lua_popx_to_python(VM* vm) {
    int type = lua_type(_L, -1);
    switch (type) {
        case LUA_TNIL: {
            lua_pop(_L, 1);
            return vm->None;
        }
        case LUA_TBOOLEAN: {
            bool val = lua_toboolean(_L, -1);
            lua_pop(_L, 1);
            return val ? vm->True : vm->False;
        }
        case LUA_TNUMBER: {
            double val = lua_tonumber(_L, -1);
            lua_pop(_L, 1);
            return VAR(val);
        }
        case LUA_TSTRING: {
            const char* val = lua_tostring(_L, -1);
            lua_pop(_L, 1);
            return VAR(val);
        }
        case LUA_TTABLE: {
            PyObject* obj = vm->heap.gcnew<PyLuaTable>(PyLuaTable::_type(vm));
            return obj;
        }
        case LUA_TFUNCTION: {
            PyObject* obj = vm->heap.gcnew<PyLuaFunction>(PyLuaFunction::_type(vm));
            return obj;
        }
        default: {
            const char* type_name = lua_typename(_L, type);
            lua_pop(_L, 1);
            vm->RuntimeError(_S("unsupported lua type: '", type_name, "'"));
        }
    }
    PK_UNREACHABLE()
}

void initialize_lua_bridge(VM* vm, lua_State* newL){
    PyObject* mod = vm->new_module("lua");

    if(_L != nullptr){
        throw std::runtime_error("lua bridge already initialized");
    }
    _L = newL;

    PyLuaTable::register_class(vm, mod);
    PyLuaFunction::register_class(vm, mod);

    vm->bind(mod, "dostring(__source: str)", [](VM* vm, ArgsView args){
        const char* source = CAST(CString, args[0]);
        int base_size = lua_gettop(_L);
        if (luaL_dostring(_L, source)) {
            const char* error = lua_tostring(_L, -1);
            lua_pop(_L, 1);  // pop error message from the stack
            vm->RuntimeError(error);
        }
        return lua_popx_multi_to_python(vm, lua_gettop(_L) - base_size);
    });
}

}   // namespace pkpy
