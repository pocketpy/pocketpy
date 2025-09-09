#include "pocketpy/pocketpy.h"

#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

py_ObjectRef py_newslice(py_OutRef out) {
    VM* vm = pk_current_vm;
    PyObject* obj = ManagedHeap__gcnew(&vm->heap, tp_slice, 3, 0);
    out->type = tp_slice;
    out->is_ptr = true;
    out->_obj = obj;
    return PyObject__slots(obj);
}

void py_newsliceint(py_OutRef out, py_i64 start, py_i64 stop, py_i64 step) {
    py_Ref slots = py_newslice(out);
    py_newint(&slots[0], start);
    py_newint(&slots[1], stop);
    py_newint(&slots[2], step);
}

static bool slice__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1 + 3);
    py_Ref slice = py_retval();
    py_newslice(slice);
    py_setslot(slice, 0, py_arg(1));
    py_setslot(slice, 1, py_arg(2));
    py_setslot(slice, 2, py_arg(3));
    return true;
}

static bool slice__repr__(int argc, py_Ref argv) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_cstr(&buf, "slice(");
    for(int i = 0; i < 3; i++) {
        py_TValue* val = py_getslot(argv, i);
        bool ok = py_repr(val);
        if(!ok) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        if(i != 2) c11_sbuf__write_cstr(&buf, ", ");
    }
    c11_sbuf__write_char(&buf, ')');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool slice_start(int argc, py_Ref argv) {
    py_Ref self = py_arg(0);
    py_TValue* val = py_getslot(self, 0);
    py_assign(py_retval(), val);
    return true;
}

static bool slice_stop(int argc, py_Ref argv) {
    py_Ref self = py_arg(0);
    py_TValue* val = py_getslot(self, 1);
    py_assign(py_retval(), val);
    return true;
}

static bool slice_step(int argc, py_Ref argv) {
    py_Ref self = py_arg(0);
    py_TValue* val = py_getslot(self, 2);
    py_assign(py_retval(), val);
    return true;
}

static bool slice__eq__(int argc, py_Ref argv) {
    py_Ref self = py_arg(0);
    py_Ref other = py_arg(1);
    if(!py_istype(other, tp_slice)) {
        py_newnotimplemented(py_retval());
        return true;
    }
    for(int i = 0; i < 3; i++) {
        py_Ref lhs = py_getslot(self, i);
        py_Ref rhs = py_getslot(other, i);
        int res = py_equal(lhs, rhs);
        if(res == -1) return false;
        if(res == 0) {
            py_newbool(py_retval(), false);
            return true;
        }
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool slice__ne__(int argc, py_Ref argv) {
    bool ok = slice__eq__(argc, argv);
    if(!ok) return false;
    py_Ref res = py_retval();
    if(py_isbool(res)) py_newbool(py_retval(), !py_tobool(res));
    return true;
}

py_Type pk_slice__register() {
    py_Type type = pk_newtype("slice", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, slice__new__);
    py_bindmagic(type, __repr__, slice__repr__);
    py_bindmagic(type, __eq__, slice__eq__);
    py_bindmagic(type, __ne__, slice__ne__);

    py_setdict(py_tpobject(type), __hash__, py_None());

    py_bindproperty(type, "start", slice_start, NULL);
    py_bindproperty(type, "stop", slice_stop, NULL);
    py_bindproperty(type, "step", slice_step, NULL);
    return type;
}