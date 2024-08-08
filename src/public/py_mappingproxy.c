#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"


void pk_mappingproxy__namedict(py_Ref out, py_Ref object){
    py_newobject(py_retval(), tp_namedict, 1, 0);
    assert(object->is_ptr && object->_obj->slots == -1);
    py_setslot(py_retval(), 0, object);
}

static bool namedict__getitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = py_getdict(py_getslot(argv, 0), name);
    if(!res) return KeyError(py_arg(1));
    py_assign(py_retval(), res);
    return true;
}

static bool namedict__setitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_setdict(py_getslot(argv, 0), name, py_arg(2));
    py_newnone(py_retval());
    return true;
}

static bool namedict__delitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    if(!py_deldict(py_getslot(argv, 0), name)) return KeyError(py_arg(1));
    py_newnone(py_retval());
    return true;
}

static bool namedict__contains__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = py_getdict(py_getslot(argv, 0), name);
    py_newbool(py_retval(), res != NULL);
    return true;
}

py_Type pk_namedict__register() {
    py_Type type = pk_newtype("namedict", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __getitem__, namedict__getitem__);
    py_bindmagic(type, __setitem__, namedict__setitem__);
    py_bindmagic(type, __delitem__, namedict__delitem__);
    py_bindmagic(type, __contains__, namedict__contains__);
    return type;
}

//////////////////////

void pk_mappingproxy__locals(py_Ref out, Frame* frame){
    assert(frame->has_function && !frame->is_dynamic);
    Frame** ud = py_newobject(py_retval(), tp_locals, 0, sizeof(Frame*));
    *ud = frame;
}

static bool locals__getitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    Frame** ud = py_touserdata(argv);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref slot = Frame__f_locals_try_get(*ud, name);
    if(!slot || py_isnil(slot)) return KeyError(py_arg(1));
    py_assign(py_retval(), slot);
    return true;
}

static bool locals__setitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    Frame** ud = py_touserdata(argv);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref slot = Frame__f_locals_try_get(*ud, name);
    if(!slot) return KeyError(py_arg(1));
    py_assign(slot, py_arg(2));
    py_newnone(py_retval());
    return true;
}

static bool locals__delitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    Frame** ud = py_touserdata(argv);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref res = Frame__f_locals_try_get(*ud, name);
    if(!res || py_isnil(res)) return KeyError(py_arg(1));
    py_newnil(res);
    py_newnone(py_retval());
    return true;
}

static bool locals__contains__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    Frame** ud = py_touserdata(argv);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_Ref slot = Frame__f_locals_try_get(*ud, name);
    py_newbool(py_retval(), slot && !py_isnil(slot));
    return true;
}

py_Type pk_locals__register() {
    py_Type type = pk_newtype("locals", tp_locals, NULL, NULL, false, true);

    py_bindmagic(type, __getitem__, locals__getitem__);
    py_bindmagic(type, __setitem__, locals__setitem__);
    py_bindmagic(type, __delitem__, locals__delitem__);
    py_bindmagic(type, __contains__, locals__contains__);
    return type;
}