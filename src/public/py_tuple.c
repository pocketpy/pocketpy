#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_newtuple(py_Ref out, int n) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_tuple, n, 0);
    out->type = tp_tuple;
    out->is_ptr = true;
    out->_obj = obj;
}

py_Ref py_tuple__getitem(const py_Ref self, int i) { return py_getslot(self, i); }

py_Ref py_tuple__data(const py_Ref self) { return PyObject__slots(self->_obj); }

void py_tuple__setitem(py_Ref self, int i, const py_Ref val) { py_setslot(self, i, val); }

int py_tuple__len(const py_Ref self) { return self->_obj->slots; }

//////////////
static bool _py_tuple__len__(int argc, py_Ref argv) {
    py_newint(py_retval(), py_tuple__len(argv));
    return true;
}

static bool _py_tuple__repr__(int argc, py_Ref argv) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, '(');
    int length = py_tuple__len(argv);
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
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

static bool _py_tuple__new__(int argc, py_Ref argv) {
    if(argc == 1 + 0) {
        py_newtuple(py_retval(), 0);
        return true;
    }
    if(argc == 1 + 1) {
        bool ok = py_tpcall(tp_list, 1, py_arg(1));
        if(!ok) return false;
        py_Ref tmp = py_pushtmp();
        *tmp = *py_retval();  // backup the list
        int length = py_list__len(tmp);
        py_newtuple(py_retval(), length);
        for(int i = 0; i < py_tuple__len(py_retval()); i++) {
            py_tuple__setitem(py_retval(), i, py_list__getitem(tmp, i));
        }
        return true;
    }
    return TypeError("tuple() takes at most 1 argument");
}

static bool _py_tuple__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int length = py_tuple__len(argv);
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
        PK_SLICE_LOOP(i, start, stop, step) py_list__append(tmp, py_getslot(argv, i));
        // convert list to tuple
        py_newtuple(py_retval(), py_list__len(tmp));
        for(int i = 0; i < py_tuple__len(py_retval()); i++) {
            py_tuple__setitem(py_retval(), i, py_list__getitem(tmp, i));
        }
        return true;
    } else {
        return TypeError("tuple indices must be integers");
    }
}

static bool _py_tuple__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(py_istype(py_arg(1), tp_tuple)) {
        int length0, length1;
        py_TValue* a0 = pk_arrayview(py_arg(0), &length0);
        py_TValue* a1 = pk_arrayview(py_arg(1), &length1);
        int res = pk_arrayeq(a0, length0, a1, length1);
        if(res == -1) return false;
        py_newbool(py_retval(), res);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_tuple__ne__(int argc, py_Ref argv) {
    bool ok = _py_tuple__eq__(argc, argv);
    if(!ok) return false;
    py_Ref retval = py_retval();
    py_newbool(retval, !py_tobool(retval));
    return true;
}

static bool _py_tuple__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_tpcall(tp_array_iterator, 1, argv);
}

py_Type pk_tuple__register() {
    pk_VM* vm = pk_current_vm;
    py_Type type = pk_VM__new_type(vm, "tuple", tp_object, NULL, false);

    py_bindmagic(type, __len__, _py_tuple__len__);
    py_bindmagic(type, __repr__, _py_tuple__repr__);
    py_bindmagic(type, __new__, _py_tuple__new__);
    py_bindmagic(type, __getitem__, _py_tuple__getitem__);
    py_bindmagic(type, __eq__, _py_tuple__eq__);
    py_bindmagic(type, __ne__, _py_tuple__ne__);
    py_bindmagic(type, __iter__, _py_tuple__iter__);
    return type;
}
