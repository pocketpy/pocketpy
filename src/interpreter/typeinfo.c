#include "pocketpy/interpreter/vm.h"

py_ItemRef pk_tpfindname(py_TypeInfo* ti, py_Name name) {
    assert(ti != NULL);
    do {
        py_Ref res = py_getdict(&ti->self, name);
        if(res) return res;
        ti = ti->base_ti;
    } while(ti);
    return NULL;
}

py_ItemRef py_tpfindname(py_Type type, py_Name name) {
    py_TypeInfo* ti = pk_typeinfo(type);
    return pk_tpfindname(ti, name);
}

py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    // assert(py_ismagicname(name));
    return py_tpfindname(t, name);
}

py_Type py_tpbase(py_Type t) {
    assert(t);
    py_TypeInfo* ti = pk_typeinfo(t);
    return ti->base;
}

PK_DEPRECATED py_Ref py_tpgetmagic(py_Type type, py_Name name) {
    // assert(py_ismagicname(name));
    py_TypeInfo* ti = pk_typeinfo(type);
    py_Ref retval = py_getdict(&ti->self, name);
    return retval != NULL ? retval : py_NIL();
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

py_TypeInfo* pk_typeinfo(py_Type type) {
    return c11__getitem(TypePointer, &pk_current_vm->types, type).ti;
}