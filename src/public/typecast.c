#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

PK_INLINE bool py_istype(py_Ref self, py_Type type) { return self->type == type; }

bool py_checktype(py_Ref self, py_Type type) {
    if(self->type == type) return true;
    return TypeError("expected '%t', got '%t'", type, self->type);
}

bool py_checkinstance(py_Ref self, py_Type type) {
    if(py_isinstance(self, type)) return true;
    return TypeError("expected '%t' or its subclass, got '%t'", type, self->type);
}

bool py_isinstance(py_Ref obj, py_Type type) { return py_issubclass(obj->type, type); }

bool py_issubclass(py_Type derived, py_Type base) {
    assert(derived != 0 && base != 0);
    py_TypeInfo* derived_ti = pk_typeinfo(derived);
    py_TypeInfo* base_ti = pk_typeinfo(base);
    do {
        if(derived_ti == base_ti) return true;
        derived_ti = derived_ti->base_ti;
    } while(derived_ti);
    return false;
}

py_Type py_typeof(py_Ref self) { return self->type; }

py_Type py_gettype(const char* module, py_Name name) {
    py_Ref mod;
    if(module != NULL) {
        mod = py_getmodule(module);
        if(!mod) return tp_nil;
    } else {
        mod = pk_current_vm->builtins;
    }
    py_Ref object = py_getdict(mod, name);
    if(object && py_istype(object, tp_type)) return py_totype(object);
    return tp_nil;
}