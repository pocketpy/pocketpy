#include "pocketpy/common/dmath.h"
#include "pocketpy/common/algorithm.h"
#include "pocketpy/common/_log_spline_tbl.h"
#include <stdint.h>

union Float64Bits {
    double f;
    uint64_t i;
};

/* IEEE 754 double precision floating point data manipulation */
typedef union 
{
    double   f;
    uint64_t u;
    struct {int32_t  i0,i1;} s;
}  udi_t;

// https://github.com/akohlmey/fastermath/blob/master/src/exp.c#L63
double dmath_exp2(double x) {
    if (x > 1000) return DMATH_INFINITY;
    if (x < -1000) return 0;
	if (dmath_isnan(x)) return DMATH_NAN;
    
    const int FM_DOUBLE_BIAS = 1023;

    static const double fm_exp2_q[] = {
    /*  1.00000000000000000000e0, */
        2.33184211722314911771e2,
        4.36821166879210612817e3
    };
    static const double fm_exp2_p[] = {
        2.30933477057345225087e-2,
        2.02020656693165307700e1,
        1.51390680115615096133e3
    };

    double   ipart, fpart, px, qx;
    udi_t    epart;

    ipart = dmath_floor(x+0.5);
    fpart = x - ipart;

    // FM_DOUBLE_INIT_EXP(epart,ipart);
    epart.s.i0 = 0;
    epart.s.i1 = (((int) ipart) + FM_DOUBLE_BIAS) << 20;

    x = fpart*fpart;

    px =        fm_exp2_p[0];
    px = px*x + fm_exp2_p[1];
    qx =    x + fm_exp2_q[0];
    px = px*x + fm_exp2_p[2];
    qx = qx*x + fm_exp2_q[1];

    px = px * fpart;

    x = 1.0 + 2.0*(px/(qx-px));
    return epart.f*x;
}

double dmath_log2(double x) {
	if(x < 0) return DMATH_NAN;
	if(x == 0) return -DMATH_INFINITY;
	if(x == DMATH_INFINITY) return DMATH_INFINITY;
	if(dmath_isnan(x)) return DMATH_NAN;

    const double fm_log_dinv =  4.09600000000000000000e+03;
    const double fm_log_dsq6 =  9.93410746256510361521e-09;

    const int FM_DOUBLE_BIAS = 1023;
    const int FM_DOUBLE_EMASK = 2146435072;
    const int FM_DOUBLE_MBITS = 20;
    const int FM_DOUBLE_MMASK = 1048575;
    const int FM_DOUBLE_EZERO = 1072693248;

    const int FM_SPLINE_SHIFT = 8;

    udi_t val;
    double a,b,y;
    int32_t hx, ipart;

    val.f = x;
    hx = val.s.i1;
    
    /* extract exponent and subtract bias */
    ipart = (((hx & FM_DOUBLE_EMASK) >> FM_DOUBLE_MBITS) - FM_DOUBLE_BIAS);

    /* mask out exponent to get the prefactor to 2**ipart */
    hx &= FM_DOUBLE_MMASK;
    val.s.i1 = hx | FM_DOUBLE_EZERO;
    x = val.f;

    /* table index */
    hx >>= FM_SPLINE_SHIFT;

    /* compute x value matching table index */
    val.s.i0 = 0;
    val.s.i1 = FM_DOUBLE_EZERO | (hx << FM_SPLINE_SHIFT);
    b = (x - val.f) * fm_log_dinv;
    a = 1.0 - b;

    /* evaluate spline */
    y = a * fm_log_q1[hx] + b * fm_log_q1[hx+1];
    a = (a*a*a-a) * fm_log_q2[hx];
    b = (b*b*b-b) * fm_log_q2[hx+1];
    y += (a + b) * fm_log_dsq6;

    return ((double) ipart) + (y * DMATH_LOG2_E);
}

double dmath_exp(double x) {
    return dmath_exp2(x * DMATH_LOG2_E); // log2(e)
}

double dmath_exp10(double x) {
    return dmath_exp2(x * 3.321928094887362); // log2(10)
}

double dmath_log(double x) {
    return dmath_log2(x) / DMATH_LOG2_E; // log2(e)
}

double dmath_log10(double x) {
    return dmath_log2(x) / 3.321928094887362; // log2(10)
}

double dmath_pow(double base, double exp) {
    int exp_int = (int)exp;
    if(exp_int == exp) {
        if(exp_int == 0) return 1;
        if(exp_int < 0) {
            base = 1 / base;
            exp_int = -exp_int;
        }
        double res = 1;
        while(exp_int > 0) {
            if(exp_int & 1) res *= base;
            base *= base;
            exp_int >>= 1;
        }
        return res;
    }
    if (base > 0) {
        return dmath_exp(exp * dmath_log(base));
    }
    if (base == 0) {
        if (exp > 0) return 0;
        if (exp == 0) return 1;
    }
    return DMATH_NAN;
}

double dmath_sqrt(double x) {
    return dmath_pow(x, 0.5);
}

double dmath_cbrt(double x) {
    return dmath_pow(x, 1.0 / 3.0);
}

// https://github.com/kraj/musl/blob/kraj/master/src/math/sincos.c
static double __sin(double x, double y, int iy)
{
static const double
S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

	double z,r,v,w;

	z = x*x;
	w = z*z;
	r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6);
	v = z*x;
	if (iy == 0)
		return x + v*(S1 + z*r);
	else
		return x - ((z*(0.5*y - v*r) - y) - v*S1);
}

static double __cos(double x, double y)
{
static const double
C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

	double hz,z,r,w;

	z  = x*x;
	w  = z*z;
	r  = z*(C1+z*(C2+z*C3)) + w*w*(C4+z*(C5+z*C6));
	hz = 0.5*z;
	w  = 1.0-hz;
	return w + (((1.0-w)-hz) + (z*r-x*y));
}

int __rem_pio2(double x, double *y)
{
static const double
toint   = 1.5/2.22044604925031308085e-16,
pio4    = 0x1.921fb54442d18p-1,
invpio2 = 6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
pio2_1  = 1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
pio2_1t = 6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
pio2_2  = 6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
pio2_2t = 2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
pio2_3  = 2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
pio2_3t = 8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

	union Float64Bits u = { .f = x };
	double z,w,t,r,fn;
	double tx[3],ty[2];
	uint32_t ix;
	int sign, n, ex, ey, i;

	sign = u.i>>63;
	ix = u.i>>32 & 0x7fffffff;
	if (ix <= 0x400f6a7a) {  /* |x| ~<= 5pi/4 */
		if ((ix & 0xfffff) == 0x921fb)  /* |x| ~= pi/2 or 2pi/2 */
			goto medium;  /* cancellation -- use medium case */
		if (ix <= 0x4002d97c) {  /* |x| ~<= 3pi/4 */
			if (!sign) {
				z = x - pio2_1;  /* one round good to 85 bits */
				y[0] = z - pio2_1t;
				y[1] = (z-y[0]) - pio2_1t;
				return 1;
			} else {
				z = x + pio2_1;
				y[0] = z + pio2_1t;
				y[1] = (z-y[0]) + pio2_1t;
				return -1;
			}
		} else {
			if (!sign) {
				z = x - 2*pio2_1;
				y[0] = z - 2*pio2_1t;
				y[1] = (z-y[0]) - 2*pio2_1t;
				return 2;
			} else {
				z = x + 2*pio2_1;
				y[0] = z + 2*pio2_1t;
				y[1] = (z-y[0]) + 2*pio2_1t;
				return -2;
			}
		}
	}
	if (ix <= 0x401c463b) {  /* |x| ~<= 9pi/4 */
		if (ix <= 0x4015fdbc) {  /* |x| ~<= 7pi/4 */
			if (ix == 0x4012d97c)  /* |x| ~= 3pi/2 */
				goto medium;
			if (!sign) {
				z = x - 3*pio2_1;
				y[0] = z - 3*pio2_1t;
				y[1] = (z-y[0]) - 3*pio2_1t;
				return 3;
			} else {
				z = x + 3*pio2_1;
				y[0] = z + 3*pio2_1t;
				y[1] = (z-y[0]) + 3*pio2_1t;
				return -3;
			}
		} else {
			if (ix == 0x401921fb)  /* |x| ~= 4pi/2 */
				goto medium;
			if (!sign) {
				z = x - 4*pio2_1;
				y[0] = z - 4*pio2_1t;
				y[1] = (z-y[0]) - 4*pio2_1t;
				return 4;
			} else {
				z = x + 4*pio2_1;
				y[0] = z + 4*pio2_1t;
				y[1] = (z-y[0]) + 4*pio2_1t;
				return -4;
			}
		}
	}
	if (ix < 0x413921fb) {  /* |x| ~< 2^20*(pi/2), medium size */
medium:
		/* rint(x/(pi/2)) */
		fn = (double)x*invpio2 + toint - toint;
		n = (int32_t)fn;
		r = x - fn*pio2_1;
		w = fn*pio2_1t;  /* 1st round, good to 85 bits */
		/* Matters with directed rounding. */
		if ((r - w < -pio4)) {
			n--;
			fn--;
			r = x - fn*pio2_1;
			w = fn*pio2_1t;
		} else if ((r - w > pio4)) {
			n++;
			fn++;
			r = x - fn*pio2_1;
			w = fn*pio2_1t;
		}
		y[0] = r - w;
		u.f = y[0];
		ey = u.i>>52 & 0x7ff;
		ex = ix>>20;
		if (ex - ey > 16) { /* 2nd round, good to 118 bits */
			t = r;
			w = fn*pio2_2;
			r = t - w;
			w = fn*pio2_2t - ((t-r)-w);
			y[0] = r - w;
			u.f = y[0];
			ey = u.i>>52 & 0x7ff;
			if (ex - ey > 49) {  /* 3rd round, good to 151 bits, covers all cases */
				t = r;
				w = fn*pio2_3;
				r = t - w;
				w = fn*pio2_3t - ((t-r)-w);
				y[0] = r - w;
			}
		}
		y[1] = (r - y[0]) - w;
		return n;
	}

    (void)tx;
    (void)ty;
    (void)i;
    return 0;
#if 0
	/*
	 * all other (large) arguments
	 */
	if (ix >= 0x7ff00000) {  /* x is inf or NaN */
		y[0] = y[1] = x - x;
		return 0;
	}
	/* set z = scalbn(|x|,-ilogb(x)+23) */
	u.f = x;
	u.i &= (uint64_t)-1>>12;
	u.i |= (uint64_t)(0x3ff + 23)<<52;
	z = u.f;
	for (i=0; i < 2; i++) {
		tx[i] = (double)(int32_t)z;
		z     = (z-tx[i])*0x1p24;
	}
	tx[i] = z;
	/* skip zero terms, first term is non-zero */
	while (tx[i] == 0.0)
		i--;
	n = __rem_pio2_large(tx,ty,(int)(ix>>20)-(0x3ff+23),i+1,1);
	if (sign) {
		y[0] = -ty[0];
		y[1] = -ty[1];
		return -n;
	}
	y[0] = ty[0];
	y[1] = ty[1];
	return n;
#endif
}

void dmath_sincos(double x, double *sin, double *cos) {
	double y[2], s, c;
	uint32_t ix;
	unsigned n;

	//GET_HIGH_WORD(ix, x);
    union Float64Bits u = { .f = x };
    ix = (uint32_t)(u.i >> 32);

	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		/* if |x| < 2**-27 * sqrt(2) */
		if (ix < 0x3e46a09e) {
			/* raise inexact if x!=0 and underflow if subnormal */

			// FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
            volatile double y_force_eval;
            y_force_eval = ix < 0x00100000 ? x/0x1p120f : x+0x1p120f;
            (void)y_force_eval;

			*sin = x;
			*cos = 1.0;
			return;
		}
		*sin = __sin(x, 0.0, 0);
		*cos = __cos(x, 0.0);
		return;
	}

	/* sincos(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000) {
		*sin = *cos = x - x;
		return;
	}

	/* argument reduction needed */
	n = __rem_pio2(x, y);
	s = __sin(y[0], y[1], 1);
	c = __cos(y[0], y[1]);
	switch (n&3) {
	case 0:
		*sin = s;
		*cos = c;
		break;
	case 1:
		*sin = c;
		*cos = -s;
		break;
	case 2:
		*sin = -s;
		*cos = -c;
		break;
	case 3:
	default:
		*sin = -c;
		*cos = s;
		break;
	}
}

double dmath_sin(double x) {
    double s, c;
    dmath_sincos(x, &s, &c);
    return s;
}

double dmath_cos(double x) {
    double s, c;
    dmath_sincos(x, &s, &c);
    return c;
}

double dmath_tan(double x) {
    double s, c;
    dmath_sincos(x, &s, &c);
    return s / c;
}

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
    const double pio2_hi = 1.57079632679489655800e+00;
    const double pio2_lo = 6.12323399573676603587e-17;

    union Float64Bits ux_union;
    ux_union.f = x;
    uint64_t ux = ux_union.i;
    uint32_t hx = (uint32_t)(ux >> 32);
    uint32_t ix = hx & 0x7FFFFFFF;

    /* |x| >= 1 or nan */
    if (ix >= 0x3FF00000) {
        uint32_t lx = (uint32_t)(ux & 0xFFFFFFFF);

        /* asin(1) = +-pi/2 with inexact */
        if (((ix - 0x3FF00000) | lx) == 0) {
            return x * pio2_hi + 0x1.0p-120;
        } else {
            return DMATH_NAN;
        }
    }

    /* |x| < 0.5 */
    if (ix < 0x3FE00000) {
        /* if 0x1p-1022 <= |x| < 0x1p-26 avoid raising overflow */
        if (ix < 0x3E500000 && ix >= 0x00100000) {
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
    if (ix >= 0x3FEF3333) {
        fx = pio2_hi - 2 * (s + s * r);
    } else {
        union Float64Bits jx_union = { .f = s };
        uint64_t jx = jx_union.i;
        union Float64Bits df_union = { .i = jx & 0xFFFFFFFF00000000ULL };
        double df = df_union.f;
        double c = (z - df * df) / (s + df);
        fx = 0.5 * pio2_hi - (2 * s * r - (pio2_lo - 2 * c) - (0.5 * pio2_hi - 2 * df));
    }

    if (hx >> 31 != 0) {
        return -fx;
    } else {
        return fx;
    }
}

double dmath_acos(double x) {
    return DMATH_PI / 2 - dmath_asin(x);
}

double dmath_atan(double x) {
    return dmath_asin(x / dmath_sqrt(1 + x * x));
}

double dmath_atan2(double y, double x) {
    if (x > 0) {
        return dmath_atan(y / x);
    } else if (x < 0 && y >= 0) {
        return dmath_atan(y / x) + DMATH_PI;
    } else if (x < 0 && y < 0) {
        return dmath_atan(y / x) - DMATH_PI;
    } else if (x == 0 && y > 0) {
        return DMATH_PI / 2;
    } else if (x == 0 && y < 0) {
        return -DMATH_PI / 2;
    } else {
        return DMATH_NAN;
    }
}

////////////////////////////////////////////////////////////////////

int dmath_isinf(double x) {
    union Float64Bits u = { .f = x };
    return (u.i & -1ULL>>1) == 0x7ffULL<<52;
}

int dmath_isnan(double x) {
    union Float64Bits u = { .f = x };
    return (u.i & -1ULL>>1) > 0x7ffULL<<52;
}

int dmath_isnormal(double x) {
    union Float64Bits u = { .f = x };
    return ((u.i+(1ULL<<52)) & -1ULL>>1) >= 1ULL<<53;
}

int dmath_isfinite(double x) {
    union Float64Bits u = { .f = x };
    return (u.i & -1ULL>>1) < 0x7ffULL<<52;
}

// https://github.com/kraj/musl/blob/kraj/master/src/math/fmod.c
double dmath_fmod(double x, double y) {
	union Float64Bits ux = { .f = x }, uy = { .f = y };
	int ex = ux.i>>52 & 0x7ff;
	int ey = uy.i>>52 & 0x7ff;
	int sx = ux.i>>63;
	uint64_t i;

	/* in the followings uxi should be ux.i, but then gcc wrongly adds */
	/* float load/store to inner loops ruining performance and code size */
	uint64_t uxi = ux.i;

	if (uy.i<<1 == 0 || dmath_isnan(y) || ex == 0x7ff)
		return (x*y)/(x*y);
	if (uxi<<1 <= uy.i<<1) {
		if (uxi<<1 == uy.i<<1)
			return 0*x;
		return x;
	}

	/* normalize x and y */
	if (!ex) {
		for (i = uxi<<12; i>>63 == 0; ex--, i <<= 1);
		uxi <<= -ex + 1;
	} else {
		uxi &= -1ULL >> 12;
		uxi |= 1ULL << 52;
	}
	if (!ey) {
		for (i = uy.i<<12; i>>63 == 0; ey--, i <<= 1);
		uy.i <<= -ey + 1;
	} else {
		uy.i &= -1ULL >> 12;
		uy.i |= 1ULL << 52;
	}

	/* x mod y */
	for (; ex > ey; ex--) {
		i = uxi - uy.i;
		if (i >> 63 == 0) {
			if (i == 0)
				return 0*x;
			uxi = i;
		}
		uxi <<= 1;
	}
	i = uxi - uy.i;
	if (i >> 63 == 0) {
		if (i == 0)
			return 0*x;
		uxi = i;
	}
	for (; uxi>>52 == 0; uxi <<= 1, ex--);

	/* scale result */
	if (ex > 0) {
		uxi -= 1ULL << 52;
		uxi |= (uint64_t)ex << 52;
	} else {
		uxi >>= -ex + 1;
	}
	uxi |= (uint64_t)sx << 63;
	ux.i = uxi;
	return ux.f;
}

// https://github.com/kraj/musl/blob/kraj/master/src/math/copysign.c
double dmath_copysign(double x, double y) {
	union Float64Bits ux = { .f = x }, uy = { .f = y };
	ux.i &= -1ULL/2;
	ux.i |= uy.i & 1ULL<<63;
	return ux.f;
}

double dmath_fabs(double x) {
    return (x < 0) ? -x : x;
}

double dmath_ceil(double x) {
	if(!dmath_isfinite(x)) return x;
    int int_part = (int)x;
    if (x > 0 && x != (double)int_part) {
        return (double)(int_part + 1);
    }
    return (double)int_part;
}

double dmath_floor(double x) {
	if(!dmath_isfinite(x)) return x;
    int int_part = (int)x;
    if (x < 0 && x != (double)int_part) {
        return (double)(int_part - 1);
    }
    return (double)int_part;
}

double dmath_trunc(double x) {
    return (double)((int)x);
}

// https://github.com/kraj/musl/blob/kraj/master/src/math/modf.c
double dmath_modf(double x, double* iptr) {
	union Float64Bits u = { .f = x };
	uint64_t mask;
	int e = (int)(u.i>>52 & 0x7ff) - 0x3ff;

	/* no fractional part */
	if (e >= 52) {
		*iptr = x;
		if (e == 0x400 && u.i<<12 != 0) /* nan */
			return x;
		u.i &= 1ULL<<63;
		return u.f;
	}

	/* no integral part*/
	if (e < 0) {
		u.i &= 1ULL<<63;
		*iptr = u.f;
		return x;
	}

	mask = -1ULL>>12>>e;
	if ((u.i & mask) == 0) {
		*iptr = x;
		u.i &= 1ULL<<63;
		return u.f;
	}
	u.i &= ~mask;
	*iptr = u.f;
	return x - u.f;
}

double dmath_fmin(double x, double y) {
    return (x < y) ? x : y;
}

double dmath_fmax(double x, double y) {
    return (x > y) ? x : y;
}
