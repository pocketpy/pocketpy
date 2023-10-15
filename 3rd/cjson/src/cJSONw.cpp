#include "cJSONw.hpp"

namespace pkpy{


static cJSON* convert_python_object_to_cjson(PyObject* obj, VM* vm);
static PyObject* convert_cjson_to_python_object(const cJSON * const item, VM* vm);


static cJSON* convert_list_to_cjson(const List& list, VM* vm){
    cJSON *cjson_list = cJSON_CreateArray();
    for(auto& element : list){
        cJSON_AddItemToArray(cjson_list, convert_python_object_to_cjson(element, vm));
    }
    return cjson_list;
}

static cJSON* convert_tuple_to_cjson(const Tuple& tuple, VM* vm){
    cJSON *cjson_list = cJSON_CreateArray();
    for(auto& element : tuple){
        cJSON_AddItemToArray(cjson_list, convert_python_object_to_cjson(element, vm));
    }
    return cjson_list;
}

static cJSON* covert_dict_to_cjson(const Dict& dict, VM* vm){
    cJSON *cjson_object = cJSON_CreateObject();
    dict.apply([&](PyObject* key, PyObject* val){
        cJSON_AddItemToObject(cjson_object, CAST(Str&, key).c_str(), convert_python_object_to_cjson(val, vm));
    });
    return cjson_object;
}

static cJSON* convert_python_object_to_cjson(PyObject* obj, VM* vm){
    if (is_type(obj, vm->tp_int)){
        return cJSON_CreateNumber(CAST(i64, obj));
    }
    else if (is_type(obj, vm->tp_float)){
        return cJSON_CreateNumber(CAST(f64, obj));
    }
    else if (is_type(obj, vm->tp_bool)){
        return cJSON_CreateBool(obj == vm->True);
    }
    else if (is_type(obj, vm->tp_str)){
        return cJSON_CreateString(CAST(Str&, obj).c_str());
    }
    else if (is_type(obj, vm->tp_dict)){
        return covert_dict_to_cjson(CAST(Dict&, obj), vm);
       
    }
    else if (is_type(obj, vm->tp_list)){
        return convert_list_to_cjson(CAST(List&, obj), vm);
    }
    else if(is_type(obj, vm->tp_tuple)){
        return convert_tuple_to_cjson(CAST(Tuple&, obj), vm);
    }
    return cJSON_CreateNull();
}
static PyObject* convert_cjson_to_list(const cJSON * const item, VM* vm){
    List list;
    cJSON *element = item->child;
    while(element != NULL){
        list.push_back(convert_cjson_to_python_object(element, vm));
        element = element->next;
    }
    return VAR(list);
}

static PyObject* convert_cjson_to_dict(const cJSON* const item, VM* vm){
    Dict output(vm);
    cJSON *child = item->child;
    while(child != NULL){
        const cJSON *child_value = cJSON_GetObjectItemCaseSensitive(item, child->string);
        output.set(VAR(Str(child->string)), convert_cjson_to_python_object(child_value, vm));
        child = child->next;
    }
    return VAR(output);
}
static PyObject* convert_cjson_to_python_object(const cJSON * const item, VM* vm)
{
    if (cJSON_IsString(item))
    {
        return VAR(Str(item->valuestring));
    }
    else if (cJSON_IsNumber(item)){
        return VAR(item->valueint);
    }
    else if (cJSON_IsBool(item)){
        return item->valueint!=0 ? vm->True : vm->False;
    }
    else if (cJSON_IsNull(item)){
        return vm->None;
    }
    else if (cJSON_IsArray(item)){
        return convert_cjson_to_list(item, vm);
    }
    else if (cJSON_IsObject(item)){
        return convert_cjson_to_dict(item, vm);
    }
    return vm->None;
}

void add_module_cjson(VM* vm){
    PyObject* mod = vm->new_module("cjson");
    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args) {

        const Str& expr = CAST(Str&, args[0]);

        cJSON *json = cJSON_ParseWithLength(expr.data, expr.size);

        PyObject* output = convert_cjson_to_python_object(json, vm);
        cJSON_Delete(json);
        return output;
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        
        cJSON* cjson = convert_python_object_to_cjson(args[0], vm);
        char* str = cJSON_Print(cjson);
        cJSON_Delete(cjson);
        return VAR(Str(str));
    });
}

}   // namespace pkpy
