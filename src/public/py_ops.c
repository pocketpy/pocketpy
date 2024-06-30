#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"

int py_eq(const py_Ref lhs, const py_Ref rhs) { return 0; }

int py_le(const py_Ref lhs, const py_Ref rhs) { return 0; }

bool py_hash(const py_Ref val, int64_t* out) { return 0; }

bool py_str(const py_Ref val, py_Ref out) { return 0; }

bool py_repr(const py_Ref val, py_Ref out) {
    const pk_TypeInfo* ti = pk_tpinfo(val);
    if(ti->m__repr__) return ti->m__repr__(1, val, out);
    bool ok = py_callmethod(val, __repr__);
    if(ok) {
        *out = pk_current_vm->last_retval;
        return true;
    }
    return false;
}

bool py_getattr(const py_Ref self, py_Name name, py_Ref out) { return true; }

bool py_setattr(py_Ref self, py_Name name, const py_Ref val) { return -1; }

bool py_delattr(py_Ref self, py_Name name) { return -1; }
