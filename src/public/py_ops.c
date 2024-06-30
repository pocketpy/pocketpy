#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"

bool py_isidentical(const py_Ref lhs, const py_Ref rhs){
    if(lhs->is_ptr && rhs->is_ptr){
        return lhs->_obj == rhs->_obj;
    }
    return false;
}

bool py_bool(const py_Ref val) { return 0; }

bool py_hash(const py_Ref val, int64_t* out) { return 0; }

bool py_getattr(const py_Ref self, py_Name name, py_Ref out) { return true; }

bool py_setattr(py_Ref self, py_Name name, const py_Ref val) { return -1; }

bool py_delattr(py_Ref self, py_Name name) { return -1; }

bool py_getitem(const py_Ref self, const py_Ref key, py_Ref out) { return -1; }

bool py_setitem(py_Ref self, const py_Ref key, const py_Ref val) { return -1; }

bool py_delitem(py_Ref self, const py_Ref key) { return -1; }

int py_eq(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __eq__, __eq__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}

int py_ne(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __ne__, __ne__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}

int py_lt(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __lt__, __gt__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}

int py_gt(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __gt__, __lt__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}

int py_ge(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __ge__, __le__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}

int py_le(const py_Ref lhs, const py_Ref rhs) {
    bool ok = py_binaryop(lhs, rhs, __le__, __ge__);
    if(!ok) return -1;
    return py_tobool(py_lastretval());
}
