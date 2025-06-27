#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/objects/iterator.h"
#include "pocketpy/interpreter/vm.h"

py_ObjectRef py_newtuple(py_OutRef out, int n) {
    VM* vm = pk_current_vm;
    PyObject* obj = ManagedHeap__gcnew(&vm->heap, tp_tuple, n, 0);
    out->type = tp_tuple;
    out->is_ptr = true;
    out->_obj = obj;
    return PyObject__slots(obj);
}

py_Ref py_tuple_getitem(py_Ref self, int i) { return py_getslot(self, i); }

py_Ref py_tuple_data(py_Ref self) { return PyObject__slots(self->_obj); }

void py_tuple_setitem(py_Ref self, int i, py_Ref val) { py_setslot(self, i, val); }

int py_tuple_len(py_Ref self) { return self->_obj->slots; }

//////////////
static bool tuple__len__(int argc, py_Ref argv) {
    py_newint(py_retval(), py_tuple_len(argv));
    return true;
}

static bool tuple__repr__(int argc, py_Ref argv) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, '(');
    int length = py_tuple_len(argv);
    for(int i = 0; i < length; i++) {
        py_TValue* val = py_getslot(argv, i);
        bool ok = py_repr(val);
        if(!ok) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        if(i != length - 1) c11_sbuf__write_cstr(&buf, ", ");
    }
    if(length == 1) c11_sbuf__write_char(&buf, ',');
    c11_sbuf__write_char(&buf, ')');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool tuple__new__(int argc, py_Ref argv) {
    if(argc == 1 + 0) {
        py_newtuple(py_retval(), 0);
        return true;
    }
    if(argc == 1 + 1) {
        bool ok = py_tpcall(tp_list, 1, py_arg(1));
        if(!ok) return false;
        py_Ref tmp = py_pushtmp();
        *tmp = *py_retval();  // backup the list
        int length = py_list_len(tmp);
        py_Ref p = py_newtuple(py_retval(), length);
        for(int i = 0; i < py_tuple_len(py_retval()); i++) {
            p[i] = *py_list_getitem(tmp, i);
        }
        py_pop();
        return true;
    }
    return TypeError("tuple() takes at most 1 argument");
}

static bool tuple__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int length = py_tuple_len(argv);
    py_Ref _1 = py_arg(1);
    if(_1->type == tp_int) {
        int index = py_toint(py_arg(1));
        if(!pk__normalize_index(&index, length)) return false;
        *py_retval() = *py_getslot(argv, index);
        return true;
    } else if(_1->type == tp_slice) {
        int start, stop, step;
        bool ok = pk__parse_int_slice(_1, length, &start, &stop, &step);
        if(!ok) return false;
        py_Ref tmp = py_pushtmp();
        py_newlist(tmp);
        PK_SLICE_LOOP(i, start, stop, step) py_list_append(tmp, py_getslot(argv, i));
        // convert list to tuple
        py_Ref p = py_newtuple(py_retval(), py_list_len(tmp));
        for(int i = 0; i < py_tuple_len(py_retval()); i++) {
            p[i] = *py_list_getitem(tmp, i);
        }
        py_pop();
        return true;
    } else {
        return TypeError("tuple indices must be integers");
    }
}

static bool tuple__eq__(int argc, py_Ref argv) {
    return pk_wrapper__arrayequal(tp_tuple, argc, argv);
}

static bool tuple__ne__(int argc, py_Ref argv) {
    if(!tuple__eq__(argc, argv)) return false;
    if(py_isbool(py_retval())) {
        bool res = py_tobool(py_retval());
        py_newbool(py_retval(), !res);
    }
    return true;
}

static bool tuple__lt__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(!py_istype(py_arg(1), tp_tuple)) {
        py_newnotimplemented(py_retval());
        return true;
    }
    py_TValue *p0, *p1;
    int lhs_length = py_tuple_len(py_arg(0));
    int rhs_length = py_tuple_len(py_arg(1));
    p0 = py_tuple_data(py_arg(0));
    p1 = py_tuple_data(py_arg(1));
    int length = lhs_length < rhs_length ? lhs_length : rhs_length;
    for(int i = 0; i < length; i++) {
        int res_lt = py_less(p0 + i, p1 + i);
        if(res_lt == -1) return false;
        if(res_lt) {
            py_newbool(py_retval(), true);
            return true;
        } else {
            int res_eq = py_equal(p0 + i, p1 + i);
            if(res_eq == -1) return false;
            if(!res_eq) {
                py_newbool(py_retval(), false);
                return true;
            }
        }
    }
    py_newbool(py_retval(), lhs_length < rhs_length);
    return true;
}

static bool tuple__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    tuple_iterator* ud = py_newobject(py_retval(), tp_tuple_iterator, 1, sizeof(tuple_iterator));
    ud->p = py_tuple_data(argv);
    ud->length = py_tuple_len(argv);
    ud->index = 0;
    py_setslot(py_retval(), 0, argv);  // keep a reference to the object
    return true;
}

static bool tuple__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    return pk_arraycontains(py_arg(0), py_arg(1));
}

static bool tuple__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int length = py_tuple_len(argv);
    py_TValue* data = py_tuple_data(argv);
    uint64_t x = 1000003;
    for(int i = 0; i < length; i++) {
        py_i64 y;
        if(!py_hash(&data[i], &y)) return false;
        // recommended by Github Copilot
        x = x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
    }
    py_newint(py_retval(), x);
    return true;
}

py_Type pk_tuple__register() {
    py_Type type = pk_newtype("tuple", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __len__, tuple__len__);
    py_bindmagic(type, __repr__, tuple__repr__);
    py_bindmagic(type, __new__, tuple__new__);
    py_bindmagic(type, __getitem__, tuple__getitem__);
    py_bindmagic(type, __eq__, tuple__eq__);
    py_bindmagic(type, __ne__, tuple__ne__);
    py_bindmagic(type, __lt__, tuple__lt__);
    py_bindmagic(type, __iter__, tuple__iter__);
    py_bindmagic(type, __contains__, tuple__contains__);
    py_bindmagic(type, __hash__, tuple__hash__);
    return type;
}
