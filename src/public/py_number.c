#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/pocketpy.h"

#include <math.h>

#define DEF_NUM_BINARY_OP(name, op, rint, rfloat)                                                  \
    static bool _py_int##name(int argc, py_Ref argv) {                                             \
        PY_CHECK_ARGC(2);                                                                          \
        if(py_isint(&argv[1])) {                                                                   \
            py_i64 lhs = py_toint(&argv[0]);                                                       \
            py_i64 rhs = py_toint(&argv[1]);                                                       \
            rint(py_retval(), lhs op rhs);                                                         \
        } else if(py_isfloat(&argv[1])) {                                                          \
            py_i64 lhs = py_toint(&argv[0]);                                                       \
            py_f64 rhs = py_tofloat(&argv[1]);                                                     \
            rfloat(py_retval(), lhs op rhs);                                                       \
        } else {                                                                                   \
            py_newnotimplemented(py_retval());                                                     \
        }                                                                                          \
        return true;                                                                               \
    }                                                                                              \
    static bool _py_float##name(int argc, py_Ref argv) {                                           \
        PY_CHECK_ARGC(2);                                                                          \
        py_f64 lhs = py_tofloat(&argv[0]);                                                         \
        py_f64 rhs;                                                                                \
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
DEF_NUM_BINARY_OP(__ne__, !=, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__lt__, <, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__le__, <=, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__gt__, >, py_newbool, py_newbool)
DEF_NUM_BINARY_OP(__ge__, >=, py_newbool, py_newbool)

#undef DEF_NUM_BINARY_OP

static bool _py_int__neg__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), -val);
    return true;
}

static bool _py_float__neg__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    py_newfloat(py_retval(), -val);
    return true;
}

static bool _py_int__truediv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    py_f64 rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_float__truediv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 lhs = py_tofloat(&argv[0]);
    py_f64 rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

#define ZeroDivisionError(msg) false

static bool _py_number__pow__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(py_isint(&argv[0]) && py_isint(&argv[1])) {
        py_i64 lhs = py_toint(&argv[0]);
        py_i64 rhs = py_toint(&argv[1]);
        if(rhs < 0) {
            if(lhs == 0) {
                return ZeroDivisionError("0.0 cannot be raised to a negative power");
            } else {
                py_newfloat(py_retval(), pow(lhs, rhs));
            }
        } else {
            // rhs >= 0
            py_i64 ret = 1;
            while(true) {
                if(rhs & 1) ret *= lhs;
                rhs >>= 1;
                if(!rhs) break;
                lhs *= lhs;  // place this here to avoid overflow
            }
            py_newint(py_retval(), ret);
        }
    } else {
        py_f64 lhs, rhs;
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
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        py_i64 rhs = py_toint(&argv[1]);
        if(rhs == 0) return -1;
        py_newint(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_int__mod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        py_i64 rhs = py_toint(&argv[1]);
        if(rhs == 0) return ZeroDivisionError("integer division or modulo by zero");
        py_newint(py_retval(), lhs % rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_int__invert__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), ~val);
    return true;
}

static bool _py_int__bit_length(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 x = py_toint(py_arg(0));
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
        PY_CHECK_ARGC(2);                                                                          \
        py_i64 lhs = py_toint(&argv[0]);                                                           \
        if(py_isint(&argv[1])) {                                                                   \
            py_i64 rhs = py_toint(&argv[1]);                                                       \
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

static bool _py_int__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    char buf[32];
    int size = snprintf(buf, sizeof(buf), "%lld", (long long)val);
    py_newstrn(py_retval(), buf, size);
    return true;
}

static bool _py_float__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_f64(&buf, val, -1);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

union c11_8bytes {
    py_i64 _i64;
    py_f64 _f64;

    union {
        uint32_t upper;
        uint32_t lower;
    } bits;
};

static py_i64 c11_8bytes__hash(union c11_8bytes u) {
    // https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key
    const uint32_t C = 2654435761;
    u.bits.upper *= C;
    u.bits.lower *= C;
    return u._i64;
}

static bool _py_int__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    union c11_8bytes u = {._i64 = val};
    py_newint(py_retval(), c11_8bytes__hash(u));
    return true;
}

static bool _py_float__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    union c11_8bytes u = {._f64 = val};
    py_newint(py_retval(), c11_8bytes__hash(u));
    return true;
}

static bool _py_int__abs__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), val < 0 ? -val : val);
    return true;
}

static bool _py_float__abs__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    py_newfloat(py_retval(), val < 0 ? -val : val);
    return true;
}

static bool _py_int__new__(int argc, py_Ref argv) {
    if(argc == 1 + 0) {
        // int() == 0
        py_newint(py_retval(), 0);
        return true;
    }
    // 1 arg
    if(argc == 1 + 1) {
        switch(argv[1].type) {
            case tp_float: {
                // int(1.1) == 1
                py_newint(py_retval(), (py_i64)py_tofloat(&argv[1]));
                return true;
            }
            case tp_int: {
                // int(1) == 1
                *py_retval() = argv[1];
                return true;
            }
            case tp_bool: {
                // int(True) == 1
                py_newint(py_retval(), (py_i64)py_tobool(&argv[1]));
                return true;
            }
            case tp_str: break;  // leave to the next block
            default: return TypeError("invalid arguments for int()");
        }
    }
    // 2+ args -> error
    if(argc > 1 + 2) return TypeError("int() takes at most 2 arguments");
    // 1 or 2 args with str
    int base = 10;
    if(argc == 1 + 2) {
        PY_CHECK_ARG_TYPE(2, tp_int);
        base = py_toint(py_arg(2));
    }

    PY_CHECK_ARG_TYPE(1, tp_str);

    int size;
    const char* data = py_tostrn(py_arg(1), &size);
    bool negative = false;
    if(size && (data[0] == '+' || data[0] == '-')) {
        negative = data[0] == '-';
        data++;
        size--;
    }
    py_i64 val;
    if(c11__parse_uint((c11_sv){data, size}, &val, base) != IntParsing_SUCCESS) {
        return ValueError("invalid literal for int() with base %d: %q", base, data);
    }
    py_newint(py_retval(), negative ? -val : val);
    return true;
}

static bool _py_float__new__(int argc, py_Ref argv) {
    if(argc == 1 + 0) {
        // float() == 0.0
        py_newfloat(py_retval(), 0.0);
        return true;
    }
    if(argc > 1 + 1) return TypeError("float() takes at most 1 argument");
    // 1 arg
    switch(argv[1].type) {
        case tp_int: {
            // float(1) == 1.0
            py_newfloat(py_retval(), py_toint(&argv[1]));
            return true;
        }
        case tp_float: {
            // float(1.1) == 1.1
            *py_retval() = argv[1];
            return true;
        }
        case tp_bool: {
            // float(True) == 1.0
            py_newfloat(py_retval(), py_tobool(&argv[1]));
            return true;
        }
        case tp_str: break;  // leave to the next block
        default: return TypeError("invalid arguments for float()");
    }
    // str to float
    int size;
    const char* data = py_tostrn(py_arg(1), &size);

    if(c11__streq(data, "inf")) {
        py_newfloat(py_retval(), INFINITY);
        return true;
    }
    if(c11__streq(data, "-inf")) {
        py_newfloat(py_retval(), -INFINITY);
        return true;
    }

    char* p_end;
    py_f64 float_out = strtod(data, &p_end);
    if(p_end != data + size) { return ValueError("invalid literal for float(): %q", data); }
    py_newfloat(py_retval(), float_out);
    return true;
}

// tp_bool
static bool _py_bool__new__(int argc, py_Ref argv) {
    assert(argc > 0);
    if(argc == 1){
        py_newbool(py_retval(), false);
        return true;
    }
    if(argc == 2){
        int res = py_bool(py_arg(1));
        if(res == -1) return false;
        py_newbool(py_retval(), res);
        return true;
    }
    return TypeError("bool() takes at most 1 argument");
}

static bool _py_bool__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    bool res = py_tobool(argv);
    py_newint(py_retval(), res);
    return true;
}

static bool _py_bool__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    bool res = py_tobool(argv);
    py_newstr(py_retval(), res ? "True" : "False");
    return true;
}

static bool _py_bool__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool lhs = py_tobool(&argv[0]);
    if(argv[1].type == tp_bool) {
        bool rhs = py_tobool(&argv[1]);
        py_newbool(py_retval(), lhs == rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool _py_bool__ne__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    bool lhs = py_tobool(&argv[0]);
    if(argv[1].type == tp_bool) {
        bool rhs = py_tobool(&argv[1]);
        py_newbool(py_retval(), lhs != rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

void pk_number__register() {
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

    // __repr__
    py_bindmagic(tp_int, __repr__, _py_int__repr__);
    py_bindmagic(tp_float, __repr__, _py_float__repr__);

    // __hash__
    py_bindmagic(tp_int, __hash__, _py_int__hash__);
    py_bindmagic(tp_float, __hash__, _py_float__hash__);

    // __abs__
    py_bindmagic(tp_int, __abs__, _py_int__abs__);
    py_bindmagic(tp_float, __abs__, _py_float__abs__);

    // __new__
    py_bindmagic(tp_int, __new__, _py_int__new__);
    py_bindmagic(tp_float, __new__, _py_float__new__);

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

    /* tp_bool */
    py_bindmagic(tp_bool, __new__, _py_bool__new__);
    py_bindmagic(tp_bool, __hash__, _py_bool__hash__);
    py_bindmagic(tp_bool, __repr__, _py_bool__repr__);
    py_bindmagic(tp_bool, __eq__, _py_bool__eq__);
    py_bindmagic(tp_bool, __ne__, _py_bool__ne__);
}