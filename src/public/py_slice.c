#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

void py_newslice(py_Ref out) {
    pk_VM* vm = pk_current_vm;
    PyObject* obj = pk_ManagedHeap__gcnew(&vm->heap, tp_slice, 3, 0);
    out->type = tp_slice;
    out->is_ptr = true;
    out->_obj = obj;
}

static bool _py_slice__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1 + 3);
    py_Ref slice = py_retval();
    py_newslice(slice);
    py_setslot(slice, 0, py_arg(1));
    py_setslot(slice, 1, py_arg(2));
    py_setslot(slice, 2, py_arg(3));
    return true;
}

static bool _py_slice__repr__(int argc, py_Ref argv) {
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
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

py_Type pk_slice__register() {
    py_Type type = pk_newtype("slice", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, _py_slice__new__);
    py_bindmagic(type, __repr__, _py_slice__repr__);
    return type;
}