#include "pocketpy/pocketpy.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include <assert.h>
#include <stdlib.h>

pk_VM* pk_current_vm;
static pk_VM pk_default_vm;

void py_initialize(){
    Pools_initialize();
    pk_StrName__initialize();
    pk_current_vm = &pk_default_vm;
    pk_VM__ctor(&pk_default_vm);
}

int py_exec_simple(const char* source){
    CodeObject* co = NULL;
    pk_VM* vm = pk_current_vm;
    Frame* frame = Frame__new(
        co,
        &vm->main,
        NULL,
        vm->stack.sp,
        vm->stack.sp,
        co
    );
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    if(res == RES_ERROR) return vm->last_error->type;
    if(res == RES_RETURN) return 0; // vm->last_retval;
    assert(0);  // unreachable
}

py_Ref py_getmodule(const char *name){
    pk_VM* vm = pk_current_vm;
    return pk_NameDict__try_get(&vm->modules, py_name(name));
}

py_Ref py_newmodule(const char *name, const char *package){
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__gcnew(heap, tp_module, -1, 0);

    py_Ref r0 = py_sysreg(0);
    py_Ref r1 = py_sysreg(1);

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
    return py_getmodule(name);
}

py_Error* py_getlasterror(){
    return pk_current_vm->last_error;
}

void py_Error__print(py_Error* self){
    abort();
}

py_Ref py_reg(int i){
    assert(i >= 0 && i < 8);
    return &pk_current_vm->reg[i];
}

py_Ref py_sysreg(int i){
    assert(i >= 0 && i < 8);
    return &pk_current_vm->sysreg[i];
}

py_Ref py_stack(int i){
    assert(i < 0);
    return &pk_current_vm->stack.sp[i];
}

void py_finalize(){
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    pk_StrName__finalize();
    Pools_finalize();
}

void py_setdict(py_Ref self, py_Name name, const py_Ref val){
    pk_NameDict__set(
        PyObject__dict(self->_obj),
        name,
        *val
    );
}

void py_newint(py_Ref self, int64_t val){
    self->type = tp_int;
    self->is_ptr = false;
    self->_i64 = val;
}

void py_newfloat(py_Ref self, double val){
    self->type = tp_float;
    self->is_ptr = false;
    self->_f64 = val;
}

void py_newbool(py_Ref self, bool val){
    pk_VM* vm = pk_current_vm;
    *self = val ? vm->True : vm->False;
}

void py_newstr(py_Ref self, const char* val){
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_str, 0, sizeof(py_Str));
    py_Str__ctor((py_Str*)PyObject__value(obj), val);
    self->type = tp_str;
    self->is_ptr = true;
    self->_obj = obj;
}
