#include "cJSONw.hpp"

namespace pkpy{


static cJSON* convert_python_object_to_cjson(PyObject* obj, VM* vm);
static PyObject* convert_cjson_to_python_object(const cJSON * const item, VM* vm);

template<typename T>
static cJSON* convert_list_to_cjson(const T& list, VM* vm){
    cJSON *cjson_list = cJSON_CreateArray();
    for(auto& element : list){
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
    if(obj == vm->None) return cJSON_CreateNull();
    Type obj_t = vm->_tp(obj);
    switch(obj_t){
        case VM::tp_int.index: cJSON_CreateNumber(_CAST(i64, obj));
        case VM::tp_float.index: cJSON_CreateNumber(_CAST(f64, obj));
        case VM::tp_bool.index: cJSON_CreateBool(obj == vm->True);
        case VM::tp_str.index: cJSON_CreateString(_CAST(Str&, obj).c_str());
        case VM::tp_dict.index: return covert_dict_to_cjson(_CAST(Dict&, obj), vm);
        case VM::tp_list.index: return convert_list_to_cjson<List>(_CAST(List&, obj), vm);
        case VM::tp_tuple.index: return convert_list_to_cjson<Tuple>(_CAST(Tuple&, obj), vm);
        default: break;
    }
    vm->TypeError(_S("unrecognized type ", _type_name(vm, obj_t).escape()));
    PK_UNREACHABLE()
}


static PyObject* convert_cjson_to_list(const cJSON * const item, VM* vm){
    List output;
    cJSON *element = item->child;
    while(element != NULL){
        output.push_back(convert_cjson_to_python_object(element, vm));
        element = element->next;
    }
    return VAR(std::move(output));
}

static PyObject* convert_cjson_to_dict(const cJSON* const item, VM* vm){
    Dict output(vm);
    cJSON *child = item->child;
    while(child != NULL){
        const char* key = child->string;
        const cJSON *child_value = cJSON_GetObjectItemCaseSensitive(item, key);
        output.set(VAR(key), convert_cjson_to_python_object(child_value, vm));
        child = child->next;
    }
    return VAR(std::move(output));
}

static PyObject* convert_cjson_to_python_object(const cJSON * const item, VM* vm)
{
    if (cJSON_IsString(item))
    {
        return VAR(Str(item->valuestring));
    }
    else if (cJSON_IsNumber(item)){
        if(item->valuedouble != item->valueint){
            return VAR(item->valuedouble);
        }
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

    PK_LOCAL_STATIC cJSON_Hooks hooks;
    hooks.malloc_fn = pool64_alloc;
    hooks.free_fn = pool64_dealloc;
    cJSON_InitHooks(&hooks);

    vm->bind_func<1>(mod, "loads", [](VM* vm, ArgsView args){
        std::string_view sv;
        if(is_type(args[0], vm->tp_bytes)){
            sv = PK_OBJ_GET(Bytes, args[0]).sv();
        }else{
            sv = CAST(Str&, args[0]).sv();
        }
        cJSON *json = cJSON_ParseWithLength(sv.data(), sv.size());
        if(json == NULL){
            const char* start = cJSON_GetErrorPtr();
            const char* end = start;
            while(*end != '\0' && *end != '\n') end++;
            vm->IOError(_S("cjson: ", std::string_view(start, end-start)));
        }
        PyObject* output = convert_cjson_to_python_object(json, vm);
        cJSON_Delete(json);
        return output;
    });

    vm->bind_func<1>(mod, "dumps", [](VM* vm, ArgsView args) {
        return vm->py_json(args[0]);
        // cJSON* cjson = convert_python_object_to_cjson(args[0], vm);
        // char* str = cJSON_Print(cjson);
        // cJSON_Delete(cjson);
        // PyObject* ret = VAR((const char*)str);
        // hooks.free_fn(str);
        // return ret;
    });
}

}   // namespace pkpy
