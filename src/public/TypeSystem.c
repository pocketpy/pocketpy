#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/interpreter/vm.h"

py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, void (*dtor)(void*)) {
    if(strlen(name) == 0) c11__abort("type name cannot be empty");
    py_Type type = pk_newtype(name, base, module, dtor, false, false);
    if(module) py_setdict(module, py_name(name), py_tpobject(type));
    return type;
}

PK_INLINE bool py_istype(py_Ref self, py_Type type) { return self->type == type; }

PK_INLINE py_Type py_typeof(py_Ref self) { return self->type; }

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

bool py_checktype(py_Ref self, py_Type type) {
    if(self->type == type) return true;
    return TypeError("expected '%t', got '%t'", type, self->type);
}

bool py_checkinstance(py_Ref self, py_Type type) {
    if(py_isinstance(self, type)) return true;
    return TypeError("expected '%t' or its subclass, got '%t'", type, self->type);
}

PK_DEPRECATED py_Ref py_tpgetmagic(py_Type type, py_Name name) {
    // assert(py_ismagicname(name));
    py_TypeInfo* ti = pk_typeinfo(type);
    py_Ref retval = py_getdict(&ti->self, name);
    return retval != NULL ? retval : py_NIL();
}

PK_INLINE py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    // assert(py_ismagicname(name));
    return py_tpfindname(t, name);
}

PK_INLINE py_ItemRef py_tpfindname(py_Type type, py_Name name) {
    py_TypeInfo* ti = pk_typeinfo(type);
    return pk_tpfindname(ti, name);
}

PK_INLINE py_Type py_tpbase(py_Type t) {
    assert(t);
    py_TypeInfo* ti = pk_typeinfo(t);
    return ti->base;
}

py_Ref py_tpobject(py_Type type) {
    assert(type);
    return &pk_typeinfo(type)->self;
}

const char* py_tpname(py_Type type) {
    if(!type) return "nil";
    py_Name name = pk_typeinfo(type)->name;
    return py_name2str(name);
}

void py_tpsetfinal(py_Type type) {
    assert(type);
    py_TypeInfo* ti = pk_typeinfo(type);
    ti->is_final = true;
}

void py_tphookattributes(py_Type type,
                         bool (*getattribute)(py_Ref self, py_Name name),
                         bool (*setattribute)(py_Ref self, py_Name name, py_Ref val),
                         bool (*delattribute)(py_Ref self, py_Name name),
                         bool (*getunboundmethod)(py_Ref self, py_Name name)) {
    assert(type);
    py_TypeInfo* ti = pk_typeinfo(type);
    ti->getattribute = getattribute;
    ti->setattribute = setattribute;
    ti->delattribute = delattribute;
    ti->getunboundmethod = getunboundmethod;
}
