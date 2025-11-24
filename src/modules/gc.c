#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

static bool gc_enable(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    ManagedHeap* heap = &pk_current_vm->heap;
    heap->gc_enabled = true;
    py_newnone(py_retval());
    return true;
}

static bool gc_disable(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    ManagedHeap* heap = &pk_current_vm->heap;
    heap->gc_enabled = false;
    py_newnone(py_retval());
    return true;
}

static bool gc_isenabled(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    ManagedHeap* heap = &pk_current_vm->heap;
    py_newbool(py_retval(), heap->gc_enabled);
    return true;
}

static bool gc_collect(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    ManagedHeap* heap = &pk_current_vm->heap;
    int res = ManagedHeap__collect(heap);
    py_newint(py_retval(), res);
    return true;
}

static bool gc_collect_hint(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    ManagedHeap* heap = &pk_current_vm->heap;
    ManagedHeap__collect_hint(heap);
    py_newnone(py_retval());
    return true;
}

static bool gc_setup_debug_callback(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    ManagedHeap* heap = &pk_current_vm->heap;
    heap->debug_callback = *argv;
    py_newnone(py_retval());
    return true;
}

void pk__add_module_gc() {
    py_Ref mod = py_newmodule("gc");

    py_bindfunc(mod, "enable", gc_enable);
    py_bindfunc(mod, "disable", gc_disable);
    py_bindfunc(mod, "isenabled", gc_isenabled);

    py_bindfunc(mod, "collect", gc_collect);
    py_bindfunc(mod, "collect_hint", gc_collect_hint);
    py_bindfunc(mod, "setup_debug_callback", gc_setup_debug_callback);
}
