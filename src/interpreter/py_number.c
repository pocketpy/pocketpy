#include "pocketpy/interpreter/vm.h"

#include <math.h>

#define DEF_NUM_BINARY_OP(name, op, rint, rfloat)                                                  \
    static bool _py_int##name(int argc, py_Ref argv) {                                             \
        py_checkargc(2);                                                                           \
        if(py_isint(&argv[1])) {                                                                   \
            int64_t lhs = py_toint(&argv[0]);                                                      \
            int64_t rhs = py_toint(&argv[1]);                                                      \
            rint(py_retval(), lhs op rhs);                                                         \
        } else if(py_isfloat(&argv[1])) {                                                          \
            int64_t lhs = py_toint(&argv[0]);                                                      \
            double rhs = py_tofloat(&argv[1]);                                                     \
            rfloat(py_retval(), lhs op rhs);                                                       \
        } else {                                                                                   \
            py_newnotimplemented(py_retval());                                                     \
        }                                                                                          \
        return true;                                                                               \
    }                                                                                              \
    static bool _py_float##name(int argc, py_Ref argv) {                                           \
        py_checkargc(2);                                                                           \
        double lhs = py_tofloat(&argv[0]);                                                         \
        double rhs;                                                                                \
        if(py_castfloat(&argv[1], &rhs)) {                                                         \
            rfloat(py_retval(), lhs op rhs);                                                       \
        } else {                                                                                   \
            py_newnotimplemented(py_retval());                                                     \
        }                                                                                          \
        return true;                                                                               \
    }

DEF_NUM_BINARY_OP(__add__, +, py_newint, py_newfloat)
DEF_NUM_BINARY_OP(__sub__, -, py_newint, py_newfloat)
DEF_NUM_BINARY_OP(__mul__, *, py_newint, py_newfloat)

DEF_NUM_BINARY_OP(__eq__, ==, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__ne__, ==, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__lt__, <, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__le__, <=, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__gt__, >, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__ge__, >=, py_newbool, py_newbool)

#undef DEF_NUM_BINARY_OP

static bool _py_int__neg__(int argc, py_Ref argv) {
    py_checkargc(1);
    int64_t val = py_toint(&argv[0]);
    py_newint(py_retval(), -val);
    return true;
}

static bool _py_float__neg__(int argc, py_Ref argv) {
    py_checkargc(1);
    double val = py_tofloat(&argv[0]);
    py_newfloat(py_retval(), -val);
    return true;
}

static bool _py_int__truediv__(int argc, py_Ref argv) {
    py_checkargc(2);
    int64_t lhs = py_toint(&argv[0]);
    double rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_float__truediv__(int argc, py_Ref argv) {
    py_checkargc(2);
    double lhs = py_tofloat(&argv[0]);
    double rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

#define ZeroDivisionError(msg) false

static bool _py_number__pow__(int argc, py_Ref argv) {
    py_checkargc(2);
    if(py_isint(&argv[0]) && py_isint(&argv[1])) {
        int64_t lhs = py_toint(&argv[0]);
        int64_t rhs = py_toint(&argv[1]);
        if(rhs < 0) {
            if(lhs == 0) {
                return ZeroDivisionError("0.0 cannot be raised to a negative power");
            } else {
                py_newfloat(py_retval(), pow(lhs, rhs));
            }
        } else {
            int64_t ret = 1;
            while(rhs) {
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            py_newint(py_retval(), ret);
        }
    } else {
        double lhs, rhs;
        py_castfloat(&argv[0], &lhs);
        if(py_castfloat(&argv[1], &rhs)) {
            py_newfloat(py_retval(), pow(lhs, rhs));
        } else {
            py_newnotimplemented(py_retval());
        }
    }
    return true;
}

static bool _py_int__floordiv__(int argc, py_Ref argv) {
    py_checkargc(2);
    int64_t lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        int64_t rhs = py_toint(&argv[1]);
        if(rhs == 0) return -1;
        py_newint(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_int__mod__(int argc, py_Ref argv) {
    py_checkargc(2);
    int64_t lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        int64_t rhs = py_toint(&argv[1]);
        if(rhs == 0) return ZeroDivisionError("integer division or modulo by zero");
        py_newint(py_retval(), lhs % rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_int__invert__(int argc, py_Ref argv) {
    py_checkargc(1);
    int64_t val = py_toint(&argv[0]);
    py_newint(py_retval(), ~val);
    return true;
}

static bool _py_int__bit_length(int argc, py_Ref argv) {
    py_checkargc(1);
    int64_t x = py_toint(py_arg(0));
    if(x < 0) x = -x;
    int bits = 0;
    while(x) {
        x >>= 1;
        bits++;
    }
    py_newint(py_retval(), bits);
    return true;
}

#define DEF_INT_BITWISE_OP(name, op)                                                               \
    static bool _py_int##name(int argc, py_Ref argv) {                                             \
        py_checkargc(2);                                                                           \
        int64_t lhs = py_toint(&argv[0]);                                                          \
        if(py_isint(&argv[1])) {                                                                   \
            int64_t rhs = py_toint(&argv[1]);                                                      \
            py_newint(py_retval(), lhs op rhs);                                                    \
        } else {                                                                                   \
            py_newnotimplemented(py_retval());                                                     \
        }                                                                                          \
        return true;                                                                               \
    }

DEF_INT_BITWISE_OP(__and__, &)
DEF_INT_BITWISE_OP(__or__, |)
DEF_INT_BITWISE_OP(__xor__, ^)
DEF_INT_BITWISE_OP(__lshift__, <<)
DEF_INT_BITWISE_OP(__rshift__, >>)

#undef DEF_INT_BITWISE_OP

void pk_VM__init_builtins(pk_VM* self) {
    /****** tp_int & tp_float ******/
    py_bindmagic(tp_int, __add__, _py_int__add__);
    py_bindmagic(tp_float, __add__, _py_float__add__);
    py_bindmagic(tp_int, __sub__, _py_int__sub__);
    py_bindmagic(tp_float, __sub__, _py_float__sub__);
    py_bindmagic(tp_int, __mul__, _py_int__mul__);
    py_bindmagic(tp_float, __mul__, _py_float__mul__);

    py_bindmagic(tp_int, __eq__, _py_int__eq__);
    py_bindmagic(tp_float, __eq__, _py_float__eq__);
    py_bindmagic(tp_int, __ne__, _py_int__ne__);
    py_bindmagic(tp_float, __ne__, _py_float__ne__);
    py_bindmagic(tp_int, __lt__, _py_int__lt__);
    py_bindmagic(tp_float, __lt__, _py_float__lt__);
    py_bindmagic(tp_int, __le__, _py_int__le__);
    py_bindmagic(tp_float, __le__, _py_float__le__);
    py_bindmagic(tp_int, __gt__, _py_int__gt__);
    py_bindmagic(tp_float, __gt__, _py_float__gt__);
    py_bindmagic(tp_int, __ge__, _py_int__ge__);
    py_bindmagic(tp_float, __ge__, _py_float__ge__);

    // __neg__
    py_bindmagic(tp_int, __neg__, _py_int__neg__);
    py_bindmagic(tp_float, __neg__, _py_float__neg__);

    // TODO: __repr__, __new__, __hash__

    // __truediv__
    py_bindmagic(tp_int, __truediv__, _py_int__truediv__);
    py_bindmagic(tp_float, __truediv__, _py_float__truediv__);

    // __pow__
    py_bindmagic(tp_int, __pow__, _py_number__pow__);
    py_bindmagic(tp_float, __pow__, _py_number__pow__);

    // __floordiv__ & __mod__
    py_bindmagic(tp_int, __floordiv__, _py_int__floordiv__);
    py_bindmagic(tp_int, __mod__, _py_int__mod__);

    // int.__invert__ & int.<BITWISE OP>
    py_bindmagic(tp_int, __invert__, _py_int__invert__);

    py_bindmagic(tp_int, __and__, _py_int__and__);
    py_bindmagic(tp_int, __or__, _py_int__or__);
    py_bindmagic(tp_int, __xor__, _py_int__xor__);
    py_bindmagic(tp_int, __lshift__, _py_int__lshift__);
    py_bindmagic(tp_int, __rshift__, _py_int__rshift__);

    // int.bit_length
    py_bindmethod(tp_int, "bit_length", _py_int__bit_length);
}