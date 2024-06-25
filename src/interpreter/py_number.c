#include "pocketpy/interpreter/vm.h"

#include <math.h>

// static int _py_print(const py_Ref args, int argc){
//     int length = py_tuple__len(args+0);
//     py_Str* sep;
//     py_Str* end;

//     int err;
//     err = py_tostr(args+1, &sep);
//     if(err) return err;
//     err = py_tostr(args+2, &end);
//     if(err) return err;

//     pk_SStream ss;
//     pk_SStream__ctor(&ss);

//     for(int i=0; i<length; i++){
//         const py_Ref item = py_tuple__getitem(args+0, i);
//         py_Str tmp;
//         int err = py_str(item, &tmp);
//         if(!err){
//             pk_SStream__write_Str(&ss, &tmp);
//             py_Str__dtor(&tmp);
//             if(i != length-1){
//                 pk_SStream__write_Str(&ss, sep);
//             }
//         }else{
//             py_Str__dtor(&tmp);
//             pk_SStream__dtor(&ss);
//             return err;
//         }
//     }
//     pk_SStream__write_Str(&ss, end);
//     py_Str out = pk_SStream__submit(&ss);
//     pk_current_vm->_stdout(py_Str__data(&out));
//     py_Str__dtor(&out);
//     return 0;
// }

#define DEF_NUM_BINARY_OP(name, op)                                                                \
    static int _py_int##name(int argc, py_Ref argv) {                                              \
        if(py_isint(&argv[1])) {                                                                   \
            int64_t lhs = py_toint(&argv[0]);                                                      \
            int64_t rhs = py_toint(&argv[1]);                                                      \
            py_pushint(lhs op rhs);                                                                \
        } else if(py_isfloat(&argv[1])) {                                                          \
            int64_t lhs = py_toint(&argv[0]);                                                      \
            double rhs = py_tofloat(&argv[1]);                                                     \
            py_pushfloat(lhs op rhs);                                                              \
        } else {                                                                                   \
            py_push_notimplemented();                                                              \
        }                                                                                          \
        return 1;                                                                                  \
    }                                                                                              \
    static int _py_float##name(int argc, py_Ref argv) {                                            \
        double lhs = py_tofloat(&argv[0]);                                                         \
        double rhs;                                                                                \
        if(py_castfloat(&argv[1], &rhs)) {                                                         \
            py_pushfloat(lhs op rhs);                                                              \
        } else {                                                                                   \
            py_push_notimplemented();                                                              \
        }                                                                                          \
        return 1;                                                                                  \
    }

DEF_NUM_BINARY_OP(__add__, +)
DEF_NUM_BINARY_OP(__sub__, -)
DEF_NUM_BINARY_OP(__mul__, *)

DEF_NUM_BINARY_OP(__eq__, ==)
DEF_NUM_BINARY_OP(__lt__, <)
DEF_NUM_BINARY_OP(__le__, <=)
DEF_NUM_BINARY_OP(__gt__, >)
DEF_NUM_BINARY_OP(__ge__, >=)

#undef DEF_NUM_BINARY_OP

static int _py_int__neg__(int argc, py_Ref argv) {
    int64_t val = py_toint(&argv[0]);
    py_pushint(-val);
    return 1;
}

static int _py_float__neg__(int argc, py_Ref argv) {
    double val = py_tofloat(&argv[0]);
    py_pushfloat(-val);
    return 1;
}

static int _py_int__truediv__(int argc, py_Ref argv) {
    int64_t lhs = py_toint(&argv[0]);
    double rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_pushfloat(lhs / rhs);
    } else {
        py_push_notimplemented();
    }
    return 1;
}

static int _py_float__truediv__(int argc, py_Ref argv) {
    double lhs = py_tofloat(&argv[0]);
    double rhs;
    if(py_castfloat(&argv[1], &rhs)) {
        py_pushfloat(lhs / rhs);
    } else {
        py_push_notimplemented();
    }
    return 1;
}

static int _py_number__pow__(int argc, py_Ref argv) {
    if(py_isint(&argv[0]) && py_isint(&argv[1])) {
        int64_t lhs = py_toint(&argv[0]);
        int64_t rhs = py_toint(&argv[1]);
        if(rhs < 0) {
            if(lhs == 0) {
                // py_pusherror("0.0 cannot be raised to a negative power");
                // TODO: ZeroDivisionError
                return -1;
            } else {
                py_pushfloat(pow(lhs, rhs));
            }
        } else {
            int64_t ret = 1;
            while(rhs) {
                if(rhs & 1) ret *= lhs;
                lhs *= lhs;
                rhs >>= 1;
            }
            py_pushint(ret);
        }
    } else {
        double lhs, rhs;
        py_castfloat(&argv[0], &lhs);
        if(py_castfloat(&argv[1], &rhs)) {
            py_pushfloat(pow(lhs, rhs));
        } else {
            py_push_notimplemented();
        }
    }
    return 1;
}

static int _py_int__floordiv__(int argc, py_Ref argv) {
    int64_t lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        int64_t rhs = py_toint(&argv[1]);
        if(rhs == 0) return -1;
        py_pushint(lhs / rhs);
    } else {
        py_push_notimplemented();
    }
    return 1;
}

static int _py_int__mod__(int argc, py_Ref argv) {
    int64_t lhs = py_toint(&argv[0]);
    if(py_isint(&argv[1])) {
        int64_t rhs = py_toint(&argv[1]);
        if(rhs == 0) return -1;
        py_pushint(lhs % rhs);
    } else {
        py_push_notimplemented();
    }
    return 1;
}

static int _py_int__invert__(int argc, py_Ref argv) {
    int64_t val = py_toint(&argv[0]);
    py_pushint(~val);
    return 1;
}

static int _py_int__bit_length(int argc, py_Ref argv) {
    int64_t x = py_toint(&argv[0]);
    if(x < 0) x = -x;
    int bits = 0;
    while(x) {
        x >>= 1;
        bits++;
    }
    py_pushint(bits);
    return 1;
}

#define DEF_INT_BITWISE_OP(name, op)                                                               \
    static int _py_int##name(int argc, py_Ref argv) {                                              \
        int64_t lhs = py_toint(&argv[0]);                                                          \
        if(py_isint(&argv[1])) {                                                                   \
            int64_t rhs = py_toint(&argv[1]);                                                      \
            py_pushint(lhs op rhs);                                                                \
        } else {                                                                                   \
            py_push_notimplemented();                                                              \
        }                                                                                          \
        return 1;                                                                                  \
    }

DEF_INT_BITWISE_OP(__and__, &)
DEF_INT_BITWISE_OP(__or__, |)
DEF_INT_BITWISE_OP(__xor__, ^)
DEF_INT_BITWISE_OP(__lshift__, <<)
DEF_INT_BITWISE_OP(__rshift__, >>)

#undef DEF_INT_BITWISE_OP

void pk_VM__init_builtins(pk_VM* self) {
    /****** tp_int & tp_float ******/
    py_Ref tmp = py_pushtmp();
    py_Ref int_type = py_pushtmp();
    *int_type = *py_getdict(&self->builtins, py_name("int"));
    py_Ref float_type = py_pushtmp();
    *float_type = *py_getdict(&self->builtins, py_name("float"));

#define BIND_INT_BINARY_OP(name)                                                                   \
    py_newnativefunc(tmp, _py_int##name, 2);                                                       \
    py_setdict(int_type, name, tmp);

#define BIND_FLOAT_BINARY_OP(name)                                                                 \
    py_newnativefunc(tmp, _py_float##name, 2);                                                     \
    py_setdict(float_type, name, tmp);

    BIND_INT_BINARY_OP(__add__);
    BIND_FLOAT_BINARY_OP(__add__);
    BIND_INT_BINARY_OP(__sub__);
    BIND_FLOAT_BINARY_OP(__sub__);
    BIND_INT_BINARY_OP(__mul__);
    BIND_FLOAT_BINARY_OP(__mul__);

    BIND_INT_BINARY_OP(__eq__);
    BIND_FLOAT_BINARY_OP(__eq__);
    BIND_INT_BINARY_OP(__lt__);
    BIND_FLOAT_BINARY_OP(__lt__);
    BIND_INT_BINARY_OP(__le__);
    BIND_FLOAT_BINARY_OP(__le__);
    BIND_INT_BINARY_OP(__gt__);
    BIND_FLOAT_BINARY_OP(__gt__);
    BIND_INT_BINARY_OP(__ge__);
    BIND_FLOAT_BINARY_OP(__ge__);

    // __neg__
    py_newnativefunc(tmp, _py_int__neg__, 1);
    py_setdict(int_type, __neg__, tmp);
    py_newnativefunc(tmp, _py_float__neg__, 1);
    py_setdict(float_type, __neg__, tmp);

    // TODO: __repr__, __new__, __hash__

    // __truediv__
    py_newnativefunc(tmp, _py_int__truediv__, 2);
    py_setdict(int_type, __truediv__, tmp);
    py_newnativefunc(tmp, _py_float__truediv__, 2);
    py_setdict(float_type, __truediv__, tmp);

    // __pow__
    py_newnativefunc(tmp, _py_number__pow__, 2);
    py_setdict(int_type, __pow__, tmp);
    py_setdict(float_type, __pow__, tmp);

    // __floordiv__ & __mod__
    py_newnativefunc(tmp, _py_int__floordiv__, 2);
    py_setdict(int_type, __floordiv__, tmp);
    py_newnativefunc(tmp, _py_int__mod__, 2);
    py_setdict(int_type, __mod__, tmp);

    // int.__invert__ & int.<BITWISE OP>
    py_newnativefunc(tmp, _py_int__invert__, 1);
    py_setdict(int_type, __invert__, tmp);

    BIND_INT_BINARY_OP(__and__);
    BIND_INT_BINARY_OP(__or__);
    BIND_INT_BINARY_OP(__xor__);
    BIND_INT_BINARY_OP(__lshift__);
    BIND_INT_BINARY_OP(__rshift__);

    // int.bit_length
    py_newnativefunc(tmp, _py_int__bit_length, 1);
    py_setdict(int_type, py_name("bit_length"), tmp);

#undef BIND_INT_BINARY_OP
#undef BIND_FLOAT_BINARY_OP

    py_poptmp(3);

    // py_Ref builtins = py_getmodule("builtins");
    // py_newfunction(py_reg(0), _py_print,
    //     "print(*args, sep=' ', end='\\n')",
    //     BindType_FUNCTION
    // );
    // py_setdict(builtins, py_name("hello"), py_reg(0));
}