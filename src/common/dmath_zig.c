// Inverse trigonometric functions ported from the Zig standard library, which
// in turn ports them from musl (MIT licensed):
// https://github.com/ziglang/zig/blob/master/lib/std/math/
//
// These live in their own file so the provenance stays obvious: each function
// below is a line-by-line translation of the Zig source linked above it, and
// should be re-synced from there rather than hand-tuned.

#include "pocketpy/common/dmath.h"
#include <stdint.h>

// Same layout as dmath.c's Float64Bits; kept distinct because pocketpy builds
// all sources as a single unity translation unit.
union ZigF64 {
    double f;
    uint64_t i;
};

// https://github.com/ziglang/zig/blob/master/lib/std/math/asin.zig
static double zig_r64(double z) {
    const double pS0 = 1.66666666666666657415e-01;
    const double pS1 = -3.25565818622400915405e-01;
    const double pS2 = 2.01212532134862925881e-01;
    const double pS3 = -4.00555345006794114027e-02;
    const double pS4 = 7.91534994289814532176e-04;
    const double pS5 = 3.47933107596021167570e-05;
    const double qS1 = -2.40339491173441421878e+00;
    const double qS2 = 2.02094576023350569471e+00;
    const double qS3 = -6.88283971605453293030e-01;
    const double qS4 = 7.70381505559019352791e-02;

    double p = z * (pS0 + z * (pS1 + z * (pS2 + z * (pS3 + z * (pS4 + z * pS5)))));
    double q = 1.0 + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
    return p / q;
}

// https://github.com/ziglang/zig/blob/master/lib/std/math/asin.zig
double dmath_asin(double x) {
    if(!(x >= -1 && x <= 1)) return DMATH_NAN;
    const double pio2_hi = 1.57079632679489655800e+00;
    const double pio2_lo = 6.12323399573676603587e-17;

    union ZigF64 ux_union;
    ux_union.f = x;
    uint64_t ux = ux_union.i;
    uint32_t hx = (uint32_t)(ux >> 32);
    uint32_t ix = hx & 0x7FFFFFFF;

    /* |x| >= 1 or nan */
    if(ix >= 0x3FF00000) {
        uint32_t lx = (uint32_t)(ux & 0xFFFFFFFF);

        /* asin(1) = +-pi/2 with inexact */
        if(((ix - 0x3FF00000) | lx) == 0) {
            return x * pio2_hi + 0x1.0p-120;
        } else {
            return DMATH_NAN;
        }
    }

    /* |x| < 0.5 */
    if(ix < 0x3FE00000) {
        /* if 0x1p-1022 <= |x| < 0x1p-26 avoid raising overflow */
        if(ix < 0x3E500000 && ix >= 0x00100000) {
            return x;
        } else {
            return x + x * zig_r64(x * x);
        }
    }

    /* 1 > |x| >= 0.5 */
    double z = (1 - dmath_fabs(x)) * 0.5;
    double s = dmath_sqrt(z);
    double r = zig_r64(z);
    double fx;

    /* |x| > 0.975 */
    if(ix >= 0x3FEF3333) {
        fx = pio2_hi - 2 * (s + s * r);
    } else {
        union ZigF64 jx_union = {.f = s};
        uint64_t jx = jx_union.i;
        union ZigF64 df_union = {.i = jx & 0xFFFFFFFF00000000ULL};
        double df = df_union.f;
        double c = (z - df * df) / (s + df);
        fx = 0.5 * pio2_hi - (2 * s * r - (pio2_lo - 2 * c) - (0.5 * pio2_hi - 2 * df));
    }

    if(hx >> 31 != 0) {
        return -fx;
    } else {
        return fx;
    }
}

// https://github.com/ziglang/zig/blob/master/lib/std/math/acos.zig
double dmath_acos(double x) {
    const double pio2_hi = 1.57079632679489655800e+00;
    const double pio2_lo = 6.12323399573676603587e-17;

    union ZigF64 ux_union = {.f = x};
    uint64_t ux = ux_union.i;
    uint32_t hx = (uint32_t)(ux >> 32);
    uint32_t ix = hx & 0x7FFFFFFF;

    /* |x| >= 1 or nan */
    if(ix >= 0x3FF00000) {
        uint32_t lx = (uint32_t)(ux & 0xFFFFFFFF);

        /* acos(1) = 0, acos(-1) = pi */
        if(((ix - 0x3FF00000) | lx) == 0) {
            if(hx >> 31 != 0) {
                return 2 * pio2_hi + 0x1.0p-120;
            } else {
                return 0;
            }
        }

        return DMATH_NAN;
    }

    /* |x| < 0.5 */
    if(ix < 0x3FE00000) {
        /* |x| < 0x1p-57 */
        if(ix <= 0x3C600000) {
            return pio2_hi + 0x1.0p-120;
        } else {
            return pio2_hi - (x - (pio2_lo - x * zig_r64(x * x)));
        }
    }

    /* x < -0.5 */
    if(hx >> 31 != 0) {
        double z = (1.0 + x) * 0.5;
        double s = dmath_sqrt(z);
        double w = zig_r64(z) * s - pio2_lo;
        return 2 * (pio2_hi - (s + w));
    }

    /* x > 0.5 */
    double z = (1.0 - x) * 0.5;
    double s = dmath_sqrt(z);
    union ZigF64 jx_union = {.f = s};
    union ZigF64 df_union = {.i = jx_union.i & 0xFFFFFFFF00000000ULL};
    double df = df_union.f;
    double c = (z - df * df) / (s + df);
    double w = zig_r64(z) * s + c;
    return 2 * (df + w);
}

// https://github.com/ziglang/zig/blob/master/lib/std/math/atan.zig
double dmath_atan(double x) {
    static const double atanhi[] = {
        4.63647609000806093515e-01, /* atan(0.5)hi */
        7.85398163397448278999e-01, /* atan(1.0)hi */
        9.82793723247329054082e-01, /* atan(1.5)hi */
        1.57079632679489655800e+00, /* atan(inf)hi */
    };
    static const double atanlo[] = {
        2.26987774529616870924e-17, /* atan(0.5)lo */
        3.06161699786838301793e-17, /* atan(1.0)lo */
        1.39033110312309984516e-17, /* atan(1.5)lo */
        6.12323399573676603587e-17, /* atan(inf)lo */
    };
    static const double aT[] = {
        3.33333333333329318027e-01,
        -1.99999999998764832476e-01,
        1.42857142725034663711e-01,
        -1.11111104054623557880e-01,
        9.09088713343650656196e-02,
        -7.69187620504482999495e-02,
        6.66107313738753120669e-02,
        -5.83357013379057348645e-02,
        4.97687799461593236017e-02,
        -3.65315727442169155270e-02,
        1.62858201153657823623e-02,
    };

    union ZigF64 ux = {.f = x};
    uint32_t ix = (uint32_t)(ux.i >> 32);
    uint32_t sign = ix >> 31;
    int id;
    double z, w, s1, s2;

    ix &= 0x7FFFFFFF;

    /* |x| >= 2^66 */
    if(ix >= 0x44100000) {
        if(dmath_isnan(x)) return x;
        z = atanhi[3] + 0x1.0p-120;
        return sign != 0 ? -z : z;
    }

    /* |x| < 0.4375 */
    if(ix < 0x3FDC0000) {
        /* |x| < 0x1p-27 */
        if(ix < 0x3E400000) return x;
        id = -1;
    } else {
        x = dmath_fabs(x);
        /* |x| < 1.1875 */
        if(ix < 0x3FF30000) {
            /* 7/16 <= |x| < 11/16 */
            if(ix < 0x3FE60000) {
                id = 0;
                x = (2.0 * x - 1.0) / (2.0 + x);
            } else {
                /* 11/16 <= |x| < 19/16 */
                id = 1;
                x = (x - 1.0) / (x + 1.0);
            }
        } else {
            /* |x| < 2.4375 */
            if(ix < 0x40038000) {
                id = 2;
                x = (x - 1.5) / (1.0 + 1.5 * x);
            } else {
                /* 2.4375 <= |x| < 2^66 */
                id = 3;
                x = -1.0 / x;
            }
        }
    }

    z = x * x;
    w = z * z;
    s1 = z * (aT[0] + w * (aT[2] + w * (aT[4] + w * (aT[6] + w * (aT[8] + w * aT[10])))));
    s2 = w * (aT[1] + w * (aT[3] + w * (aT[5] + w * (aT[7] + w * aT[9]))));

    if(id < 0) return x - x * (s1 + s2);

    z = atanhi[id] - ((x * (s1 + s2) - atanlo[id]) - x);
    return sign != 0 ? -z : z;
}

// https://github.com/ziglang/zig/blob/master/lib/std/math/atan2.zig
double dmath_atan2(double y, double x) {
    const double pi = 3.1415926535897931160E+00;    /* 0x400921FB, 0x54442D18 */
    const double pi_lo = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

    double z;
    uint32_t m, lx, ly, ix, iy;

    if(dmath_isnan(x) || dmath_isnan(y)) return x + y;

    union ZigF64 ux = {.f = x}, uy = {.f = y};
    ix = (uint32_t)(ux.i >> 32);
    lx = (uint32_t)(ux.i & 0xFFFFFFFF);
    iy = (uint32_t)(uy.i >> 32);
    ly = (uint32_t)(uy.i & 0xFFFFFFFF);

    /* x = 1.0 */
    if(((ix - 0x3FF00000) | lx) == 0) return dmath_atan(y);

    m = ((iy >> 31) & 1) | ((ix >> 30) & 2); /* 2 * sign(x) + sign(y) */
    ix &= 0x7FFFFFFF;
    iy &= 0x7FFFFFFF;

    /* when y = 0 */
    if((iy | ly) == 0) {
        switch(m) {
            case 0:
            case 1: return y;    /* atan(+-0, +anything) = +-0 */
            case 2: return pi;   /* atan(+0, -anything) = pi */
            default: return -pi; /* atan(-0, -anything) = -pi */
        }
    }

    /* when x = 0 */
    if((ix | lx) == 0) return m & 1 ? -pi / 2 : pi / 2;

    /* when x is INF */
    if(ix == 0x7FF00000) {
        if(iy == 0x7FF00000) {
            switch(m) {
                case 0: return pi / 4;       /* atan(+INF, +INF) */
                case 1: return -pi / 4;      /* atan(-INF, +INF) */
                case 2: return 3 * pi / 4;   /* atan(+INF, -INF) */
                default: return -3 * pi / 4; /* atan(-INF, -INF) */
            }
        } else {
            switch(m) {
                case 0: return 0.0;  /* atan(+..., +INF) */
                case 1: return -0.0; /* atan(-..., +INF) */
                case 2: return pi;   /* atan(+..., -INF) */
                default: return -pi; /* atan(-..., -INF) */
            }
        }
    }

    /* |y/x| > 0x1p64 */
    if(ix + (64 << 20) < iy || iy == 0x7FF00000) return m & 1 ? -pi / 2 : pi / 2;

    /* z = atan(|y/x|) without spurious underflow */
    if((m & 2) && iy + (64 << 20) < ix) /* |y/x| < 0x1p-64, x < 0 */
        z = 0;
    else
        z = dmath_atan(dmath_fabs(y / x));

    switch(m) {
        case 0: return z;                 /* atan(+, +) */
        case 1: return -z;                /* atan(-, +) */
        case 2: return pi - (z - pi_lo);  /* atan(+, -) */
        default: return (z - pi_lo) - pi; /* atan(-, -) */
    }
}
