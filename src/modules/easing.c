#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

#include <math.h>

// https://easings.net/

const double kPi = 3.1415926545;

static double easeLinear(double x) { return x; }

static double easeInSine(double x) { return 1.0 - cos(x * kPi / 2); }

static double easeOutSine(double x) { return sin(x * kPi / 2); }

static double easeInOutSine(double x) { return -(cos(kPi * x) - 1) / 2; }

static double easeInQuad(double x) { return x * x; }

static double easeOutQuad(double x) { return 1 - pow(1 - x, 2); }

static double easeInOutQuad(double x) {
    if(x < 0.5) {
        return 2 * x * x;
    } else {
        return 1 - pow(-2 * x + 2, 2) / 2;
    }
}

static double easeInCubic(double x) { return x * x * x; }

static double easeOutCubic(double x) { return 1 - pow(1 - x, 3); }

static double easeInOutCubic(double x) {
    if(x < 0.5) {
        return 4 * x * x * x;
    } else {
        return 1 - pow(-2 * x + 2, 3) / 2;
    }
}

static double easeInQuart(double x) { return pow(x, 4); }

static double easeOutQuart(double x) { return 1 - pow(1 - x, 4); }

static double easeInOutQuart(double x) {
    if(x < 0.5) {
        return 8 * pow(x, 4);
    } else {
        return 1 - pow(-2 * x + 2, 4) / 2;
    }
}

static double easeInQuint(double x) { return pow(x, 5); }

static double easeOutQuint(double x) { return 1 - pow(1 - x, 5); }

static double easeInOutQuint(double x) {
    if(x < 0.5) {
        return 16 * pow(x, 5);
    } else {
        return 1 - pow(-2 * x + 2, 5) / 2;
    }
}

static double easeInExpo(double x) { return x == 0 ? 0 : pow(2, 10 * x - 10); }

static double easeOutExpo(double x) { return x == 1 ? 1 : 1 - pow(2, -10 * x); }

static double easeInOutExpo(double x) {
    if(x == 0) {
        return 0;
    } else if(x == 1) {
        return 1;
    } else if(x < 0.5) {
        return pow(2, 20 * x - 10) / 2;
    } else {
        return (2 - pow(2, -20 * x + 10)) / 2;
    }
}

static double easeInCirc(double x) { return 1 - sqrt(1 - pow(x, 2)); }

static double easeOutCirc(double x) { return sqrt(1 - pow(x - 1, 2)); }

static double easeInOutCirc(double x) {
    if(x < 0.5) {
        return (1 - sqrt(1 - pow(2 * x, 2))) / 2;
    } else {
        return (sqrt(1 - pow(-2 * x + 2, 2)) + 1) / 2;
    }
}

static double easeInBack(double x) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return c3 * x * x * x - c1 * x * x;
}

static double easeOutBack(double x) {
    const double c1 = 1.70158;
    const double c3 = c1 + 1;
    return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
}

static double easeInOutBack(double x) {
    const double c1 = 1.70158;
    const double c2 = c1 * 1.525;
    if(x < 0.5) {
        return (pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2;
    } else {
        return (pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    }
}

static double easeInElastic(double x) {
    const double c4 = (2 * kPi) / 3;
    if(x == 0) {
        return 0;
    } else if(x == 1) {
        return 1;
    } else {
        return -pow(2, 10 * x - 10) * sin((x * 10 - 10.75) * c4);
    }
}

static double easeOutElastic(double x) {
    const double c4 = (2 * kPi) / 3;
    if(x == 0) {
        return 0;
    } else if(x == 1) {
        return 1;
    } else {
        return pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
    }
}

static double easeInOutElastic(double x) {
    const double c5 = (2 * kPi) / 4.5;
    if(x == 0) {
        return 0;
    } else if(x == 1) {
        return 1;
    } else if(x < 0.5) {
        return -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2;
    } else {
        return (pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
    }
}

static double easeOutBounce(double x) {
    const double n1 = 7.5625;
    const double d1 = 2.75;
    if(x < 1 / d1) {
        return n1 * x * x;
    } else if(x < 2 / d1) {
        x -= 1.5 / d1;
        return n1 * x * x + 0.75;
    } else if(x < 2.5 / d1) {
        x -= 2.25 / d1;
        return n1 * x * x + 0.9375;
    } else {
        x -= 2.625 / d1;
        return n1 * x * x + 0.984375;
    }
}

static double easeInBounce(double x) { return 1 - easeOutBounce(1 - x); }

static double easeInOutBounce(double x) {
    return x < 0.5 ? (1 - easeOutBounce(1 - 2 * x)) / 2 : (1 + easeOutBounce(2 * x - 1)) / 2;
}

#define DEF_EASE(name)                                                                             \
    static bool easing_##name(int argc, py_Ref argv) {                                             \
        PY_CHECK_ARGC(1);                                                                          \
        py_f64 t;                                                                                  \
        if(!py_castfloat(argv, &t)) return false;                                                  \
        py_newfloat(py_retval(), ease##name(t));                                                   \
        return true;                                                                               \
    }

DEF_EASE(Linear)
DEF_EASE(InSine)
DEF_EASE(OutSine)
DEF_EASE(InOutSine)
DEF_EASE(InQuad)
DEF_EASE(OutQuad)
DEF_EASE(InOutQuad)
DEF_EASE(InCubic)
DEF_EASE(OutCubic)
DEF_EASE(InOutCubic)
DEF_EASE(InQuart)
DEF_EASE(OutQuart)
DEF_EASE(InOutQuart)
DEF_EASE(InQuint)
DEF_EASE(OutQuint)
DEF_EASE(InOutQuint)
DEF_EASE(InExpo)
DEF_EASE(OutExpo)
DEF_EASE(InOutExpo)
DEF_EASE(InCirc)
DEF_EASE(OutCirc)
DEF_EASE(InOutCirc)
DEF_EASE(InBack)
DEF_EASE(OutBack)
DEF_EASE(InOutBack)
DEF_EASE(InElastic)
DEF_EASE(OutElastic)
DEF_EASE(InOutElastic)
DEF_EASE(InBounce)
DEF_EASE(OutBounce)
DEF_EASE(InOutBounce)

#undef DEF_EASE

void pk__add_module_easing() {
    py_GlobalRef mod = py_newmodule("easing");

    py_bindfunc(mod, "Linear", easing_Linear);
    py_bindfunc(mod, "InSine", easing_InSine);
    py_bindfunc(mod, "OutSine", easing_OutSine);
    py_bindfunc(mod, "InOutSine", easing_InOutSine);
    py_bindfunc(mod, "InQuad", easing_InQuad);
    py_bindfunc(mod, "OutQuad", easing_OutQuad);
    py_bindfunc(mod, "InOutQuad", easing_InOutQuad);
    py_bindfunc(mod, "InCubic", easing_InCubic);
    py_bindfunc(mod, "OutCubic", easing_OutCubic);
    py_bindfunc(mod, "InOutCubic", easing_InOutCubic);
    py_bindfunc(mod, "InQuart", easing_InQuart);
    py_bindfunc(mod, "OutQuart", easing_OutQuart);
    py_bindfunc(mod, "InOutQuart", easing_InOutQuart);
    py_bindfunc(mod, "InQuint", easing_InQuint);
    py_bindfunc(mod, "OutQuint", easing_OutQuint);
    py_bindfunc(mod, "InOutQuint", easing_InOutQuint);
    py_bindfunc(mod, "InExpo", easing_InExpo);
    py_bindfunc(mod, "OutExpo", easing_OutExpo);
    py_bindfunc(mod, "InOutExpo", easing_InOutExpo);
    py_bindfunc(mod, "InCirc", easing_InCirc);
    py_bindfunc(mod, "OutCirc", easing_OutCirc);
    py_bindfunc(mod, "InOutCirc", easing_InOutCirc);
    py_bindfunc(mod, "InBack", easing_InBack);
    py_bindfunc(mod, "OutBack", easing_OutBack);
    py_bindfunc(mod, "InOutBack", easing_InOutBack);
    py_bindfunc(mod, "InElastic", easing_InElastic);
    py_bindfunc(mod, "OutElastic", easing_OutElastic);
    py_bindfunc(mod, "InOutElastic", easing_InOutElastic);
    py_bindfunc(mod, "InBounce", easing_InBounce);
    py_bindfunc(mod, "OutBounce", easing_OutBounce);
    py_bindfunc(mod, "InOutBounce", easing_InOutBounce);
}
