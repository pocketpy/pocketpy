#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

static bool inspect_isgeneratorfunction(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_Ref obj = argv;
    if(py_istype(argv, tp_boundmethod)) {
        py_TValue* slots = PyObject__slots(argv->_obj);
        obj = &slots[1];  // callable
    }
    if(py_istype(obj, tp_function)) {
        Function* fn = py_touserdata(obj);
        py_newbool(py_retval(), fn->decl->type == FuncType_GENERATOR);
    } else {
        py_newbool(py_retval(), false);
    }
    return true;
}

static bool inspect_is_user_defined_type(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_type);
    py_TypeInfo* ti = py_touserdata(argv);
    py_newbool(py_retval(), ti->is_python);
    return true;
}

void pk__add_module_inspect() {
    py_Ref mod = py_newmodule("inspect");

    py_bindfunc(mod, "isgeneratorfunction", inspect_isgeneratorfunction);
    py_bindfunc(mod, "is_user_defined_type", inspect_is_user_defined_type);
}
