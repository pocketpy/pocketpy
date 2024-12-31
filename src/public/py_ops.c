#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

bool py_isidentical(py_Ref lhs, py_Ref rhs) {
    if(lhs->type != rhs->type) return false;
    switch(lhs->type) {
        case tp_int: return lhs->_i64 == rhs->_i64;
        case tp_float: return lhs->_f64 == rhs->_f64;
        case tp_bool: return lhs->_bool == rhs->_bool;
        case tp_nativefunc: return lhs->_cfunc == rhs->_cfunc;
        case tp_NoneType: return true;
        case tp_NotImplementedType: return true;
        case tp_ellipsis: return true;
        // fallback to pointer comparison
        default: return lhs->is_ptr && rhs->is_ptr && lhs->_obj == rhs->_obj;
    }
}

int py_bool(py_Ref val) {
    switch(val->type) {
        case tp_bool: return val->_bool;
        case tp_int: return val->_i64 != 0;
        case tp_float: return val->_f64 != 0;
        case tp_NoneType: return 0;
        default: {
            py_Ref tmp = py_tpfindmagic(val->type, __bool__);
            if(tmp) {
                if(!py_call(tmp, 1, val)) return -1;
                if(!py_checkbool(py_retval())) return -1;
                return py_tobool(py_retval());
            } else {
                tmp = py_tpfindmagic(val->type, __len__);
                if(tmp) {
                    if(!py_call(tmp, 1, val)) return -1;
                    if(!py_checkint(py_retval())) return -1;
                    return py_toint(py_retval());
                } else {
                    return 1;  // True
                }
            }
        }
    }
}

bool py_hash(py_Ref val, int64_t* out) {
    py_TypeInfo* ti = pk__type_info(val->type);
    do {
        py_Ref slot_hash = TypeList__magic_common(ti, __hash__);
        if(py_isnone(slot_hash)) break;
        py_Ref slot_eq = TypeList__magic_common(ti, __eq__);
        if(!py_isnil(slot_eq)) {
            if(py_isnil(slot_hash)) break;
            if(!py_call(slot_hash, 1, val)) return false;
            if(!py_checkint(py_retval())) return false;
            *out = py_toint(py_retval());
            return true;
        }
        ti = ti->base_ti;
    } while(ti);
    return TypeError("unhashable type: '%t'", val->type);
}

bool py_iter(py_Ref val) {
    py_Ref tmp = py_tpfindmagic(val->type, __iter__);
    if(!tmp) return TypeError("'%t' object is not iterable", val->type);
    return py_call(tmp, 1, val);
}

int py_next(py_Ref val) {
    VM* vm = pk_current_vm;
    py_Ref tmp = py_tpfindmagic(val->type, __next__);
    if(!tmp) {
        TypeError("'%t' object is not an iterator", val->type);
        return -1;
    }
    if(py_call(tmp, 1, val)) return 1;
    if(vm->curr_exception.type == tp_StopIteration) {
        vm->last_retval = vm->curr_exception;
        py_clearexc(NULL);
        return 0;
    }
    return -1;
}

bool py_getattr(py_Ref self, py_Name name) {
    // https://docs.python.org/3/howto/descriptor.html#invocation-from-an-instance
    py_Type type = self->type;
    py_Ref cls_var = py_tpfindname(type, name);
    if(cls_var) {
        // handle descriptor
        if(py_istype(cls_var, tp_property)) {
            py_Ref getter = py_getslot(cls_var, 0);
            return py_call(getter, 1, self);
        }
    }
    // handle instance __dict__
    if(self->is_ptr && self->_obj->slots == -1) {
        if(!py_istype(self, tp_type)) {
            py_Ref res = py_getdict(self, name);
            if(res) {
                py_assign(py_retval(), res);
                return true;
            }
        } else {
            py_Type* inner_type = py_touserdata(self);
            py_Ref res = py_tpfindname(*inner_type, name);
            if(res) {
                if(py_istype(res, tp_staticmethod)) {
                    res = py_getslot(res, 0);
                } else if(py_istype(res, tp_classmethod)) {
                    res = py_getslot(res, 0);
                    py_newboundmethod(py_retval(), self, res);
                    return true;
                }
                py_assign(py_retval(), res);
                return true;
            }
        }
    }

    if(cls_var) {
        // bound method is non-data descriptor
        switch(cls_var->type) {
            case tp_function: {
                if(name == __new__) goto __STATIC_NEW;
                py_newboundmethod(py_retval(), self, cls_var);
                return true;
            }
            case tp_nativefunc: {
                if(name == __new__) goto __STATIC_NEW;
                py_newboundmethod(py_retval(), self, cls_var);
                return true;
            }
            case tp_staticmethod: {
                py_assign(py_retval(), py_getslot(cls_var, 0));
                return true;
            }
            case tp_classmethod: {
                py_newboundmethod(py_retval(), py_tpobject(type), py_getslot(cls_var, 0));
                return true;
            }
            default: {
            __STATIC_NEW:
                py_assign(py_retval(), cls_var);
                return true;
            }
        }
    }

    py_Ref fallback = py_tpfindmagic(type, __getattr__);
    if(fallback) {
        py_push(fallback);
        py_push(self);
        py_newstr(py_pushtmp(), py_name2str(name));
        return py_vectorcall(1, 0);
    }

    if(self->type == tp_module) {
        py_Ref path = py_getdict(self, __path__);
        c11_sbuf buf;
        c11_sbuf__ctor(&buf);
        pk_sprintf(&buf, "%v.%n", py_tosv(path), name);
        c11_string* new_path = c11_sbuf__submit(&buf);
        int res = py_import(new_path->data);
        c11_string__delete(new_path);
        if(res == -1) {
            return false;
        } else if(res == 1) {
            return true;
        }
    }

    return AttributeError(self, name);
}

bool py_setattr(py_Ref self, py_Name name, py_Ref val) {
    py_Type type = self->type;
    py_Ref cls_var = py_tpfindname(type, name);
    if(cls_var) {
        // handle descriptor
        if(py_istype(cls_var, tp_property)) {
            py_Ref setter = py_getslot(cls_var, 1);
            if(!py_isnone(setter)) {
                py_push(setter);
                py_push(self);
                py_push(val);
                return py_vectorcall(1, 0);
            } else {
                return TypeError("readonly attribute: '%n'", name);
            }
        }
    }

    // handle instance __dict__
    if(self->is_ptr && self->_obj->slots == -1) {
        py_setdict(self, name, val);
        return true;
    }

    return TypeError("cannot set attribute");
}

bool py_delattr(py_Ref self, py_Name name) {
    if(self->is_ptr && self->_obj->slots == -1) {
        if(py_deldict(self, name)) return true;
        return AttributeError(self, name);
    }
    return TypeError("cannot delete attribute");
}

bool py_getitem(py_Ref self, py_Ref key) {
    py_push(self);
    py_push(key);
    bool ok = pk_callmagic(__getitem__, 2, py_peek(-2));
    py_shrink(2);
    return ok;
}

bool py_setitem(py_Ref self, py_Ref key, py_Ref val) {
    py_push(self);
    py_push(key);
    py_push(val);
    bool ok = pk_callmagic(__setitem__, 3, py_peek(-3));
    py_shrink(3);
    return ok;
}

bool py_delitem(py_Ref self, py_Ref key) {
    py_push(self);
    py_push(key);
    bool ok = pk_callmagic(__delitem__, 2, py_peek(-2));
    py_shrink(2);
    return ok;
}

int py_equal(py_Ref lhs, py_Ref rhs) {
    if(py_isidentical(lhs, rhs)) return 1;
    if(!py_eq(lhs, rhs)) return -1;
    return py_bool(py_retval());
}

int py_less(py_Ref lhs, py_Ref rhs) {
    if(!py_lt(lhs, rhs)) return -1;
    return py_bool(py_retval());
}