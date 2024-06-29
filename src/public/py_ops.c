#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"

int py_eq(const py_Ref lhs, const py_Ref rhs) { return 0; }

int py_le(const py_Ref lhs, const py_Ref rhs) { return 0; }

int py_hash(const py_Ref val, int64_t* out) { return 0; }

int py_str(const py_Ref val, py_Ref out) { return 0; }

int py_repr(const py_Ref val, py_Ref out) {
    const pk_TypeInfo* ti = c11__at(pk_TypeInfo, &pk_current_vm->types, val->type);
    int err;
    if(ti->m__repr__) err = ti->m__repr__(1, val);
    err = py_callmethod(val, __repr__);
    if(err) return err;
    if(out) *out = *--pk_current_vm->stack.sp;
    return 0;
}