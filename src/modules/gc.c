#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

static bool gc_collect(int argc, py_Ref argv){
    ManagedHeap* heap = &pk_current_vm->heap;
    int res = ManagedHeap__collect(heap);
    py_newint(py_retval(), res);
    return true;
}

void pk__add_module_gc() {
    py_Ref mod = py_newmodule("gc");

    py_bindfunc(mod, "collect", gc_collect);
}