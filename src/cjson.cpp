#include "pocketpy/cffi.h"
#include "pocketpy/cJSON_c.h"


namespace pkpy{

void add_module_cjson(VM* vm){
    PyObject* mod = vm->new_module("cjson");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
        const Str& expr = CAST(Str&, args[0]);

        cJSON *json = cJSON_Parse(expr.c_str());
        char *json_str = cJSON_Print(json);

        return vm->eval(json_str);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
        
    });
}

}   // namespace pkpy
