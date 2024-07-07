#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

bool py_isidentical(const py_Ref lhs, const py_Ref rhs) {
    if(lhs->type != rhs->type) return false;
    switch(lhs->type) {
        case tp_int: return lhs->_i64 == rhs->_i64;
        case tp_float: return lhs->_f64 == rhs->_f64;
        case tp_bool: return lhs->_bool == rhs->_bool;
        case tp_nativefunc: return lhs->_cfunc == rhs->_cfunc;
        case tp_none_type: return true;
        case tp_not_implemented_type: return true;
        case tp_ellipsis: return true;
        // fallback to pointer comparison
        default: return lhs->is_ptr && rhs->is_ptr && lhs->_obj == rhs->_obj;
    }
}

int py_bool(const py_Ref val) {
    switch(val->type) {
        case tp_bool: return val->_bool;
        case tp_int: return val->_i64 != 0;
        case tp_float: return val->_f64 != 0;
        case tp_none_type: return 0;
        default: {
            py_Ref tmp = py_tpfindmagic(val->type, __bool__);
            if(tmp) {
                bool ok = py_call(tmp, 1, val);
                if(!ok) return -1;
                return py_tobool(py_retval());
            } else {
                tmp = py_tpfindmagic(val->type, __len__);
                if(tmp) {
                    bool ok = py_call(tmp, 1, val);
                    if(!ok) return -1;
                    return py_toint(py_retval());
                } else {
                    return 1;  // True
                }
            }
        }
    }
}

bool py_hash(const py_Ref val, int64_t* out) {
    py_Type t = val->type;
    pk_TypeInfo* types = (pk_TypeInfo*)pk_current_vm->types.data;
    do {
        py_Ref _hash = &types[t].magic[__hash__];
        py_Ref _eq = &types[t].magic[__eq__];
        if(!py_isnil(_hash) && !py_isnil(_eq)) {
            bool ok = py_call(_hash, 1, val);
            if(!ok) return false;
            *out = py_toint(py_retval());
            return true;
        }
        t = types[t].base;
    } while(t);
    return TypeError("unhashable type: '%t'", val->type);
}

bool py_iter(const py_Ref val) {
    py_Ref tmp = py_tpfindmagic(val->type, __iter__);
    if(!tmp) return TypeError("'%t' object is not iterable", val->type);
    return py_call(tmp, 1, val);
}

int py_next(const py_Ref val) {
    pk_VM* vm = pk_current_vm;
    vm->is_stopiteration = false;
    py_Ref tmp = py_tpfindmagic(val->type, __next__);
    if(!tmp) return TypeError("'%t' object is not an iterator", val->type);
    bool ok = py_call(tmp, 1, val);
    if(ok) return true;
    return vm->is_stopiteration ? 0 : -1;
}

int py_getattr(const py_Ref self, py_Name name, py_Ref out) { return -1; }

bool py_setattr(py_Ref self, py_Name name, const py_Ref val) { return false; }

bool py_delattr(py_Ref self, py_Name name) { return false; }

bool py_getitem(const py_Ref self, const py_Ref key, py_Ref out) { return -1; }

bool py_setitem(py_Ref self, const py_Ref key, const py_Ref val) { return -1; }

bool py_delitem(py_Ref self, const py_Ref key) { return -1; }

#define COMPARE_OP_IMPL(name, op, rop)                                                             \
    int py_##name(const py_Ref lhs, const py_Ref rhs) {                                            \
        bool ok = py_binaryop(lhs, rhs, op, rop);                                                  \
        if(!ok) return -1;                                                                         \
        return py_tobool(py_retval());                                                             \
    }

COMPARE_OP_IMPL(eq, __eq__, __eq__)
COMPARE_OP_IMPL(ne, __ne__, __ne__)
COMPARE_OP_IMPL(lt, __lt__, __gt__)
COMPARE_OP_IMPL(le, __le__, __ge__)
COMPARE_OP_IMPL(gt, __gt__, __lt__)
COMPARE_OP_IMPL(ge, __ge__, __le__)