#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getmodule(const char *name){
    pk_VM* vm = pk_current_vm;
    return pk_NameDict__try_get(&vm->modules, py_name(name));
}

py_Ref py_newmodule(const char *name, const char *package){
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_module, -1, 0);

    py_Ref r0 = py_pushtmp();
    py_Ref r1 = py_pushtmp();

    *r0 = PyVar__fromobj(obj);

    py_newstr(r1, name);
    py_setdict(r0, __name__, r1);

    package = package ? package : "";

    py_newstr(r1, package);
    py_setdict(r0, __package__, r1);

    // convert to fullname
    if(package[0] != '\0'){
        // package.name
        char buf[256];
        snprintf(buf, sizeof(buf), "%s.%s", package, name);
        name = buf;
    }

    py_newstr(r1, name);
    py_setdict(r0, __path__, r1);

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    bool exists = pk_NameDict__contains(&pk_current_vm->modules, py_name(name));
    if(exists) abort();
    pk_NameDict__set(&pk_current_vm->modules, py_name(name), *r0);

    py_poptmp(2);
    return py_getmodule(name);
}
