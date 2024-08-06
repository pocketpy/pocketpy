#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

/* staticmethod */

static bool staticmethod__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_newobject(py_retval(), tp_staticmethod, 1, 0);
    py_setslot(py_retval(), 0, py_arg(1));
    return true;
}

py_Type pk_staticmethod__register(){
    py_Type type = pk_newtype("staticmethod", tp_object, NULL, NULL, false, true);
    
    py_bindmagic(type, __new__, staticmethod__new__);
    return type;
}

/* classmethod */
static bool classmethod__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_newobject(py_retval(), tp_classmethod, 1, 0);
    py_setslot(py_retval(), 0, py_arg(1));
    return true;
}

py_Type pk_classmethod__register(){
    py_Type type = pk_newtype("classmethod", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, classmethod__new__);
    return type;
}

/* boundmethod */
static bool boundmethod__self__getter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_assign(py_retval(), py_getslot(argv, 0));
    return true;
}

static bool boundmethod__func__getter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_assign(py_retval(), py_getslot(argv, 1));
    return true;
}

py_Type pk_boundmethod__register(){
    py_Type type = pk_newtype("boundmethod", tp_object, NULL, NULL, false, true);

    py_bindproperty(type, "__self__", boundmethod__self__getter, NULL);
    py_bindproperty(type, "__func__", boundmethod__func__getter, NULL);
    return type;
}