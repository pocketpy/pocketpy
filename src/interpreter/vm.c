#include "pocketpy/interpreter/vm.h"

static unsigned char* pk_default_import_file(pk_VM* vm, const char* path){
    return NULL;
}

static void pk_default_stdout(pk_VM* vm, const char* s){
    fprintf(stdout, "%s", s);
}

static void pk_default_stderr(pk_VM* vm, const char* s){
    fprintf(stderr, "%s", s);
}

void pk_TypeInfo__ctor(pk_TypeInfo *self, StrName name, Type base, PyObject* obj, PyObject* module, bool subclass_enabled){
    memset(self, 0, sizeof(pk_TypeInfo));
    
    self->name = name;
    self->base = base;

    self->obj = obj;
    self->module = module;
    self->subclass_enabled = subclass_enabled;

    c11_vector__ctor(&self->annotated_fields, sizeof(StrName));
}

void pk_TypeInfo__dtor(pk_TypeInfo *self){
    c11_vector__dtor(&self->annotated_fields);
}

void pk_VM__ctor(pk_VM* self){
    self->top_frame = NULL;

    pk_NameDict__ctor(&self->modules);
    c11_vector__ctor(&self->types, sizeof(pk_TypeInfo));

    self->StopIteration = NULL;
    self->builtins = NULL;
    self->main = NULL;

    self->_ceval_on_step = NULL;
    self->_import_file = pk_default_import_file;
    self->_stdout = pk_default_stdout;
    self->_stderr = pk_default_stderr;

    self->__last_exception = NULL;
    self->__curr_class = NULL;
    self->__cached_object_new = NULL;
    self->__dynamic_func_decl = NULL;

    pk_ManagedHeap__ctor(&self->heap, self);
    ValueStack__ctor(&self->stack);

    self->True = (PyVar){.type=tp_bool, .is_ptr=true, .extra=1,
        ._obj=pk_ManagedHeap__gcnew(&self->heap, tp_bool, 1),
    };
    self->False = (PyVar){.type=tp_bool, .is_ptr=true, .extra=0,
        ._obj=pk_ManagedHeap__gcnew(&self->heap, tp_bool, 1),
    };
    self->None = (PyVar){.type=tp_none_type, .is_ptr=true,
        ._obj=pk_ManagedHeap__gcnew(&self->heap, tp_none_type, 1),
    };
    self->NotImplemented = (PyVar){.type=tp_not_implemented_type, .is_ptr=true,
        ._obj=pk_ManagedHeap__gcnew(&self->heap, tp_not_implemented_type, 1),
    };
    self->Ellipsis = (PyVar){.type=tp_ellipsis, .is_ptr=true,
        ._obj=pk_ManagedHeap__gcnew(&self->heap, tp_ellipsis, 1),
    };

    /* Init Builtin Types */
    // 0: unused
    pk_TypeInfo__ctor(c11_vector__emplace(&self->types), 0, 0, NULL, NULL);
    #define validate(t, expr) if(t != (expr)) abort()

    validate(tp_object, pk_VM__new_type(self, "object", 0, NULL, true));
    validate(tp_type, pk_VM__new_type(self, "type", 1, NULL, false));

    validate(tp_int, pk_VM__new_type(self, "int", tp_object, NULL, false));
    validate(tp_float, pk_VM__new_type(self, "float", tp_object, NULL, false));
    validate(tp_bool, pk_VM__new_type(self, "bool", tp_object, NULL, false));
    validate(tp_str, pk_VM__new_type(self, "str", tp_object, NULL, false));

    validate(tp_list, pk_VM__new_type(self, "list", tp_object, NULL, false));
    validate(tp_tuple, pk_VM__new_type(self, "tuple", tp_object, NULL, false));

    validate(tp_slice, pk_VM__new_type(self, "slice", tp_object, NULL, false));
    validate(tp_range, pk_VM__new_type(self, "range", tp_object, NULL, false));
    validate(tp_module, pk_VM__new_type(self, "module", tp_object, NULL, false));

    validate(tp_function, pk_VM__new_type(self, "function", tp_object, NULL, false));
    validate(tp_native_func, pk_VM__new_type(self, "native_func", tp_object, NULL, false));
    validate(tp_bound_method, pk_VM__new_type(self, "bound_method", tp_object, NULL, false));

    validate(tp_super, pk_VM__new_type(self, "super", tp_object, NULL, false));
    validate(tp_exception, pk_VM__new_type(self, "Exception", tp_object, NULL, true));
    validate(tp_bytes, pk_VM__new_type(self, "bytes", tp_object, NULL, false));
    validate(tp_mappingproxy, pk_VM__new_type(self, "mappingproxy", tp_object, NULL, false));

    validate(tp_dict, pk_VM__new_type(self, "dict", tp_object, NULL, true));
    validate(tp_property, pk_VM__new_type(self, "property", tp_object, NULL, false));
    validate(tp_star_wrapper, pk_VM__new_type(self, "star_wrapper", tp_object, NULL, false));

    validate(tp_staticmethod, pk_VM__new_type(self, "staticmethod", tp_object, NULL, false));
    validate(tp_classmethod, pk_VM__new_type(self, "classmethod", tp_object, NULL, false));

    validate(tp_none_type, pk_VM__new_type(self, "NoneType", tp_object, NULL, false));
    validate(tp_not_implemented_type, pk_VM__new_type(self, "NotImplementedType", tp_object, NULL, false));
    validate(tp_ellipsis, pk_VM__new_type(self, "ellipsis", tp_object, NULL, false));

    validate(tp_op_call, pk_VM__new_type(self, "__op_call", tp_object, NULL, false));
    validate(tp_op_yield, pk_VM__new_type(self, "__op_yield", tp_object, NULL, false));

    validate(tp_syntax_error, pk_VM__new_type(self, "SyntaxError", tp_exception, NULL, false));
    validate(tp_stop_iteration, pk_VM__new_type(self, "StopIteration", tp_exception, NULL, false));
    #undef validate

    self->StopIteration = c11__at(pk_TypeInfo, &self->types, tp_stop_iteration)->obj;
    self->builtins = pk_VM__new_module(self, "builtins", NULL);
    
    /* Setup Public Builtin Types */
    Type public_types[] = {
        tp_object, tp_type,
        tp_int, tp_float, tp_bool, tp_str,
        tp_list, tp_tuple,
        tp_slice, tp_range,
        tp_bytes, tp_dict, tp_property,
        tp_exception, tp_stop_iteration, tp_syntax_error
    };

    for(int i=0; i<PK_ARRAY_COUNT(public_types); i++){
        Type t = public_types[i];
        pk_TypeInfo* ti = c11__at(pk_TypeInfo, &self->types, t);
        pk_NameDict__set(self->builtins->dict, ti->name, PyVar__fromobj(ti->obj));
    }
    pk_NameDict__set(self->builtins->dict, pk_StrName__map("NotImplemented"), self->NotImplemented);

    /* Do Buildin Bindings*/
    // TODO: ...
    self->main = pk_VM__new_module(self, "__main__", NULL);
}

void pk_VM__dtor(pk_VM* self){
    PK_DECREF(self->__dynamic_func_decl);
    // destroy all objects
    pk_ManagedHeap__dtor(&self->heap);
    // clear frames
    // ...
    pk_NameDict__dtor(&self->modules);
    c11_vector__dtor(&self->types);
    ValueStack__clear(&self->stack);
}

Type pk_VM__new_type(pk_VM* self, const char* name, Type base, PyObject* module, bool subclass_enabled){
    Type type = self->types.count;
    pk_TypeInfo* ti = c11_vector__emplace(&self->types);
    PyObject* typeobj = pk_ManagedHeap__gcnew(&self->heap, tp_type, PK_OBJ_SIZEOF(Type));
    *PyObject__as(Type, typeobj) = type;
    pk_TypeInfo__ctor(ti, pk_StrName__map(name), base, typeobj, module, subclass_enabled);
    return type;
}
