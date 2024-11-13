#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

#define DEF_TVALUE_METHODS(T, Field)                                                               \
    static bool TValue_##T##__new__(int argc, py_Ref argv) {                                       \
        PY_CHECK_ARGC(2);                                                                          \
        PY_CHECK_ARG_TYPE(0, tp_type);                                                             \
        PY_CHECK_ARG_TYPE(1, tp_##T);                                                              \
        *py_retval() = (py_TValue){                                                                \
            .type = py_totype(&argv[0]),                                                           \
            .is_ptr = false,                                                                       \
            .Field = py_to##T(&argv[1]),                                                           \
        };                                                                                         \
        return true;                                                                               \
    }                                                                                              \
    static bool TValue_##T##_value(int argc, py_Ref argv) {                                        \
        PY_CHECK_ARGC(1);                                                                          \
        py_new##T(py_retval(), argv->Field);                                                       \
        return true;                                                                               \
    }                                                                                              \
    static bool TValue_##T##__repr__(int argc, py_Ref argv) {                                      \
        PY_CHECK_ARGC(1);                                                                          \
        py_newstr(py_retval(), "<TValue_" #T " object>");                                          \
        return true;                                                                               \
    }

DEF_TVALUE_METHODS(int, _i64)
DEF_TVALUE_METHODS(float, _f64)
DEF_TVALUE_METHODS(vec2, _vec2)
DEF_TVALUE_METHODS(vec2i, _vec2i)

void pk__add_module_pkpy() {
    py_Ref mod = py_newmodule("pkpy");

    py_Type ttype;
    py_Ref TValue_dict = py_pushtmp();
    py_newdict(TValue_dict);

    ttype = pk_newtype("TValue_int", tp_object, mod, NULL, false, false);
    py_bindmagic(ttype, __new__, TValue_int__new__);
    py_bindmagic(ttype, __repr__, TValue_int__repr__);
    py_bindproperty(ttype, "value", TValue_int_value, NULL);
    py_dict_setitem(TValue_dict, py_tpobject(tp_int), py_tpobject(ttype));

    ttype = pk_newtype("TValue_float", tp_object, mod, NULL, false, false);
    py_bindmagic(ttype, __new__, TValue_float__new__);
    py_bindmagic(ttype, __repr__, TValue_float__repr__);
    py_bindproperty(ttype, "value", TValue_float_value, NULL);
    py_dict_setitem(TValue_dict, py_tpobject(tp_float), py_tpobject(ttype));

    ttype = pk_newtype("TValue_vec2", tp_object, mod, NULL, false, false);
    py_bindmagic(ttype, __new__, TValue_vec2__new__);
    py_bindmagic(ttype, __repr__, TValue_vec2__repr__);
    py_bindproperty(ttype, "value", TValue_vec2_value, NULL);
    py_dict_setitem(TValue_dict, py_tpobject(tp_vec2), py_tpobject(ttype));

    ttype = pk_newtype("TValue_vec2i", tp_object, mod, NULL, false, false);
    py_bindmagic(ttype, __new__, TValue_vec2i__new__);
    py_bindmagic(ttype, __repr__, TValue_vec2i__repr__);
    py_bindproperty(ttype, "value", TValue_vec2i_value, NULL);
    py_dict_setitem(TValue_dict, py_tpobject(tp_vec2i), py_tpobject(ttype));

    py_setdict(mod, py_name("TValue"), TValue_dict);
    py_pop();
}