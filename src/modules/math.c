#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

#include <math.h>

#if PK_ENABLE_DETERMINISM
    #ifndef _DMATH_H
        #error "_DMATH_H not defined"
    #endif
#endif

#define ONE_ARG_FUNC(name, func)                                                                   \
    static bool math_##name(int argc, py_Ref argv) {                                               \
        PY_CHECK_ARGC(1);                                                                          \
        double x;                                                                                  \
        if(!py_castfloat(py_arg(0), &x)) return false;                                             \
        py_newfloat(py_retval(), func(x));                                                         \
        return true;                                                                               \
    }

#define TWO_ARG_FUNC(name, func)                                                                   \
    static bool math_##name(int argc, py_Ref argv) {                                               \
        PY_CHECK_ARGC(2);                                                                          \
        double x, y;                                                                               \
        if(!py_castfloat(py_arg(0), &x)) return false;                                             \
        if(!py_castfloat(py_arg(1), &y)) return false;                                             \
        py_newfloat(py_retval(), func(x, y));                                                      \
        return true;                                                                               \
    }

ONE_ARG_FUNC(ceil, ceil)
ONE_ARG_FUNC(fabs, fabs)
ONE_ARG_FUNC(floor, floor)
ONE_ARG_FUNC(trunc, trunc)

static bool math_fsum(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_list);
    py_Ref list = py_arg(0);
    double sum = 0;
    double c = 0;
    for(int i = 0; i < py_list_len(list); i++) {
        py_Ref item = py_list_getitem(list, i);
        double x;
        if(!py_castfloat(item, &x)) return false;
        double y = x - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    py_newfloat(py_retval(), sum);
    return true;
}

static bool math_gcd(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    py_i64 a = py_toint(py_arg(0));
    py_i64 b = py_toint(py_arg(1));
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b != 0) {
        py_i64 t = b;
        b = a % b;
        a = t;
    }
    py_newint(py_retval(), a);
    return true;
}

ONE_ARG_FUNC(isfinite, isfinite)
ONE_ARG_FUNC(isinf, isinf)
ONE_ARG_FUNC(isnan, isnan)

static bool math_isclose(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    double a, b;
    if(!py_castfloat(py_arg(0), &a)) return false;
    if(!py_castfloat(py_arg(1), &b)) return false;
    py_newbool(py_retval(), fabs(a - b) < 1e-9);
    return true;
}

ONE_ARG_FUNC(exp, exp)

static bool math_log(int argc, py_Ref argv) {
    double x;
    if(!py_castfloat(py_arg(0), &x)) return false;
    if(argc == 1) {
        py_newfloat(py_retval(), log(x));
    } else if(argc == 2) {
        double base;
        if(!py_castfloat(py_arg(1), &base)) return false;
        py_newfloat(py_retval(), log(x) / log(base));
    } else {
        return TypeError("log() takes 1 or 2 arguments");
    }
    return true;
}

ONE_ARG_FUNC(log2, log2)
ONE_ARG_FUNC(log10, log10)

TWO_ARG_FUNC(pow, pow)

ONE_ARG_FUNC(sqrt, sqrt)

ONE_ARG_FUNC(acos, acos)
ONE_ARG_FUNC(asin, asin)
ONE_ARG_FUNC(atan, atan)

ONE_ARG_FUNC(cos, cos)
ONE_ARG_FUNC(sin, sin)
ONE_ARG_FUNC(tan, tan)

TWO_ARG_FUNC(atan2, atan2)

static bool math_degrees(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    double x;
    if(!py_castfloat(py_arg(0), &x)) return false;
    py_newfloat(py_retval(), x * PK_M_RAD2DEG);
    return true;
}

static bool math_radians(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    double x;
    if(!py_castfloat(py_arg(0), &x)) return false;
    py_newfloat(py_retval(), x * PK_M_DEG2RAD);
    return true;
}

TWO_ARG_FUNC(fmod, fmod)
TWO_ARG_FUNC(copysign, copysign)

static bool math_modf(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    double i;
    double f = modf(py_tofloat(py_arg(0)), &i);
    py_Ref p = py_newtuple(py_retval(), 2);
    py_newfloat(&p[0], f);
    py_newfloat(&p[1], i);
    return true;
}

static bool math_factorial(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    py_i64 n = py_toint(py_arg(0));
    if(n < 0) return ValueError("factorial() not defined for negative values");
    py_i64 r = 1;
    for(py_i64 i = 2; i <= n; i++)
        r *= i;
    py_newint(py_retval(), r);
    return true;
}

void pk__add_module_math() {
    py_Ref mod = py_newmodule("math");

    py_newfloat(py_emplacedict(mod, py_name("pi")), PK_M_PI);
    py_newfloat(py_emplacedict(mod, py_name("e")), PK_M_E);
    py_newfloat(py_emplacedict(mod, py_name("inf")), INFINITY);
    py_newfloat(py_emplacedict(mod, py_name("nan")), NAN);

    py_bindfunc(mod, "ceil", math_ceil);
    py_bindfunc(mod, "fabs", math_fabs);
    py_bindfunc(mod, "floor", math_floor);
    py_bindfunc(mod, "trunc", math_trunc);

    py_bindfunc(mod, "fsum", math_fsum);
    py_bindfunc(mod, "gcd", math_gcd);

    py_bindfunc(mod, "isfinite", math_isfinite);
    py_bindfunc(mod, "isinf", math_isinf);
    py_bindfunc(mod, "isnan", math_isnan);
    py_bindfunc(mod, "isclose", math_isclose);

    py_bindfunc(mod, "exp", math_exp);
    py_bindfunc(mod, "log", math_log);
    py_bindfunc(mod, "log2", math_log2);
    py_bindfunc(mod, "log10", math_log10);

    py_bindfunc(mod, "pow", math_pow);
    py_bindfunc(mod, "sqrt", math_sqrt);

    py_bindfunc(mod, "acos", math_acos);
    py_bindfunc(mod, "asin", math_asin);
    py_bindfunc(mod, "atan", math_atan);

    py_bindfunc(mod, "cos", math_cos);
    py_bindfunc(mod, "sin", math_sin);
    py_bindfunc(mod, "tan", math_tan);

    py_bindfunc(mod, "atan2", math_atan2);

    py_bindfunc(mod, "degrees", math_degrees);
    py_bindfunc(mod, "radians", math_radians);

    py_bindfunc(mod, "fmod", math_fmod);
    py_bindfunc(mod, "modf", math_modf);
    py_bindfunc(mod, "copysign", math_copysign);
    py_bindfunc(mod, "factorial", math_factorial);
}

#undef ONE_ARG_FUNC
#undef TWO_ARG_FUNC