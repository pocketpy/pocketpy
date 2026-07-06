#include "pocketpy/pocketpy.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/_generated.h"

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

// Returns a tuple of (name, kind[, default]) entries.
static bool inspect__signature_data(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    if(!py_istype(argv, tp_function)) {
        return ValueError("no signature found for '%t' object", argv->type);
    }
    Function* fn = py_touserdata(argv);
    FuncDecl* decl = fn->decl;
    const CodeObject* co = &decl->code;

    bool has_starred_arg = decl->starred_arg != -1;
    bool has_starred_kwarg = decl->starred_kwarg != -1;
    int total = decl->args.length + decl->kwargs.length + (int)has_starred_arg +
                (int)has_starred_kwarg;

    py_TValue* items = py_newtuple(py_retval(), total);
    int j = 0;
    for(int i = 0; i < decl->args.length; i++) {
        int32_t index = c11__getitem(int32_t, &decl->args, i);
        py_Name name = c11__getitem(py_Name, &co->varnames, index);
        py_TValue* entry = py_newtuple(&items[j++], 2);
        py_newstr(&entry[0], py_name2str(name));
        py_newint(&entry[1], 1);
    }
    if(has_starred_arg) {
        py_Name name = c11__getitem(py_Name, &co->varnames, decl->starred_arg);
        py_TValue* entry = py_newtuple(&items[j++], 2);
        py_newstr(&entry[0], py_name2str(name));
        py_newint(&entry[1], 2);
    }
    for(int i = 0; i < decl->kwargs.length; i++) {
        FuncDeclKwArg kv = c11__getitem(FuncDeclKwArg, &decl->kwargs, i);
        py_TValue* entry = py_newtuple(&items[j++], 3);
        py_newstr(&entry[0], py_name2str(kv.key));
        // defaults after *args can only be passed by keyword
        py_newint(&entry[1], has_starred_arg ? 3 : 1);
        entry[2] = kv.value;
    }
    if(has_starred_kwarg) {
        py_Name name = c11__getitem(py_Name, &co->varnames, decl->starred_kwarg);
        py_TValue* entry = py_newtuple(&items[j++], 2);
        py_newstr(&entry[0], py_name2str(name));
        py_newint(&entry[1], 4);
    }
    return true;
}

void pk__add_module_inspect() {
    py_Ref mod = py_newmodule("inspect");

    py_bindfunc(mod, "isgeneratorfunction", inspect_isgeneratorfunction);
    py_bindfunc(mod, "is_user_defined_type", inspect_is_user_defined_type);
    py_bindfunc(mod, "_signature_data", inspect__signature_data);

    if(!py_exec(kPythonLibs_inspect, "inspect.py", EXEC_MODE, mod)) {
        py_printexc();
        c11__abort("failed to execute inspect.py");
    }
}
