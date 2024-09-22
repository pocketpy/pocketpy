#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

static bool property__new__(int argc, py_Ref argv) {
    py_newobject(py_retval(), tp_property, 2, 0);
    if(argc == 1 + 1) {
        py_setslot(py_retval(), 0, py_arg(1));
        py_setslot(py_retval(), 1, py_None());
    } else if(argc == 1 + 2) {
        py_setslot(py_retval(), 0, py_arg(1));
        py_setslot(py_retval(), 1, py_arg(2));
    } else {
        return TypeError("property() expected 1 or 2 arguments, got %d", argc - 1);
    }
    return true;
}

static bool property_setter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_setslot(argv, 1, py_arg(1));
    py_assign(py_retval(), argv);
    return true;
}

static bool property_fget(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_Ref fget = py_getslot(argv, 0);
    py_assign(py_retval(), fget);
    return true;
}

static bool property_fset(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_Ref fset = py_getslot(argv, 1);
    py_assign(py_retval(), fset);
    return true;
}

py_Type pk_property__register() {
    py_Type type = pk_newtype("property", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, property__new__);
    py_bindmethod(type, "setter", property_setter);

    py_bindproperty(type, "fget", property_fget, NULL);
    py_bindproperty(type, "fset", property_fset, NULL);
    return type;
}
