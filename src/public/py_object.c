#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/pocketpy.h"

static bool _py_object__new__(int argc, py_Ref argv) {
    assert(argc >= 1);
    py_Type cls = py_totype(py_arg(0));
    py_newobject(py_retval(), cls, 0, 0);
    return true;
}

static bool _py_object__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    assert(argv->is_ptr);
    py_newint(py_retval(), (py_i64)argv->_obj);
    return true;
}

static bool _py_object__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool res = py_isidentical(py_arg(0), py_arg(1));
    py_newbool(py_retval(), res);
    return true;
}

static bool _py_object__ne__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool res = py_isidentical(py_arg(0), py_arg(1));
    py_newbool(py_retval(), !res);
    return true;
}

static bool _py_object__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    assert(argv->is_ptr);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "<%t object at %p>", argv->type, argv->_obj);
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

static bool _py_type__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "<class '%t'>", py_totype(argv));
    c11_string* res = c11_sbuf__submit(&buf);
    py_newstrn(py_retval(), res->data, res->size);
    c11_string__delete(res);
    return true;
}

void pk_object__register() {
    py_bindmagic(tp_object, __new__, _py_object__new__);
    py_bindmagic(tp_object, __hash__, _py_object__hash__);
    py_bindmagic(tp_object, __eq__, _py_object__eq__);
    py_bindmagic(tp_object, __ne__, _py_object__ne__);
    py_bindmagic(tp_object, __repr__, _py_object__repr__);

    // type patch...
    py_bindmagic(tp_type, __repr__, _py_type__repr__);
}