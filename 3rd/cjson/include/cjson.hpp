#include "cjson/cJSON.h"
#include "pocketpy/pocketpy.h"


namespace pkpy{

static Dict convert_cjson_to_dict(const cJSON * const item, VM* vm)
{

    Dict output(vm);
    List list;

    if ((item == NULL))
    {
        return NULL;
    }

    if (((item->type) & 0xFF) == cJSON_Object){
            cJSON *child = item->child;
            while(child != NULL){
                const cJSON *child_value = cJSON_GetObjectItemCaseSensitive(item, child->string);
           
                if (cJSON_IsString(child_value))
                {
                    output.set(VAR(Str(child->string)), VAR(Str(child_value->valuestring)));
                }
                else if (cJSON_IsNumber(child_value)){
                    output.set(VAR(Str(child->string)), VAR(child_value->valueint));
                }
                else if (cJSON_IsBool(child_value)){
                    output.set(VAR(Str(child->string)), VAR(child_value->valueint == 0?false:true));
                }
                else if (cJSON_IsNull(child_value)){
                    output.set(VAR(Str(child->string)), VAR(vm->None)); //Todo: Covert to python None
                }
                else if (cJSON_IsArray(child_value)){
                    cJSON *array_child = child_value->child;
                    while(array_child != NULL){
                        if (cJSON_IsString(array_child))
                        {
                            list.push_back(VAR(Str(array_child->valuestring)));
                        }
                        else if (cJSON_IsNumber(array_child)){
                            list.push_back(VAR(array_child->valueint));
                        }
                        else if (cJSON_IsBool(array_child)){
                            list.push_back(VAR(array_child->valueint == 0?false:true));
                        }
                        else if (cJSON_IsNull(array_child)){
                            list.push_back(VAR(vm->None));
                        }
                        array_child = array_child->next;
                    }
                    output.set(VAR(Str(child->string)), VAR(list));
                }
                else if (cJSON_IsObject(child_value)){
                    Dict child_object = convert_cjson_to_dict(child_value, vm);
                    output.set(VAR(Str(child->string)), VAR(child_object));
                }
                child = child->next;
            }

    }
    return output;
}

inline void add_module_cjson(VM* vm){
    PyObject* mod = vm->new_module("cjson");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {

        const Str& expr = CAST(Str&, args[0]);
        cJSON *json = cJSON_Parse(expr.c_str());

        Dict output = convert_cjson_to_dict(json, vm);
        return VAR(output);
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {

        const Dict& dict = CAST(Dict&, args[0]);

        //TODO: Convert dict to cJSON object
        char* out = "This will be created from the dictionary";
        return VAR(Str(out));
        
    });
}

}   // namespace pkpy
