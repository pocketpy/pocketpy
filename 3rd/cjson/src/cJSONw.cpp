#include "cJSONw.hpp"

namespace pkpy{


static std::string convert_python_object_to_string(PyObject* obj, VM* vm);
static PyObject* convert_cjson_to_python_object(const cJSON * const item, VM* vm);


static std::string convert_list_to_string(const List& list, VM* vm){
    bool extra_comma_flag = false;
    std::string output = "[";
    for(auto& element : list){
        output = output + convert_python_object_to_string(element, vm) + ", ";
        extra_comma_flag = true;
    }
    if(extra_comma_flag){
        output.pop_back();
        output.pop_back();
    }
    return output + "]";
}

static std::string convert_tuple_to_string(const Tuple& tuple, VM* vm){
    bool extra_comma_flag = false;
    std::string output = "[";

    for(auto& element : tuple){
        output = output + convert_python_object_to_string(element, vm) + ", ";
        extra_comma_flag = true;
    }
    if (extra_comma_flag){
        output.pop_back();
        output.pop_back();
    }
    return output + "]";
}

static std::string covert_dict_to_string(const Dict& dict, VM* vm){
    std::string output = "{";
    dict.apply([&](PyObject* key, PyObject* val){
        output = output + "\"" + CAST(Str&, key).c_str() + "\":";
        output = output + convert_python_object_to_string(val, vm) + ",";
    });
    output.pop_back();
    return output + "}";
}

static std::string convert_python_object_to_string(PyObject* obj, VM* vm){
    if (is_type(obj, vm->tp_int)){
        return std::to_string(CAST(i64, obj));
    }
    else if (is_type(obj, vm->tp_float)){
        return std::to_string(CAST(f64, obj));
    }
    else if (is_type(obj, vm->tp_bool)){
        if (obj == vm->True){
            return "true";
        }
        else{
            return "false";
        }
    }
    else if (is_type(obj, vm->tp_str)){
        std::string str = CAST(Str&, obj).replace("\n", "\\n").replace("\"", "\\\"").c_str();
        return std::string("\"") + str + std::string("\"");
    }
    else if (is_type(obj, vm->tp_dict)){
        return covert_dict_to_string(CAST(Dict&, obj), vm);
       
    }
    else if (is_type(obj, vm->tp_list)){
        return convert_list_to_string(CAST(List&, obj), vm);
    }
    else if(is_type(obj, vm->tp_tuple)){
        return convert_tuple_to_string(CAST(Tuple&, obj), vm);
    }
    return "null";
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

        cJSON *json = cJSON_Parse(expr.c_str());

        PyObject* output = convert_cjson_to_python_object(json, vm);
        cJSON_Delete(json);
        return output;
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        
        std::string str = convert_python_object_to_string(args[0], vm);
        return VAR(str);

    });
}

}   // namespace pkpy
