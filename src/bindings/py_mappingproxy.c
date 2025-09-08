#include "pocketpy/pocketpy.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include <stdbool.h>

void pk_mappingproxy__namedict(py_Ref out, py_Ref object) {
    py_newobject(out, tp_namedict, 1, 0);
    assert(object->is_ptr && object->_obj->slots == -1);
    py_setslot(out, 0, object);
}

static bool namedict__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = py_getdict(py_getslot(argv, 0), name);
    if(!res) return KeyError(py_arg(1));
    py_assign(py_retval(), res);
    return true;
}

static bool namedict__get(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = py_getdict(py_getslot(argv, 0), name);
    py_assign(py_retval(), res ? res : py_arg(2));
    return true;
}

static bool namedict__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_setdict(py_getslot(argv, 0), name, py_arg(2));
    py_newnone(py_retval());
    return true;
}

static bool namedict__delitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    if(!py_deldict(py_getslot(argv, 0), name)) return KeyError(py_arg(1));
    py_newnone(py_retval());
    return true;
}

static bool namedict__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = py_getdict(py_getslot(argv, 0), name);
    py_newbool(py_retval(), res != NULL);
    return true;
}

static bool namedict_items(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_Ref object = py_getslot(argv, 0);
    NameDict* dict = PyObject__dict(object->_obj);
    py_newlist(py_retval());
    for(int i = 0; i < dict->capacity; i++) {
        NameDict_KV* kv = &dict->items[i];
        if(kv->key == NULL) continue;
        py_Ref slot = py_list_emplace(py_retval());
        py_Ref p = py_newtuple(slot, 2);
        p[0] = *py_name2ref(kv->key);
        p[1] = kv->value;
    }
    return true;
}

static bool namedict_clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_Ref object = py_getslot(argv, 0);
    NameDict* dict = PyObject__dict(object->_obj);
    NameDict__clear(dict);
    py_newnone(py_retval());
    return true;
}

py_Type pk_namedict__register() {
    py_Type type = pk_newtype("namedict", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __getitem__, namedict__getitem__);
    py_bindmagic(type, __setitem__, namedict__setitem__);
    py_bindmagic(type, __delitem__, namedict__delitem__);
    py_bindmagic(type, __contains__, namedict__contains__);
    py_setdict(py_tpobject(type), __hash__, py_None());
    py_bindmethod(type, "items", namedict_items);
    py_bindmethod(type, "clear", namedict_clear);
    py_bindmethod(type, "get", namedict__get);
    return type;
}
