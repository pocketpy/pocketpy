#include "pocketpy.h"

static bool hello_add(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    return py_binaryadd(py_arg(0), py_arg(1));
}

bool py_module_initialize(){
    py_GlobalRef mod = py_newmodule("hello");

    py_bindfunc(mod, "add", hello_add);

    py_assign(py_retval(), mod);
    return true;
}