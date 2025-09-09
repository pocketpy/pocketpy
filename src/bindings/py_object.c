#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/pocketpy.h"

bool pk__object_new(int argc, py_Ref argv) {
    if(argc == 0) return TypeError("object.__new__(): not enough arguments");
    py_TypeInfo* ti = py_touserdata(argv);
    py_Type cls = ti->index;
    if(!ti->is_python) {
        return TypeError("object.__new__(%t) is not safe, use %t.__new__() instead", cls, cls);
    }
    py_newobject(py_retval(), cls, -1, 0);
    return true;
}

static bool object__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    assert(argv->is_ptr);
    py_newint(py_retval(), (intptr_t)argv->_obj);
    return true;
}

static bool object__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool res = py_isidentical(py_arg(0), py_arg(1));
    py_newbool(py_retval(), res);
    return true;
}

static bool object__ne__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool res = py_isidentical(py_arg(0), py_arg(1));
    py_newbool(py_retval(), !res);
    return true;
}

static bool object__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    assert(argv->is_ptr);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "<%t object at %p>", argv->type, argv->_obj);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool object__dict__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    if(argv->is_ptr && argv->_obj->slots == -1) {
        pk_mappingproxy__namedict(py_retval(), argv);
    } else {
        py_newnone(py_retval());
    }
    return true;
}

static bool type__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "<class '%t'>", py_totype(argv));
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool type__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_Type type = py_typeof(py_arg(1));
    py_assign(py_retval(), py_tpobject(type));
    return true;
}

static bool type__base__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_TypeInfo* ti = py_touserdata(argv);
    if(ti->base) {
        py_assign(py_retval(), &ti->base_ti->self);
    } else {
        py_newnone(py_retval());
    }
    return true;
}

static bool type__name__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_TypeInfo* ti = py_touserdata(argv);
    py_assign(py_retval(), py_name2ref(ti->name));
    return true;
}

static bool type__getitem__(int argc, py_Ref argv) {
    py_assign(py_retval(), argv);
    return true;
}

static bool type__or__(int argc, py_Ref argv) {
    py_assign(py_retval(), argv);
    return true;
}

static bool type__module__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_TypeInfo* ti = py_touserdata(argv);
    if(py_isnil(ti->module)) {
        py_newnone(py_retval());
    } else {
        py_ModuleInfo* mi = py_touserdata(ti->module);
        py_newstrv(py_retval(), c11_string__sv(mi->path));
    }
    return true;
}

static bool type__annotations__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_TypeInfo* ti = py_touserdata(argv);
    if(py_isnil(&ti->annotations)) {
        py_newdict(py_retval());
    } else {
        py_assign(py_retval(), &ti->annotations);
    }
    return true;
}

void pk_object__register() {
    py_bindmagic(tp_object, __new__, pk__object_new);

    py_bindmagic(tp_object, __hash__, object__hash__);
    py_bindmagic(tp_object, __eq__, object__eq__);
    py_bindmagic(tp_object, __ne__, object__ne__);
    py_bindmagic(tp_object, __repr__, object__repr__);

    py_bindmagic(tp_type, __repr__, type__repr__);
    py_bindmagic(tp_type, __new__, type__new__);
    py_bindmagic(tp_type, __getitem__, type__getitem__);
    py_bindmagic(tp_type, __or__, type__or__);
    py_bindproperty(tp_type, "__module__", type__module__, NULL);

    py_bindproperty(tp_type, "__base__", type__base__, NULL);
    py_bindproperty(tp_type, "__name__", type__name__, NULL);
    py_bindproperty(tp_object, "__dict__", object__dict__, NULL);
    py_bindproperty(tp_type, "__annotations__", type__annotations__, NULL);
}