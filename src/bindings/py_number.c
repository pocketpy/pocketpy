#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/pocketpy.h"

#include <math.h>

static bool try_castfloat(py_Ref self, double* out) {
    switch(self->type) {
        case tp_int: *out = (double)self->_i64; return true;
        case tp_float: *out = self->_f64; return true;
        default: return false;
    }
}

#define DEF_NUM_BINARY_OP(name, op, rint, rfloat)                                                  \
    static bool int##name(int argc, py_Ref argv) {                                                 \
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
    static bool float##name(int argc, py_Ref argv) {                                               \
        PY_CHECK_ARGC(2);                                                                          \
        py_f64 lhs = py_tofloat(&argv[0]);                                                         \
        py_f64 rhs;                                                                                \
        if(try_castfloat(&argv[1], &rhs)) {                                                        \
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

static bool int__neg__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), -val);
    return true;
}

static bool float__neg__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    py_newfloat(py_retval(), -val);
    return true;
}

static bool int__truediv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    py_f64 rhs;
    if(try_castfloat(&argv[1], &rhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float division by zero");
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool float__truediv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 lhs = py_tofloat(&argv[0]);
    py_f64 rhs;
    if(try_castfloat(&argv[1], &rhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float division by zero");
        py_newfloat(py_retval(), lhs / rhs);
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool number__pow__(int argc, py_Ref argv) {
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
        if(!py_castfloat(&argv[0], &lhs)) return false;
        if(try_castfloat(&argv[1], &rhs)) {
            py_newfloat(py_retval(), pow(lhs, rhs));
        } else {
            py_newnotimplemented(py_retval());
        }
    }
    return true;
}

static py_i64 i64_abs(py_i64 x) { return x < 0 ? -x : x; }

static py_i64 cpy11__fast_floor_div(py_i64 a, py_i64 b) {
    assert(b != 0);
    if(a == 0) return 0;
    if((a < 0) == (b < 0)) {
        return i64_abs(a) / i64_abs(b);
    } else {
        return -1 - (i64_abs(a) - 1) / i64_abs(b);
    }
}

static py_i64 cpy11__fast_mod(py_i64 a, py_i64 b) {
    assert(b != 0);
    if(a == 0) return 0;
    py_i64 res;
    if((a < 0) == (b < 0)) {
        res = i64_abs(a) % i64_abs(b);
    } else {
        res = i64_abs(b) - 1 - (i64_abs(a) - 1) % i64_abs(b);
    }
    return b < 0 ? -res : res;
}

// https://github.com/python/cpython/blob/3.11/Objects/floatobject.c#L677
static void cpy11__float_div_mod(double vx, double wx, double *floordiv, double *mod)
{
    double div;
    *mod = fmod(vx, wx);
    /* fmod is typically exact, so vx-mod is *mathematically* an
       exact multiple of wx.  But this is fp arithmetic, and fp
       vx - mod is an approximation; the result is that div may
       not be an exact integral value after the division, although
       it will always be very close to one.
    */
    div = (vx - *mod) / wx;
    if (*mod) {
        /* ensure the remainder has the same sign as the denominator */
        if ((wx < 0) != (*mod < 0)) {
            *mod += wx;
            div -= 1.0;
        }
    }
    else {
        /* the remainder is zero, and in the presence of signed zeroes
           fmod returns different results across platforms; ensure
           it has the same sign as the denominator. */
        *mod = copysign(0.0, wx);
    }
    /* snap quotient to nearest integral value */
    if (div) {
        *floordiv = floor(div);
        if (div - *floordiv > 0.5) {
            *floordiv += 1.0;
        }
    }
    else {
        /* div is zero - get the same sign as the true quotient */
        *floordiv = copysign(0.0, vx / wx); /* zero w/ sign of vx/wx */
    }
}

static bool int__floordiv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        py_i64 rhs = py_toint(&argv[1]);
        if(rhs == 0) return ZeroDivisionError("integer division by zero");
        py_newint(py_retval(), cpy11__fast_floor_div(lhs, rhs));
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool int__mod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_i64 lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        py_i64 rhs = py_toint(&argv[1]);
        if(rhs == 0) return ZeroDivisionError("integer modulo by zero");
        py_newint(py_retval(), cpy11__fast_mod(lhs, rhs));
    } else {
        py_newnotimplemented(py_retval());
    }
    return true;
}

static bool float__floordiv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 lhs = py_tofloat(&argv[0]);
    py_f64 rhs;
    if(try_castfloat(&argv[1], &rhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float modulo by zero");
        double q, r;
        cpy11__float_div_mod(lhs, rhs, &q, &r);
        py_newfloat(py_retval(), q);
        return true;
    }
    py_newnotimplemented(py_retval());
    return true;
}

static bool float__rfloordiv__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 rhs = py_tofloat(&argv[0]);
    py_f64 lhs;
    if(try_castfloat(&argv[1], &lhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float modulo by zero");
        double q, r;
        cpy11__float_div_mod(lhs, rhs, &q, &r);
        py_newfloat(py_retval(), q);
        return true;
    }
    py_newnotimplemented(py_retval());
    return true;
}

static bool float__mod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 lhs = py_tofloat(&argv[0]);
    py_f64 rhs;
    if(try_castfloat(&argv[1], &rhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float modulo by zero");
        double q, r;
        cpy11__float_div_mod(lhs, rhs, &q, &r);
        py_newfloat(py_retval(), r);
        return true;
    }
    py_newnotimplemented(py_retval());
    return true;
}

static bool float__rmod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 rhs = py_tofloat(&argv[0]);
    py_f64 lhs;
    if(try_castfloat(&argv[1], &lhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float modulo by zero");
        double q, r;
        cpy11__float_div_mod(lhs, rhs, &q, &r);
        py_newfloat(py_retval(), r);
        return true;
    }
    py_newnotimplemented(py_retval());
    return true;
}

static bool float__divmod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    py_f64 lhs = py_tofloat(&argv[0]);
    py_f64 rhs;
    if(try_castfloat(&argv[1], &rhs)) {
        if(rhs == 0.0) return ZeroDivisionError("float modulo by zero");
        double q, r;
        cpy11__float_div_mod(lhs, rhs, &q, &r);
        py_Ref p = py_newtuple(py_retval(), 2);
        py_newfloat(&p[0], q);
        py_newfloat(&p[1], r);
        return true;
    }
    return TypeError("divmod() expects int or float as divisor");
}

static bool int__divmod__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 lhs = py_toint(&argv[0]);
    py_i64 rhs = py_toint(&argv[1]);
    if(rhs == 0) return ZeroDivisionError("integer division or modulo by zero");
    py_Ref p = py_newtuple(py_retval(), 2);
    py_newint(&p[0], cpy11__fast_floor_div(lhs, rhs));
    py_newint(&p[1], cpy11__fast_mod(lhs, rhs));
    return true;
}

static bool int__invert__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), ~val);
    return true;
}

static bool int_bit_length(int argc, py_Ref argv) {
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
    static bool int##name(int argc, py_Ref argv) {                                                 \
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

static bool int__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    char buf[32];
    int size = snprintf(buf, sizeof(buf), "%lld", (long long)val);
    py_newstrv(py_retval(), (c11_sv){buf, size});
    return true;
}

static bool float__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_f64(&buf, val, -1);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool int__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_assign(py_retval(), argv);
    return true;
}

static bool float__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    py_i64 h_user;
    memcpy(&h_user, &val, sizeof(py_f64));
    py_newint(py_retval(), h_user);
    return true;
}

static bool int__abs__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val = py_toint(&argv[0]);
    py_newint(py_retval(), val < 0 ? -val : val);
    return true;
}

static bool float__abs__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 val = py_tofloat(&argv[0]);
    py_newfloat(py_retval(), val < 0 ? -val : val);
    return true;
}

static bool int__new__(int argc, py_Ref argv) {
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
            default: return TypeError("int() argument must be a string, number or boolean");
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

    c11_sv sv = py_tosv(py_arg(1));
    bool negative = false;
    if(sv.size && (sv.data[0] == '+' || sv.data[0] == '-')) {
        negative = sv.data[0] == '-';
        sv.data++;
        sv.size--;
    }
    py_i64 val;
    if(c11__parse_uint(sv, &val, base) != IntParsing_SUCCESS) {
        return ValueError("invalid literal for int() with base %d: %q", base, sv);
    }
    py_newint(py_retval(), negative ? -val : val);
    return true;
}

static bool float__new__(int argc, py_Ref argv) {
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
        case tp_str: {
            // str to float
            c11_sv sv = py_tosv(py_arg(1));

            if(c11__sveq2(sv, "inf")) {
                py_newfloat(py_retval(), INFINITY);
                return true;
            }
            if(c11__sveq2(sv, "-inf")) {
                py_newfloat(py_retval(), -INFINITY);
                return true;
            }

            char* p_end;
            py_f64 float_out = strtod(sv.data, &p_end);
            if(p_end != sv.data + sv.size) return ValueError("invalid literal for float(): %q", sv);
            py_newfloat(py_retval(), float_out);
            return true;
        }
        default: return TypeError("float() argument must be a string or a real number");
    }
}

// tp_bool
static bool bool__new__(int argc, py_Ref argv) {
    assert(argc > 0);
    if(argc == 1) {
        py_newbool(py_retval(), false);
        return true;
    }
    if(argc == 2) {
        int res = py_bool(py_arg(1));
        if(res == -1) return false;
        py_newbool(py_retval(), res);
        return true;
    }
    return TypeError("bool() takes at most 1 argument");
}

static bool bool__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    bool res = py_tobool(argv);
    py_newint(py_retval(), res);
    return true;
}

static bool bool__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    bool res = py_tobool(argv);
    py_newstr(py_retval(), res ? "True" : "False");
    return true;
}

static bool bool__eq__(int argc, py_Ref argv) {
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

static bool bool__ne__(int argc, py_Ref argv) {
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

#define DEF_BOOL_BITWISE(name, op)                                                                 \
    static bool bool##name(int argc, py_Ref argv) {                                                \
        PY_CHECK_ARGC(2);                                                                          \
        bool lhs = py_tobool(&argv[0]);                                                            \
        if(argv[1].type == tp_bool) {                                                              \
            bool rhs = py_tobool(&argv[1]);                                                        \
            py_newbool(py_retval(), lhs op rhs);                                                   \
        } else {                                                                                   \
            py_newnotimplemented(py_retval());                                                     \
        }                                                                                          \
        return true;                                                                               \
    }

DEF_BOOL_BITWISE(__and__, &&)
DEF_BOOL_BITWISE(__or__, ||)
DEF_BOOL_BITWISE(__xor__, !=)

static bool bool__invert__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    bool val = py_tobool(&argv[0]);
    py_newbool(py_retval(), !val);
    return true;
}

void pk_number__register() {
    /****** tp_int & tp_float ******/
    py_bindmagic(tp_int, __add__, int__add__);
    py_bindmagic(tp_float, __add__, float__add__);
    py_bindmagic(tp_int, __sub__, int__sub__);
    py_bindmagic(tp_float, __sub__, float__sub__);
    py_bindmagic(tp_int, __mul__, int__mul__);
    py_bindmagic(tp_float, __mul__, float__mul__);

    py_bindmagic(tp_int, __eq__, int__eq__);
    py_bindmagic(tp_float, __eq__, float__eq__);
    py_bindmagic(tp_int, __ne__, int__ne__);
    py_bindmagic(tp_float, __ne__, float__ne__);
    py_bindmagic(tp_int, __lt__, int__lt__);
    py_bindmagic(tp_float, __lt__, float__lt__);
    py_bindmagic(tp_int, __le__, int__le__);
    py_bindmagic(tp_float, __le__, float__le__);
    py_bindmagic(tp_int, __gt__, int__gt__);
    py_bindmagic(tp_float, __gt__, float__gt__);
    py_bindmagic(tp_int, __ge__, int__ge__);
    py_bindmagic(tp_float, __ge__, float__ge__);

    // __neg__
    py_bindmagic(tp_int, __neg__, int__neg__);
    py_bindmagic(tp_float, __neg__, float__neg__);

    // __repr__
    py_bindmagic(tp_int, __repr__, int__repr__);
    py_bindmagic(tp_float, __repr__, float__repr__);

    // __hash__
    py_bindmagic(tp_int, __hash__, int__hash__);
    py_bindmagic(tp_float, __hash__, float__hash__);

    // __abs__
    py_bindmagic(tp_int, __abs__, int__abs__);
    py_bindmagic(tp_float, __abs__, float__abs__);

    // __new__
    py_bindmagic(tp_int, __new__, int__new__);
    py_bindmagic(tp_float, __new__, float__new__);

    // __truediv__
    py_bindmagic(tp_int, __truediv__, int__truediv__);
    py_bindmagic(tp_float, __truediv__, float__truediv__);

    // __pow__
    py_bindmagic(tp_int, __pow__, number__pow__);
    py_bindmagic(tp_float, __pow__, number__pow__);

    // __floordiv__ & __mod__ & __divmod__
    py_bindmagic(tp_int, __floordiv__, int__floordiv__);
    py_bindmagic(tp_int, __mod__, int__mod__);
    py_bindmagic(tp_int, __divmod__, int__divmod__);

    // fmod
    py_bindmagic(tp_float, __floordiv__, float__floordiv__);
    py_bindmagic(tp_float, __rfloordiv__, float__rfloordiv__);
    py_bindmagic(tp_float, __mod__, float__mod__);
    py_bindmagic(tp_float, __rmod__, float__rmod__);
    py_bindmagic(tp_float, __divmod__, float__divmod__);

    // int.__invert__ & int.<BITWISE OP>
    py_bindmagic(tp_int, __invert__, int__invert__);

    py_bindmagic(tp_int, __and__, int__and__);
    py_bindmagic(tp_int, __or__, int__or__);
    py_bindmagic(tp_int, __xor__, int__xor__);
    py_bindmagic(tp_int, __lshift__, int__lshift__);
    py_bindmagic(tp_int, __rshift__, int__rshift__);

    // int.bit_length
    py_bindmethod(tp_int, "bit_length", int_bit_length);

    /* tp_bool */
    py_bindmagic(tp_bool, __new__, bool__new__);
    py_bindmagic(tp_bool, __hash__, bool__hash__);
    py_bindmagic(tp_bool, __repr__, bool__repr__);
    py_bindmagic(tp_bool, __eq__, bool__eq__);
    py_bindmagic(tp_bool, __ne__, bool__ne__);
    py_bindmagic(tp_bool, __and__, bool__and__);
    py_bindmagic(tp_bool, __or__, bool__or__);
    py_bindmagic(tp_bool, __xor__, bool__xor__);
    py_bindmagic(tp_bool, __invert__, bool__invert__);
}

#undef DEF_NUM_BINARY_OP
#undef DEF_INT_BITWISE_OP
#undef DEF_BOOL_BITWISE