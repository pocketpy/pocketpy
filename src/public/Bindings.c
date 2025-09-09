#include "pocketpy/interpreter/vm.h"

void py_bind(py_Ref obj, const char* sig, py_CFunction f) {
    py_Ref tmp = py_pushtmp();
    py_Name name = py_newfunction(tmp, sig, f, NULL, 0);
    py_setdict(obj, name, tmp);
    py_pop();
}

void py_bindmethod(py_Type type, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bindstaticmethod(py_Type type, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    bool ok = py_tpcall(tp_staticmethod, 1, &tmp);
    if(!ok) {
        py_printexc();
        c11__abort("py_bindstaticmethod(): failed to create staticmethod");
    }
    py_setdict(py_tpobject(type), py_name(name), py_retval());
}

void py_bindfunc(py_Ref obj, const char* name, py_CFunction f) {
    py_TValue tmp;
    py_newnativefunc(&tmp, f);
    py_setdict(obj, py_name(name), &tmp);
}

void py_bindproperty(py_Type type, const char* name, py_CFunction getter, py_CFunction setter) {
    py_TValue tmp;
    py_newobject(&tmp, tp_property, 2, 0);
    py_newnativefunc(py_getslot(&tmp, 0), getter);
    if(setter) {
        py_newnativefunc(py_getslot(&tmp, 1), setter);
    } else {
        py_setslot(&tmp, 1, py_None());
    }
    py_setdict(py_tpobject(type), py_name(name), &tmp);
}

void py_bindmagic(py_Type type, py_Name name, py_CFunction f) {
    py_Ref tmp = py_emplacedict(py_tpobject(type), name);
    py_newnativefunc(tmp, f);
}

void py_macrobind(const char* sig, py_CFunction f) {
    py_Ref tmp = py_pushtmp();
    py_Name name = py_newfunction(tmp, sig, f, NULL, 0);
    NameDict__set(&pk_current_vm->compile_time_funcs, name, tmp);
    py_pop();
}

py_ItemRef py_macroget(py_Name name) {
    NameDict* d = &pk_current_vm->compile_time_funcs;
    if(d->length == 0) return NULL;
    return NameDict__try_get(d, name);
}
