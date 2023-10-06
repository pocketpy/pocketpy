#include "cjson/cJSON.h"
#include "pocketpy/pocketpy.h"


namespace pkpy{

inline void add_module_cjson(VM* vm){
    PyObject* mod = vm->new_module("cjson");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {
    
        const Str& expr = CAST(Str&, args[0]);

        cJSON *json = cJSON_Parse(expr.c_str());
        /*
        Plan:
        Create dictionary from cJSON object
        Return dictionary
        */

        //Following is an example dictionary that we will return
        Dict d(vm);
        d.set(py_var(vm, "x"), py_var(vm, 1));
        d.set(py_var(vm, "y"), py_var(vm, "123"));
        return VAR(d);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        /*
        Plan:
        Create cJSON object from dictionary
        Return string
        */
        const Dict& dict = CAST(Dict&, args[0]);

        char* out = "This will be created from the dictionary";
        return VAR(Str(out));
        
    });
}

}   // namespace pkpy
