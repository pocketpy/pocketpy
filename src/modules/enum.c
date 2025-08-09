#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"

static bool Enum__wrapper_field(py_Name name, py_Ref value, void* ctx) {
    c11_sv name_sv = py_name2sv(name);
    if(name_sv.size == 0 || name_sv.data[0] == '_') return true;
    py_push(ctx);
    py_pushnil();
    py_assign(py_pushtmp(), py_name2ref(name));
    py_push(value);
    bool ok = py_vectorcall(2, 0);
    if(!ok) return false;
    py_assign(value, py_retval());
    return true;
}

static void Enum__on_end_subclass(py_TypeInfo* derived_ti) {
    derived_ti->is_final = true;
    py_applydict(&derived_ti->self, Enum__wrapper_field, &derived_ti->self);
}

static bool Enum__new__(int argc, py_Ref argv) {
    py_newobject(py_retval(), py_totype(argv), 2, 0);
    return true;
}

static bool Enum__init__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_setslot(argv, 0, py_arg(1));
    py_setslot(argv, 1, py_arg(2));
    py_newnone(py_retval());
    return true;
}

static bool Enum__str__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // f'{type(self).__name__}.{self.name}'
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_cstr(&buf, py_tpname(argv->type));
    c11_sbuf__write_char(&buf, '.');
    c11_sbuf__write_cstr(&buf, py_tostr(py_getslot(argv, 0)));
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool Enum__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // f'<{str(self)}: {self.value!r}>'
    if(!py_str(argv)) return false;
    py_push(py_retval());  // str(self)
    if(!py_repr(py_getslot(argv, 1))) return false;
    py_push(py_retval());  // repr(self.value)
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_cstr(&buf, "<");
    c11_sbuf__write_cstr(&buf, py_tostr(py_peek(-2)));
    c11_sbuf__write_cstr(&buf, ": ");
    c11_sbuf__write_cstr(&buf, py_tostr(py_peek(-1)));
    c11_sbuf__write_cstr(&buf, ">");
    c11_sbuf__py_submit(&buf, py_retval());
    py_shrink(2);
    return true;
}

static bool Enum__name(int argc, py_Ref argv) {
    py_assign(py_retval(), py_getslot(argv, 0));
    return true;
}

static bool Enum__value(int argc, py_Ref argv) {
    py_assign(py_retval(), py_getslot(argv, 1));
    return true;
}

void pk__add_module_enum() {
    py_Ref mod = py_newmodule("enum");
    py_Type type = py_newtype("Enum", tp_object, mod, NULL);

    py_bindmagic(type, __new__, Enum__new__);
    py_bindmagic(type, __init__, Enum__init__);
    py_bindmagic(type, __str__, Enum__str__);
    py_bindmagic(type, __repr__, Enum__repr__);
    py_bindproperty(type, "name", Enum__name, NULL);
    py_bindproperty(type, "value", Enum__value, NULL);

    pk_typeinfo(type)->on_end_subclass = Enum__on_end_subclass;
}