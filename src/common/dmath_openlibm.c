/*
 * pocketpy's fixed binary64 mathematical kernels.
 * Derived from OpenLibm v0.8.8, commit
 * 5fe399749f9276eaa0b8403e507470da05cbbb3f.
 * See 3rd/openlibm/README.md for provenance and the numerical contract.
 * Original notices are retained beside each kernel below.
 *
 * Local changes: private names, function-local constants, endian-independent
 * word access, defined integer arithmetic, and canonical NaNs.
 * The approximation coefficients and floating-point evaluation order are fixed.
 */
#include "pocketpy/common/dmath.h"
#include <float.h>
#include <stdint.h>
#include <string.h>

// Floating-point options are supplied by the build configuration.

static uint64_t dmath_ol_bits(double x) {
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return bits;
}

static double dmath_ol_from_bits(uint64_t bits) {
    double x;
    memcpy(&x, &bits, sizeof(x));
    return x;
}

/* Avoid implementation-defined unsigned-to-signed conversions of bit words. */
static int32_t dmath_ol_i32(uint32_t word) {
    return word <= INT32_MAX ? (int32_t)word : (int32_t)((int64_t)word - 0x100000000LL);
}

static double dmath_ol_result(double x) {
    uint64_t magnitude = dmath_ol_bits(x) & UINT64_C(0x7fffffffffffffff);
    return magnitude > UINT64_C(0x7ff0000000000000)
               ? dmath_ol_from_bits(UINT64_C(0x7ff8000000000000))
               : x;
}

#define DMATH_OL_GET_HIGH_WORD(hi, x) ((hi) = dmath_ol_i32((uint32_t)(dmath_ol_bits(x) >> 32)))
#define DMATH_OL_GET_LOW_WORD(lo, x) ((lo) = dmath_ol_i32((uint32_t)dmath_ol_bits(x)))
#define DMATH_OL_EXTRACT_WORDS(hi, lo, x)                                                          \
    do {                                                                                           \
        uint64_t dmath_ol_words = dmath_ol_bits(x);                                                \
        (hi) = dmath_ol_i32((uint32_t)(dmath_ol_words >> 32));                                     \
        (lo) = dmath_ol_i32((uint32_t)dmath_ol_words);                                             \
    } while(0)
#define DMATH_OL_INSERT_WORDS(x, hi, lo)                                                           \
    ((x) = dmath_ol_from_bits(((uint64_t)(uint32_t)(hi) << 32) | (uint32_t)(lo)))
#define DMATH_OL_SET_HIGH_WORD(x, hi)                                                              \
    ((x) = dmath_ol_from_bits((dmath_ol_bits(x) & UINT64_C(0xffffffff)) |                          \
                              ((uint64_t)(uint32_t)(hi) << 32)))
#define DMATH_OL_SET_LOW_WORD(x, lo)                                                               \
    ((x) = dmath_ol_from_bits((dmath_ol_bits(x) & UINT64_C(0xffffffff00000000)) | (uint32_t)(lo)))
#define DMATH_OL_STRICT_ASSIGN(type, dst, value) ((dst) = (value))

// OpenLibm src/k_log.h
/* @(#)e_log.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * dmath_ol_log1p_kernel(f):
 * Return log(1+f) - f for 1+f in ~[dmath_sqrt(2)/2, dmath_sqrt(2)].
 *
 * The following describes the overall strategy for computing
 * logarithms in base e.  The argument reduction and adding the final
 * term of the polynomial are done by the caller for increased accuracy
 * when different bases are used.
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  dmath_sqrt(2)/2 < 1+f < dmath_sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *	Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	     	 = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 * 	a polynomial of degree 14 to approximate R The maximum error
 *	of this polynomial approximation is bounded by 2**-58.45. In
 *	other words,
 *		        2      4      6      8      10      12      14
 *	    R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *  	(the values of Lg1 to Lg7 are listed in the program)
 *	and
 *	    |      2          14          |     -58.45
 *	    | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *	    |                             |
 *	Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *	In order to guarantee error in log below 1ulp, we compute log
 *	by
 *		log(1+f) = f - s*(f - R)	(if f is not too large)
 *		log(1+f) = f - (hfsq - s*(hfsq+R)).	(better accuracy)
 *
 *	3. Finally,  log(x) = k*ln2 + log(1+f).
 *			    = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *	   Here ln2 is split into two floating point number:
 *			ln2_hi + ln2_lo,
 *	   where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *	log(x) is NaN with signal if x < 0 (including -INF) ;
 *	log(+INF) is +INF; log(0) is -INF with signal;
 *	log(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *	according to an error analysis, the error is always less than
 *	1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static inline double dmath_ol_log1p_kernel(double f) {
    static const double Lg1 = 6.666666666666735130e-01, /* 3FE55555 55555593 */
        Lg2 = 3.999999999940941908e-01,                 /* 3FD99999 9997FA04 */
        Lg3 = 2.857142874366239149e-01,                 /* 3FD24924 94229359 */
        Lg4 = 2.222219843214978396e-01,                 /* 3FCC71C5 1D8E78AF */
        Lg5 = 1.818357216161805012e-01,                 /* 3FC74664 96CB03DE */
        Lg6 = 1.531383769920937332e-01,                 /* 3FC39A09 D078C69F */
        Lg7 = 1.479819860511658591e-01;                 /* 3FC2F112 DF3E5244 */

    /*
     * We always inline dmath_ol_log1p_kernel(), since doing so produces a
     * substantial performance improvement (~40% on amd64).
     */

    double hfsq, s, z, R, w, t1, t2;

    s = f / (2.0 + f);
    z = s * s;
    w = z * z;
    t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
    t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
    R = t2 + t1;
    hfsq = 0.5 * f * f;
    return s * (hfsq + R);
}

// OpenLibm src/s_scalbn.c
/* @(#)s_scalbn.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * dmath_ol_scalbn (double x, int n)
 * dmath_ol_scalbn(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

static double dmath_ol_scalbn(double x, int n) {
    static const double two54 = 1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
        twom54 = 5.55111512312578270212e-17,                /* 0x3C900000, 0x00000000 */
        huge = 1.0e+300, tiny = 1.0e-300;

    int32_t k, hx, lx;
    DMATH_OL_EXTRACT_WORDS(hx, lx, x);
    k = (hx & 0x7ff00000) >> 20;                    /* extract exponent */
    if(k == 0) {                                    /* 0 or subnormal x */
        if((lx | (hx & 0x7fffffff)) == 0) return x; /* +-0 */
        x *= two54;
        DMATH_OL_GET_HIGH_WORD(hx, x);
        k = ((hx & 0x7ff00000) >> 20) - 54;
        if(n < -50000) return tiny * x; /*underflow*/
    }
    if(k == 0x7ff) return x + x; /* NaN or Inf */
    /* Bound n before adding it: the upstream overflow check was too late. */
    if(n > 50000) return huge * dmath_copysign(huge, x);
    if(n < -50000) return tiny * dmath_copysign(tiny, x);
    k = k + n;
    if(k > 0x7fe) return huge * dmath_copysign(huge, x); /* overflow  */
    if(k > 0)                                            /* normal result */
    {
        DMATH_OL_SET_HIGH_WORD(x, (hx & 0x800fffff) | (k << 20));
        return x;
    }
    if(k <= -54) {
        if(n > 50000)                              /* in case integer overflow in n+k */
            return huge * dmath_copysign(huge, x); /*overflow*/
        else
            return tiny * dmath_copysign(tiny, x); /*underflow*/
    }
    k += 54; /* subnormal result */
    DMATH_OL_SET_HIGH_WORD(x, (hx & 0x800fffff) | (k << 20));
    return x * twom54;
}

// OpenLibm src/e_exp.c
/* @(#)e_exp.c 1.6 04/04/22 */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_exp(x)
 * Returns the exponential of x.
 *
 * Method
 *   1. Argument reduction:
 *      Reduce x to an r so that |r| <= 0.5*ln2 ~ 0.34658.
 *	Given x, find r and integer k such that
 *
 *               x = k*ln2 + r,  |r| <= 0.5*ln2.
 *
 *      Here r will be represented as r = hi-lo for better
 *	accuracy.
 *
 *   2. Approximation of exp(r) by a special rational function on
 *	the interval [0,0.34658]:
 *	Write
 *	    R(r**2) = r*(exp(r)+1)/(exp(r)-1) = 2 + r*r/6 - r**4/360 + ...
 *      We use a special Remes algorithm on [0,0.34658] to generate
 * 	a polynomial of degree 5 to approximate R. The maximum error
 *	of this polynomial approximation is bounded by 2**-59. In
 *	other words,
 *	    R(z) ~ 2.0 + P1*z + P2*z**2 + P3*z**3 + P4*z**4 + P5*z**5
 *  	(where z=r*r, and the values of P1 to P5 are listed below)
 *	and
 *	    |                  5          |     -59
 *	    | 2.0+P1*z+...+P5*z   -  R(z) | <= 2
 *	    |                             |
 *	The computation of exp(r) thus becomes
 *                             2*r
 *		exp(r) = 1 + -------
 *		              R - r
 *                                 r*R1(r)
 *		       = 1 + r + ----------- (for better accuracy)
 *		                  2 - R1(r)
 *	where
 *			         2       4             10
 *		R1(r) = r - (P1*r  + P2*r  + ... + P5*r   ).
 *
 *   3. Scale back to obtain exp(x):
 *	From step 1, we have
 *	   exp(x) = 2^k * exp(r)
 *
 * Special cases:
 *	exp(INF) is INF, exp(NaN) is NaN;
 *	exp(-INF) is 0, and
 *	for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *	according to an error analysis, the error is always less than
 *	1 ulp (unit in the last place).
 *
 * Misc. info.
 *	For IEEE double
 *	    if x >  7.09782712893383973096e+02 then exp(x) overflow
 *	    if x < -7.45133219101941108420e+02 then exp(x) underflow
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static double dmath_ol_exp(double x) /* default IEEE double exp */
{
    static const double one = 1.0,
                        halF[2] =
                            {
                                0.5,
                                -0.5,
                            },
                        huge = 1.0e+300,
                        o_threshold = 7.09782712893383973096e+02, /* 0x40862E42, 0xFEFA39EF */
        u_threshold = -7.45133219101941108420e+02,                /* 0xc0874910, 0xD52D3051 */
        ln2HI[2] =
            {
                6.93147180369123816490e-01, /* 0x3fe62e42, 0xfee00000 */
                -6.93147180369123816490e-01,
            }, /* 0xbfe62e42, 0xfee00000 */
        ln2LO[2] =
            {
                1.90821492927058770002e-10, /* 0x3dea39ef, 0x35793c76 */
                -1.90821492927058770002e-10,
            },                               /* 0xbdea39ef, 0x35793c76 */
        invln2 = 1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
        P1 = 1.66666666666666019037e-01,     /* 0x3FC55555, 0x5555553E */
        P2 = -2.77777777770155933842e-03,    /* 0xBF66C16C, 0x16BEBD93 */
        P3 = 6.61375632143793436117e-05,     /* 0x3F11566A, 0xAF25DE2C */
        P4 = -1.65339022054652515390e-06,    /* 0xBEBBBD41, 0xC5D26BF1 */
        P5 = 4.13813679705723846039e-08;     /* 0x3E663769, 0x72BEA4D0 */

    static volatile double twom1000 = 9.33263618503218878990e-302; /* 2**-1000=0x01700000,0*/

    double y, hi = 0.0, lo = 0.0, c, t, twopk;
    int32_t k = 0, xsb;
    uint32_t hx;

    DMATH_OL_GET_HIGH_WORD(hx, x);
    xsb = (hx >> 31) & 1; /* sign bit of x */
    hx &= 0x7fffffff;     /* high word of |x| */

    /* filter out non-finite argument */
    if(hx >= 0x40862E42) { /* if |x|>=709.78... */
        if(hx >= 0x7ff00000) {
            uint32_t lx;
            DMATH_OL_GET_LOW_WORD(lx, x);
            if(((hx & 0xfffff) | lx) != 0)
                return x + x; /* NaN */
            else
                return (xsb == 0) ? x : 0.0; /* exp(+-inf)={inf,0} */
        }
        if(x > o_threshold) return huge * huge;         /* overflow */
        if(x < u_threshold) return twom1000 * twom1000; /* underflow */
    }

    /* this implementation gives 2.7182818284590455 for exp(1.0),
       which is well within the allowable error. however,
       2.718281828459045 is closer to the true value so we prefer that
       answer, given that 1.0 is such an important argument value. */
    if(x == 1.0) return 2.718281828459045235360;

    /* argument reduction */
    if(hx > 0x3fd62e42) {     /* if  |x| > 0.5 ln2 */
        if(hx < 0x3FF0A2B2) { /* and |x| < 1.5 ln2 */
            hi = x - ln2HI[xsb];
            lo = ln2LO[xsb];
            k = 1 - xsb - xsb;
        } else {
            k = (int)(invln2 * x + halF[xsb]);
            t = k;
            hi = x - t * ln2HI[0]; /* t*ln2HI is exact here */
            lo = t * ln2LO[0];
        }
        DMATH_OL_STRICT_ASSIGN(double, x, hi - lo);
    } else if(hx < 0x3e300000) {           /* when |x|<2**-28 */
        if(huge + x > one) return one + x; /* trigger inexact */
    } else
        k = 0;

    /* x is now in primary range */
    t = x * x;
    if(k >= -1021)
        DMATH_OL_INSERT_WORDS(twopk, 0x3ff00000 + (k * 0x00100000), 0);
    else
        DMATH_OL_INSERT_WORDS(twopk, 0x3ff00000 + ((k + 1000) * 0x00100000), 0);
    c = x - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
    if(k == 0)
        return one - ((x * c) / (c - 2.0) - x);
    else
        y = one - ((lo - (x * c) / (2.0 - c)) - hi);
    if(k >= -1021) {
        if(k == 1024) return y * 2.0 * 0x1p1023;
        return y * twopk;
    } else {
        return y * twopk * twom1000;
    }
}

// OpenLibm src/s_exp2.c
/*-
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define DMATH_OL_TBLBITS 8
#define DMATH_OL_TBLSIZE (1 << DMATH_OL_TBLBITS)

static double dmath_ol_exp2(double x) {
    static const double huge = 0x1p1000, redux = 0x1.8p52 / DMATH_OL_TBLSIZE,
                        P1 = 0x1.62e42fefa39efp-1, P2 = 0x1.ebfbdff82c575p-3,
                        P3 = 0x1.c6b08d704a0a6p-5, P4 = 0x1.3b2ab88f70400p-7,
                        P5 = 0x1.5d88003875c74p-10;

    static volatile double twom1000 = 0x1p-1000;

    static const double tbl[DMATH_OL_TBLSIZE * 2] = {
        /*	dmath_ol_exp2(z + eps)		eps	*/
        0x1.6a09e667f3d5dp-1, 0x1.9880p-44,  0x1.6b052fa751744p-1, 0x1.8000p-50,
        0x1.6c012750bd9fep-1, -0x1.8780p-45, 0x1.6cfdcddd476bfp-1, 0x1.ec00p-46,
        0x1.6dfb23c651a29p-1, -0x1.8000p-50, 0x1.6ef9298593ae3p-1, -0x1.c000p-52,
        0x1.6ff7df9519386p-1, -0x1.fd80p-45, 0x1.70f7466f42da3p-1, -0x1.c880p-45,
        0x1.71f75e8ec5fc3p-1, 0x1.3c00p-46,  0x1.72f8286eacf05p-1, -0x1.8300p-44,
        0x1.73f9a48a58152p-1, -0x1.0c00p-47, 0x1.74fbd35d7ccfcp-1, 0x1.f880p-45,
        0x1.75feb564267f1p-1, 0x1.3e00p-47,  0x1.77024b1ab6d48p-1, -0x1.7d00p-45,
        0x1.780694fde5d38p-1, -0x1.d000p-50, 0x1.790b938ac1d00p-1, 0x1.3000p-49,
        0x1.7a11473eb0178p-1, -0x1.d000p-49, 0x1.7b17b0976d060p-1, 0x1.0400p-45,
        0x1.7c1ed0130c133p-1, 0x1.0000p-53,  0x1.7d26a62ff8636p-1, -0x1.6900p-45,
        0x1.7e2f336cf4e3bp-1, -0x1.2e00p-47, 0x1.7f3878491c3e8p-1, -0x1.4580p-45,
        0x1.80427543e1b4ep-1, 0x1.3000p-44,  0x1.814d2add1071ap-1, 0x1.f000p-47,
        0x1.82589994ccd7ep-1, -0x1.1c00p-45, 0x1.8364c1eb942d0p-1, 0x1.9d00p-45,
        0x1.8471a4623cab5p-1, 0x1.7100p-43,  0x1.857f4179f5bbcp-1, 0x1.2600p-45,
        0x1.868d99b4491afp-1, -0x1.2c40p-44, 0x1.879cad931a395p-1, -0x1.3000p-45,
        0x1.88ac7d98a65b8p-1, -0x1.a800p-45, 0x1.89bd0a4785800p-1, -0x1.d000p-49,
        0x1.8ace5422aa223p-1, 0x1.3280p-44,  0x1.8be05bad619fap-1, 0x1.2b40p-43,
        0x1.8cf3216b54383p-1, -0x1.ed00p-45, 0x1.8e06a5e08664cp-1, -0x1.0500p-45,
        0x1.8f1ae99157807p-1, 0x1.8280p-45,  0x1.902fed0282c0ep-1, -0x1.cb00p-46,
        0x1.9145b0b91ff96p-1, -0x1.5e00p-47, 0x1.925c353aa2ff9p-1, 0x1.5400p-48,
        0x1.93737b0cdc64ap-1, 0x1.7200p-46,  0x1.948b82b5f98aep-1, -0x1.9000p-47,
        0x1.95a44cbc852cbp-1, 0x1.5680p-45,  0x1.96bdd9a766f21p-1, -0x1.6d00p-44,
        0x1.97d829fde4e2ap-1, -0x1.1000p-47, 0x1.98f33e47a23a3p-1, 0x1.d000p-45,
        0x1.9a0f170ca0604p-1, -0x1.8a40p-44, 0x1.9b2bb4d53ff89p-1, 0x1.55c0p-44,
        0x1.9c49182a3f15bp-1, 0x1.6b80p-45,  0x1.9d674194bb8c5p-1, -0x1.c000p-49,
        0x1.9e86319e3238ep-1, 0x1.7d00p-46,  0x1.9fa5e8d07f302p-1, 0x1.6400p-46,
        0x1.a0c667b5de54dp-1, -0x1.5000p-48, 0x1.a1e7aed8eb8f6p-1, 0x1.9e00p-47,
        0x1.a309bec4a2e27p-1, 0x1.ad80p-45,  0x1.a42c980460a5dp-1, -0x1.af00p-46,
        0x1.a5503b23e259bp-1, 0x1.b600p-47,  0x1.a674a8af46213p-1, 0x1.8880p-44,
        0x1.a799e1330b3a7p-1, 0x1.1200p-46,  0x1.a8bfe53c12e8dp-1, 0x1.6c00p-47,
        0x1.a9e6b5579fcd2p-1, -0x1.9b80p-45, 0x1.ab0e521356fb8p-1, 0x1.b700p-45,
        0x1.ac36bbfd3f381p-1, 0x1.9000p-50,  0x1.ad5ff3a3c2780p-1, 0x1.4000p-49,
        0x1.ae89f995ad2a3p-1, -0x1.c900p-45, 0x1.afb4ce622f367p-1, 0x1.6500p-46,
        0x1.b0e07298db790p-1, 0x1.fd40p-45,  0x1.b20ce6c9a89a9p-1, 0x1.2700p-46,
        0x1.b33a2b84f1a4bp-1, 0x1.d470p-43,  0x1.b468415b747e7p-1, -0x1.8380p-44,
        0x1.b59728de5593ap-1, 0x1.8000p-54,  0x1.b6c6e29f1c56ap-1, 0x1.ad00p-47,
        0x1.b7f76f2fb5e50p-1, 0x1.e800p-50,  0x1.b928cf22749b2p-1, -0x1.4c00p-47,
        0x1.ba5b030a10603p-1, -0x1.d700p-47, 0x1.bb8e0b79a6f66p-1, 0x1.d900p-47,
        0x1.bcc1e904bc1ffp-1, 0x1.2a00p-47,  0x1.bdf69c3f3a16fp-1, -0x1.f780p-46,
        0x1.bf2c25bd71db8p-1, -0x1.0a00p-46, 0x1.c06286141b2e9p-1, -0x1.1400p-46,
        0x1.c199bdd8552e0p-1, 0x1.be00p-47,  0x1.c2d1cd9fa64eep-1, -0x1.9400p-47,
        0x1.c40ab5fffd02fp-1, -0x1.ed00p-47, 0x1.c544778fafd15p-1, 0x1.9660p-44,
        0x1.c67f12e57d0cbp-1, -0x1.a100p-46, 0x1.c7ba88988c1b6p-1, -0x1.8458p-42,
        0x1.c8f6d9406e733p-1, -0x1.a480p-46, 0x1.ca3405751c4dfp-1, 0x1.b000p-51,
        0x1.cb720dcef9094p-1, 0x1.1400p-47,  0x1.ccb0f2e6d1689p-1, 0x1.0200p-48,
        0x1.cdf0b555dc412p-1, 0x1.3600p-48,  0x1.cf3155b5bab3bp-1, -0x1.6900p-47,
        0x1.d072d4a0789bcp-1, 0x1.9a00p-47,  0x1.d1b532b08c8fap-1, -0x1.5e00p-46,
        0x1.d2f87080d8a85p-1, 0x1.d280p-46,  0x1.d43c8eacaa203p-1, 0x1.1a00p-47,
        0x1.d5818dcfba491p-1, 0x1.f000p-50,  0x1.d6c76e862e6a1p-1, -0x1.3a00p-47,
        0x1.d80e316c9834ep-1, -0x1.cd80p-47, 0x1.d955d71ff6090p-1, 0x1.4c00p-48,
        0x1.da9e603db32aep-1, 0x1.f900p-48,  0x1.dbe7cd63a8325p-1, 0x1.9800p-49,
        0x1.dd321f301b445p-1, -0x1.5200p-48, 0x1.de7d5641c05bfp-1, -0x1.d700p-46,
        0x1.dfc97337b9aecp-1, -0x1.6140p-46, 0x1.e11676b197d5ep-1, 0x1.b480p-47,
        0x1.e264614f5a3e7p-1, 0x1.0ce0p-43,  0x1.e3b333b16ee5cp-1, 0x1.c680p-47,
        0x1.e502ee78b3fb4p-1, -0x1.9300p-47, 0x1.e653924676d68p-1, -0x1.5000p-49,
        0x1.e7a51fbc74c44p-1, -0x1.7f80p-47, 0x1.e8f7977cdb726p-1, -0x1.3700p-48,
        0x1.ea4afa2a490e8p-1, 0x1.5d00p-49,  0x1.eb9f4867ccae4p-1, 0x1.61a0p-46,
        0x1.ecf482d8e680dp-1, 0x1.5500p-48,  0x1.ee4aaa2188514p-1, 0x1.6400p-51,
        0x1.efa1bee615a13p-1, -0x1.e800p-49, 0x1.f0f9c1cb64106p-1, -0x1.a880p-48,
        0x1.f252b376bb963p-1, -0x1.c900p-45, 0x1.f3ac948dd7275p-1, 0x1.a000p-53,
        0x1.f50765b6e4524p-1, -0x1.4f00p-48, 0x1.f6632798844fdp-1, 0x1.a800p-51,
        0x1.f7bfdad9cbe38p-1, 0x1.abc0p-48,  0x1.f91d802243c82p-1, -0x1.4600p-50,
        0x1.fa7c1819e908ep-1, -0x1.b0c0p-47, 0x1.fbdba3692d511p-1, -0x1.0e00p-51,
        0x1.fd3c22b8f7194p-1, -0x1.0de8p-46, 0x1.fe9d96b2a23eep-1, 0x1.e430p-49,
        0x1.0000000000000p+0, 0x0.0000p+0,   0x1.00b1afa5abcbep+0, -0x1.3400p-52,
        0x1.0163da9fb3303p+0, -0x1.2170p-46, 0x1.02168143b0282p+0, 0x1.a400p-52,
        0x1.02c9a3e77806cp+0, 0x1.f980p-49,  0x1.037d42e11bbcap+0, -0x1.7400p-51,
        0x1.04315e86e7f89p+0, 0x1.8300p-50,  0x1.04e5f72f65467p+0, -0x1.a3f0p-46,
        0x1.059b0d315855ap+0, -0x1.2840p-47, 0x1.0650a0e3c1f95p+0, 0x1.1600p-48,
        0x1.0706b29ddf71ap+0, 0x1.5240p-46,  0x1.07bd42b72a82dp+0, -0x1.9a00p-49,
        0x1.0874518759bd0p+0, 0x1.6400p-49,  0x1.092bdf66607c8p+0, -0x1.0780p-47,
        0x1.09e3ecac6f383p+0, -0x1.8000p-54, 0x1.0a9c79b1f3930p+0, 0x1.fa00p-48,
        0x1.0b5586cf988fcp+0, -0x1.ac80p-48, 0x1.0c0f145e46c8ap+0, 0x1.9c00p-50,
        0x1.0cc922b724816p+0, 0x1.5200p-47,  0x1.0d83b23395dd8p+0, -0x1.ad00p-48,
        0x1.0e3ec32d3d1f3p+0, 0x1.bac0p-46,  0x1.0efa55fdfa9a6p+0, -0x1.4e80p-47,
        0x1.0fb66affed2f0p+0, -0x1.d300p-47, 0x1.1073028d7234bp+0, 0x1.1500p-48,
        0x1.11301d0125b5bp+0, 0x1.c000p-49,  0x1.11edbab5e2af9p+0, 0x1.6bc0p-46,
        0x1.12abdc06c31d5p+0, 0x1.8400p-49,  0x1.136a814f2047dp+0, -0x1.ed00p-47,
        0x1.1429aaea92de9p+0, 0x1.8e00p-49,  0x1.14e95934f3138p+0, 0x1.b400p-49,
        0x1.15a98c8a58e71p+0, 0x1.5300p-47,  0x1.166a45471c3dfp+0, 0x1.3380p-47,
        0x1.172b83c7d5211p+0, 0x1.8d40p-45,  0x1.17ed48695bb9fp+0, -0x1.5d00p-47,
        0x1.18af9388c8d93p+0, -0x1.c880p-46, 0x1.1972658375d66p+0, 0x1.1f00p-46,
        0x1.1a35beb6fcba7p+0, 0x1.0480p-46,  0x1.1af99f81387e3p+0, -0x1.7390p-43,
        0x1.1bbe084045d54p+0, 0x1.4e40p-45,  0x1.1c82f95281c43p+0, -0x1.a200p-47,
        0x1.1d4873168b9b2p+0, 0x1.3800p-49,  0x1.1e0e75eb44031p+0, 0x1.ac00p-49,
        0x1.1ed5022fcd938p+0, 0x1.1900p-47,  0x1.1f9c18438cdf7p+0, -0x1.b780p-46,
        0x1.2063b88628d8fp+0, 0x1.d940p-45,  0x1.212be3578a81ep+0, 0x1.8000p-50,
        0x1.21f49917ddd41p+0, 0x1.b340p-45,  0x1.22bdda2791323p+0, 0x1.9f80p-46,
        0x1.2387a6e7561e7p+0, -0x1.9c80p-46, 0x1.2451ffb821427p+0, 0x1.2300p-47,
        0x1.251ce4fb2a602p+0, -0x1.3480p-46, 0x1.25e85711eceb0p+0, 0x1.2700p-46,
        0x1.26b4565e27d16p+0, 0x1.1d00p-46,  0x1.2780e341de00fp+0, 0x1.1ee0p-44,
        0x1.284dfe1f5633ep+0, -0x1.4c00p-46, 0x1.291ba7591bb30p+0, -0x1.3d80p-46,
        0x1.29e9df51fdf09p+0, 0x1.8b00p-47,  0x1.2ab8a66d10e9bp+0, -0x1.27c0p-45,
        0x1.2b87fd0dada3ap+0, 0x1.a340p-45,  0x1.2c57e39771af9p+0, -0x1.0800p-46,
        0x1.2d285a6e402d9p+0, -0x1.ed00p-47, 0x1.2df961f641579p+0, -0x1.4200p-48,
        0x1.2ecafa93e2ecfp+0, -0x1.4980p-45, 0x1.2f9d24abd8822p+0, -0x1.6300p-46,
        0x1.306fe0a31b625p+0, -0x1.2360p-44, 0x1.31432edeea50bp+0, -0x1.0df8p-40,
        0x1.32170fc4cd7b8p+0, -0x1.2480p-45, 0x1.32eb83ba8e9a2p+0, -0x1.5980p-45,
        0x1.33c08b2641766p+0, 0x1.ed00p-46,  0x1.3496266e3fa27p+0, -0x1.c000p-50,
        0x1.356c55f929f0fp+0, -0x1.0d80p-44, 0x1.36431a2de88b9p+0, 0x1.2c80p-45,
        0x1.371a7373aaa39p+0, 0x1.0600p-45,  0x1.37f26231e74fep+0, -0x1.6600p-46,
        0x1.38cae6d05d838p+0, -0x1.ae00p-47, 0x1.39a401b713ec3p+0, -0x1.4720p-43,
        0x1.3a7db34e5a020p+0, 0x1.8200p-47,  0x1.3b57fbfec6e95p+0, 0x1.e800p-44,
        0x1.3c32dc313a8f2p+0, 0x1.f800p-49,  0x1.3d0e544ede122p+0, -0x1.7a00p-46,
        0x1.3dea64c1234bbp+0, 0x1.6300p-45,  0x1.3ec70df1c4eccp+0, -0x1.8a60p-43,
        0x1.3fa4504ac7e8cp+0, -0x1.cdc0p-44, 0x1.40822c367a0bbp+0, 0x1.5b80p-45,
        0x1.4160a21f72e95p+0, 0x1.ec00p-46,  0x1.423fb27094646p+0, -0x1.3600p-46,
        0x1.431f5d950a920p+0, 0x1.3980p-45,  0x1.43ffa3f84b9ebp+0, 0x1.a000p-48,
        0x1.44e0860618919p+0, -0x1.6c00p-48, 0x1.45c2042a7d201p+0, -0x1.bc00p-47,
        0x1.46a41ed1d0016p+0, -0x1.2800p-46, 0x1.4786d668b3326p+0, 0x1.0e00p-44,
        0x1.486a2b5c13c00p+0, -0x1.d400p-45, 0x1.494e1e192af04p+0, 0x1.c200p-47,
        0x1.4a32af0d7d372p+0, -0x1.e500p-46, 0x1.4b17dea6db801p+0, 0x1.7800p-47,
        0x1.4bfdad53629e1p+0, -0x1.3800p-46, 0x1.4ce41b817c132p+0, 0x1.0800p-47,
        0x1.4dcb299fddddbp+0, 0x1.c700p-45,  0x1.4eb2d81d8ab96p+0, -0x1.ce00p-46,
        0x1.4f9b2769d2d02p+0, 0x1.9200p-46,  0x1.508417f4531c1p+0, -0x1.8c00p-47,
        0x1.516daa2cf662ap+0, -0x1.a000p-48, 0x1.5257de83f51eap+0, 0x1.a080p-43,
        0x1.5342b569d4edap+0, -0x1.6d80p-45, 0x1.542e2f4f6ac1ap+0, -0x1.2440p-44,
        0x1.551a4ca5d94dbp+0, 0x1.83c0p-43,  0x1.56070dde9116bp+0, 0x1.4b00p-45,
        0x1.56f4736b529dep+0, 0x1.15a0p-43,  0x1.57e27dbe2c40ep+0, -0x1.9e00p-45,
        0x1.58d12d497c76fp+0, -0x1.3080p-45, 0x1.59c0827ff0b4cp+0, 0x1.dec0p-43,
        0x1.5ab07dd485427p+0, -0x1.4000p-51, 0x1.5ba11fba87af4p+0, 0x1.0080p-44,
        0x1.5c9268a59460bp+0, -0x1.6c80p-45, 0x1.5d84590998e3fp+0, 0x1.69a0p-43,
        0x1.5e76f15ad20e1p+0, -0x1.b400p-46, 0x1.5f6a320dcebcap+0, 0x1.7700p-46,
        0x1.605e1b976dcb8p+0, 0x1.6f80p-45,  0x1.6152ae6cdf715p+0, 0x1.1000p-47,
        0x1.6247eb03a5531p+0, -0x1.5d00p-46, 0x1.633dd1d1929b5p+0, -0x1.2d00p-46,
        0x1.6434634ccc313p+0, -0x1.a800p-49, 0x1.652b9febc8efap+0, -0x1.8600p-45,
        0x1.6623882553397p+0, 0x1.1fe0p-40,  0x1.671c1c708328ep+0, -0x1.7200p-44,
        0x1.68155d44ca97ep+0, 0x1.6800p-49,  0x1.690f4b19e9471p+0, -0x1.9780p-45,
    };

    /*
     * dmath_ol_exp2(x): compute the base 2 exponential of x
     *
     * Accuracy: Peak error < 0.503 ulp for normalized results.
     *
     * Method: (accurate tables)
     *
     *   Reduce x:
     *     x = 2**k + y, for integer k and |y| <= 1/2.
     *     Thus we have dmath_ol_exp2(x) = 2**k * dmath_ol_exp2(y).
     *
     *   Reduce y:
     *     y = i/DMATH_OL_TBLSIZE + z - eps[i] for integer i near y * DMATH_OL_TBLSIZE.
     *     Thus we have dmath_ol_exp2(y) = dmath_ol_exp2(i/DMATH_OL_TBLSIZE) * dmath_ol_exp2(z -
     *eps[i]), with |z - eps[i]| <= 2**-9 + 2**-39 for the table used.
     *
     *   We compute dmath_ol_exp2(i/DMATH_OL_TBLSIZE) via table lookup and dmath_ol_exp2(z - eps[i])
     *via a degree-5 minimax polynomial with maximum error under 1.3 * 2**-61. The values in exp2t[]
     *and eps[] are chosen such that exp2t[i] = dmath_ol_exp2(i/DMATH_OL_TBLSIZE + eps[i]), and
     *eps[i] is a small offset such that exp2t[i] is accurate to 2**-64.
     *
     *   Note that the range of i is +-DMATH_OL_TBLSIZE/2, so we actually index the tables
     *   by i0 = i + DMATH_OL_TBLSIZE/2.  For cache efficiency, exp2t[] and eps[] are
     *   virtual tables, interleaved in the real table tbl[].
     *
     *   This method is due to Gal, with many details due to Gal and Bachelis:
     *
     *	Gal, S. and Bachelis, B.  An Accurate Elementary Mathematical Library
     *	for the IEEE Floating Point Standard.  TOMS 17(1), 26-46 (1991).
     */

    double r, t, twopk, twopkp1000, z;
    uint32_t hx, ix, lx, i0;
    int k;

    /* Filter out exceptional cases. */
    DMATH_OL_GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;  /* high word of |x| */
    if(ix >= 0x40900000) { /* |x| >= 1024 */
        if(ix >= 0x7ff00000) {
            DMATH_OL_GET_LOW_WORD(lx, x);
            if(((ix & 0xfffff) | lx) != 0 || (hx & 0x80000000) == 0)
                return (x + x); /* x is NaN or +Inf */
            else
                return (0.0); /* x is -Inf */
        }
        if(x >= 0x1.0p10) return (huge * huge);            /* overflow */
        if(x <= -0x1.0ccp10) return (twom1000 * twom1000); /* underflow */
    } else if(ix < 0x3c900000) {                           /* |x| < 0x1p-54 */
        return (1.0 + x);
    }

    /* Reduce x, computing z, i0, and k. */
    DMATH_OL_STRICT_ASSIGN(double, t, x + redux);
    DMATH_OL_GET_LOW_WORD(i0, t);
    i0 += DMATH_OL_TBLSIZE / 2;
    k = dmath_ol_i32((i0 >> DMATH_OL_TBLBITS) << 20);
    i0 = (i0 & (DMATH_OL_TBLSIZE - 1)) << 1;
    t -= redux;
    z = x - t;

    /* Compute r = dmath_ol_exp2(y) = exp2t[i0] * p(z - eps[i]). */
    t = tbl[i0];      /* exp2t[i0] */
    z -= tbl[i0 + 1]; /* eps[i0]   */
    if(k >= -(1021 << 20))
        DMATH_OL_INSERT_WORDS(twopk, 0x3ff00000 + k, 0);
    else
        DMATH_OL_INSERT_WORDS(twopkp1000, 0x3ff00000 + k + (1000 << 20), 0);
    r = t + t * z * (P1 + z * (P2 + z * (P3 + z * (P4 + z * P5))));

    /* Scale by 2**(k>>20). */
    if(k >= -(1021 << 20)) {
        if(k == 1024 << 20) return (r * 2.0 * 0x1p1023);
        return (r * twopk);
    } else {
        return (r * twopkp1000 * twom1000);
    }
}

#undef DMATH_OL_TBLBITS
#undef DMATH_OL_TBLSIZE

// OpenLibm src/e_log.c
/* @(#)e_log.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_log(x)
 * Return the logrithm of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  dmath_sqrt(2)/2 < 1+f < dmath_sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *	Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *	     	 = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 * 	a polynomial of degree 14 to approximate R The maximum error
 *	of this polynomial approximation is bounded by 2**-58.45. In
 *	other words,
 *		        2      4      6      8      10      12      14
 *	    R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *  	(the values of Lg1 to Lg7 are listed in the program)
 *	and
 *	    |      2          14          |     -58.45
 *	    | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *	    |                             |
 *	Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *	In order to guarantee error in log below 1ulp, we compute log
 *	by
 *		log(1+f) = f - s*(f - R)	(if f is not too large)
 *		log(1+f) = f - (hfsq - s*(hfsq+R)).	(better accuracy)
 *
 *	3. Finally,  log(x) = k*ln2 + log(1+f).
 *			    = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *	   Here ln2 is split into two floating point number:
 *			ln2_hi + ln2_lo,
 *	   where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *	log(x) is NaN with signal if x < 0 (including -INF) ;
 *	log(+INF) is +INF; log(0) is -INF with signal;
 *	log(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *	according to an error analysis, the error is always less than
 *	1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static double dmath_ol_log(double x) {
    static const double ln2_hi = 6.93147180369123816490e-01, /* 3fe62e42 fee00000 */
        ln2_lo = 1.90821492927058770002e-10,                 /* 3dea39ef 35793c76 */
        two54 = 1.80143985094819840000e+16,                  /* 43500000 00000000 */
        Lg1 = 6.666666666666735130e-01,                      /* 3FE55555 55555593 */
        Lg2 = 3.999999999940941908e-01,                      /* 3FD99999 9997FA04 */
        Lg3 = 2.857142874366239149e-01,                      /* 3FD24924 94229359 */
        Lg4 = 2.222219843214978396e-01,                      /* 3FCC71C5 1D8E78AF */
        Lg5 = 1.818357216161805012e-01,                      /* 3FC74664 96CB03DE */
        Lg6 = 1.531383769920937332e-01,                      /* 3FC39A09 D078C69F */
        Lg7 = 1.479819860511658591e-01;                      /* 3FC2F112 DF3E5244 */

    static const double zero = 0.0;

    double hfsq, f, s, z, R, w, t1, t2, dk;
    int32_t k, hx, i, j;
    uint32_t lx;

    DMATH_OL_EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if(hx < 0x00100000) {                                       /* x < 2**-1022  */
        if(((hx & 0x7fffffff) | lx) == 0) return -two54 / zero; /* log(+-0)=-inf */
        if(hx < 0) return (x - x) / zero;                       /* log(-#) = NaN */
        k -= 54;
        x *= two54; /* subnormal number, scale up x */
        DMATH_OL_GET_HIGH_WORD(hx, x);
    }
    if(hx >= 0x7ff00000) return x + x;
    k += (hx >> 20) - 1023;
    hx &= 0x000fffff;
    i = (hx + 0x95f64) & 0x100000;
    DMATH_OL_SET_HIGH_WORD(x, hx | (i ^ 0x3ff00000)); /* normalize x or x/2 */
    k += (i >> 20);
    f = x - 1.0;
    if((0x000fffff & (2 + hx)) < 3) { /* -2**-20 <= f < 2**-20 */
        if(f == zero) {
            if(k == 0) {
                return zero;
            } else {
                dk = (double)k;
                return dk * ln2_hi + dk * ln2_lo;
            }
        }
        R = f * f * (0.5 - 0.33333333333333333 * f);
        if(k == 0)
            return f - R;
        else {
            dk = (double)k;
            return dk * ln2_hi - ((R - dk * ln2_lo) - f);
        }
    }
    s = f / (2.0 + f);
    dk = (double)k;
    z = s * s;
    i = hx - 0x6147a;
    w = z * z;
    j = 0x6b851 - hx;
    t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
    t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
    i |= j;
    R = t2 + t1;
    if(i > 0) {
        hfsq = 0.5 * f * f;
        if(k == 0)
            return f - (hfsq - s * (hfsq + R));
        else
            return dk * ln2_hi - ((hfsq - (s * (hfsq + R) + dk * ln2_lo)) - f);
    } else {
        if(k == 0)
            return f - s * (f - R);
        else
            return dk * ln2_hi - ((s * (f - R) - dk * ln2_lo) - f);
    }
}

// OpenLibm src/e_log2.c
/* @(#)e_log10.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * Return the base 2 logarithm of x.  See e_log.c and k_log.h for most
 * comments.
 *
 * This reduces x to {k, 1+f} exactly as in e_log.c, then calls the kernel,
 * then does the combining and scaling steps
 *    log2(x) = (f - 0.5*f*f + dmath_ol_log1p_kernel(f)) / ln2 + k
 * in not-quite-routine extra precision.
 */

static double dmath_ol_log2(double x) {
    static const double two54 = 1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
        ivln2hi = 1.44269504072144627571e+00,               /* 0x3ff71547, 0x65200000 */
        ivln2lo = 1.67517131648865118353e-10;               /* 0x3de705fc, 0x2eefa200 */

    static const double zero = 0.0;

    double f, hfsq, hi, lo, r, val_hi, val_lo, w, y;
    int32_t i, k, hx;
    uint32_t lx;

    DMATH_OL_EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if(hx < 0x00100000) {                                       /* x < 2**-1022  */
        if(((hx & 0x7fffffff) | lx) == 0) return -two54 / zero; /* log(+-0)=-inf */
        if(hx < 0) return (x - x) / zero;                       /* log(-#) = NaN */
        k -= 54;
        x *= two54; /* subnormal number, scale up x */
        DMATH_OL_GET_HIGH_WORD(hx, x);
    }
    if(hx >= 0x7ff00000) return x + x;
    if(hx == 0x3ff00000 && lx == 0) return zero; /* log(1) = +0 */
    k += (hx >> 20) - 1023;
    hx &= 0x000fffff;
    i = (hx + 0x95f64) & 0x100000;
    DMATH_OL_SET_HIGH_WORD(x, hx | (i ^ 0x3ff00000)); /* normalize x or x/2 */
    k += (i >> 20);
    y = (double)k;
    f = x - 1.0;
    hfsq = 0.5 * f * f;
    r = dmath_ol_log1p_kernel(f);

    /*
     * f-hfsq must (for args near 1) be evaluated in extra precision
     * to avoid a large cancellation when x is near dmath_sqrt(2) or 1/dmath_sqrt(2).
     * This is fairly efficient since f-hfsq only depends on f, so can
     * be evaluated in parallel with R.  Not combining hfsq with R also
     * keeps R small (though not as small as a true `lo' term would be),
     * so that extra precision is not needed for terms involving R.
     *
     * Compiler bugs involving extra precision used to break Dekker's
     * theorem for spitting f-hfsq as hi+lo, unless double_t was used
     * or the multi-precision calculations were avoided when double_t
     * has extra precision.  These problems are now automatically
     * avoided as a side effect of the optimization of combining the
     * Dekker splitting step with the clear-low-bits step.
     *
     * y must (for args near dmath_sqrt(2) and 1/dmath_sqrt(2)) be added in extra
     * precision to avoid a very large cancellation when x is very near
     * these values.  Unlike the above cancellations, this problem is
     * specific to base 2.  It is strange that adding +-1 is so much
     * harder than adding +-ln2 or +-log10_2.
     *
     * This uses Dekker's theorem to normalize y+val_hi, so the
     * compiler bugs are back in some configurations, sigh.  And I
     * don't want to used double_t to avoid them, since that gives a
     * pessimization and the support for avoiding the pessimization
     * is not yet available.
     *
     * The multi-precision calculations for the multiplications are
     * routine.
     */
    hi = f - hfsq;
    DMATH_OL_SET_LOW_WORD(hi, 0);
    lo = (f - hi) - hfsq + r;
    val_hi = hi * ivln2hi;
    val_lo = (lo + hi) * ivln2lo + lo * ivln2hi;

    /* spadd(val_hi, val_lo, y), except for not using double_t: */
    w = y + val_hi;
    val_lo += (y - w) + val_hi;
    val_hi = w;

    return val_lo + val_hi;
}

// OpenLibm src/e_log10.c
/* @(#)e_log10.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * Return the base 10 logarithm of x.  See e_log.c and k_log.h for most
 * comments.
 *
 *    log10(x) = (f - 0.5*f*f + dmath_ol_log1p_kernel(f)) / ln10 + k * log10(2)
 * in not-quite-routine extra precision.
 */

static double dmath_ol_log10(double x) {
    static const double two54 = 1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
        ivln10hi = 4.34294481878168880939e-01,              /* 0x3fdbcb7b, 0x15200000 */
        ivln10lo = 2.50829467116452752298e-11,              /* 0x3dbb9438, 0xca9aadd5 */
        log10_2hi = 3.01029995663611771306e-01,             /* 0x3FD34413, 0x509F6000 */
        log10_2lo = 3.69423907715893078616e-13;             /* 0x3D59FEF3, 0x11F12B36 */

    static const double zero = 0.0;

    double f, hfsq, hi, lo, r, val_hi, val_lo, w, y, y2;
    int32_t i, k, hx;
    uint32_t lx;

    DMATH_OL_EXTRACT_WORDS(hx, lx, x);

    k = 0;
    if(hx < 0x00100000) {                                       /* x < 2**-1022  */
        if(((hx & 0x7fffffff) | lx) == 0) return -two54 / zero; /* log(+-0)=-inf */
        if(hx < 0) return (x - x) / zero;                       /* log(-#) = NaN */
        k -= 54;
        x *= two54; /* subnormal number, scale up x */
        DMATH_OL_GET_HIGH_WORD(hx, x);
    }
    if(hx >= 0x7ff00000) return x + x;
    if(hx == 0x3ff00000 && lx == 0) return zero; /* log(1) = +0 */
    k += (hx >> 20) - 1023;
    hx &= 0x000fffff;
    i = (hx + 0x95f64) & 0x100000;
    DMATH_OL_SET_HIGH_WORD(x, hx | (i ^ 0x3ff00000)); /* normalize x or x/2 */
    k += (i >> 20);
    y = (double)k;
    f = x - 1.0;
    hfsq = 0.5 * f * f;
    r = dmath_ol_log1p_kernel(f);

    /* See e_log2.c for most details. */
    hi = f - hfsq;
    DMATH_OL_SET_LOW_WORD(hi, 0);
    lo = (f - hi) - hfsq + r;
    val_hi = hi * ivln10hi;
    y2 = y * log10_2hi;
    val_lo = y * log10_2lo + (lo + hi) * ivln10lo + lo * ivln10hi;

    /*
     * Extra precision in for adding y*log10_2hi is not strictly needed
     * since there is no very large cancellation near x = dmath_sqrt(2) or
     * x = 1/dmath_sqrt(2), but we do it anyway since it costs little on CPUs
     * with some parallelism and it reduces the error for many args.
     */
    w = y2 + val_hi;
    val_lo += (y2 - w) + val_hi;
    val_hi = w;

    return val_lo + val_hi;
}

// OpenLibm src/e_pow.c
/* @(#)e_pow.c 1.5 04/04/22 SMI */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_pow(x,y) return x**y
 *
 *		      n
 * Method:  Let x =  2   * (1+f)
 *	1. Compute and return log2(x) in two pieces:
 *		log2(x) = w1 + w2,
 *	   where w1 has 53-24 = 29 bit trailing zeros.
 *	2. Perform y*log2(x) = n+y' by simulating muti-precision
 *	   arithmetic, where |y'|<=0.5.
 *	3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *	1.  (anything) ** 0  is 1
 *	2.  (anything) ** 1  is itself
 *	3.  (anything) ** NAN is NAN
 *	4.  NAN ** (anything except 0) is NAN
 *	5.  +-(|x| > 1) **  +INF is +INF
 *	6.  +-(|x| > 1) **  -INF is +0
 *	7.  +-(|x| < 1) **  +INF is +0
 *	8.  +-(|x| < 1) **  -INF is +INF
 *	9.  +-1         ** +-INF is 1
 *	10. +0 ** (+anything except 0, NAN)               is +0
 *	11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *	12. +0 ** (-anything except 0, NAN)               is +INF
 *	13. -0 ** (-anything except 0, NAN, odd integer)  is +INF
 *	14. -0 ** (odd integer) = -( +0 ** (odd integer) )
 *	15. +INF ** (+anything except 0,NAN) is +INF
 *	16. +INF ** (-anything except 0,NAN) is +0
 *	17. -INF ** (anything)  = -0 ** (-anything)
 *	18. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *	19. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 * Accuracy:
 *	pow(x,y) returns x**y nearly rounded. In particular
 *			pow(integer,integer)
 *	always returns the correct integer provided it is
 *	representable.
 *
 * Constants :
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static double dmath_ol_pow(double x, double y) {
    static const double bp[] =
        {
            1.0,
            1.5,
        },
                        dp_h[] =
                            {
                                0.0,
                                5.84962487220764160156e-01,
                            }, /* 0x3FE2B803, 0x40000000 */
        dp_l[] =
            {
                0.0,
                1.35003920212974897128e-08,
            },                                                        /* 0x3E4CFDEB, 0x43CFD006 */
        zero = 0.0, one = 1.0, two = 2.0, two53 = 9007199254740992.0, /* 0x43400000, 0x00000000 */
        huge = 1.0e300, tiny = 1.0e-300,
                        /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
        L1 = 5.99999999999994648725e-01,      /* 0x3FE33333, 0x33333303 */
        L2 = 4.28571428578550184252e-01,      /* 0x3FDB6DB6, 0xDB6FABFF */
        L3 = 3.33333329818377432918e-01,      /* 0x3FD55555, 0x518F264D */
        L4 = 2.72728123808534006489e-01,      /* 0x3FD17460, 0xA91D4101 */
        L5 = 2.30660745775561754067e-01,      /* 0x3FCD864A, 0x93C9DB65 */
        L6 = 2.06975017800338417784e-01,      /* 0x3FCA7E28, 0x4A454EEF */
        P1 = 1.66666666666666019037e-01,      /* 0x3FC55555, 0x5555553E */
        P2 = -2.77777777770155933842e-03,     /* 0xBF66C16C, 0x16BEBD93 */
        P3 = 6.61375632143793436117e-05,      /* 0x3F11566A, 0xAF25DE2C */
        P4 = -1.65339022054652515390e-06,     /* 0xBEBBBD41, 0xC5D26BF1 */
        P5 = 4.13813679705723846039e-08,      /* 0x3E663769, 0x72BEA4D0 */
        lg2 = 6.93147180559945286227e-01,     /* 0x3FE62E42, 0xFEFA39EF */
        lg2_h = 6.93147182464599609375e-01,   /* 0x3FE62E43, 0x00000000 */
        lg2_l = -1.90465429995776804525e-09,  /* 0xBE205C61, 0x0CA86C39 */
        ovt = 8.0085662595372944372e-0017,    /* -(1024-log2(ovfl+.5ulp)) */
        cp = 9.61796693925975554329e-01,      /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
        cp_h = 9.61796700954437255859e-01,    /* 0x3FEEC709, 0xE0000000 =(float)cp */
        cp_l = -7.02846165095275826516e-09,   /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
        ivln2 = 1.44269504088896338700e+00,   /* 0x3FF71547, 0x652B82FE =1/ln2 */
        ivln2_h = 1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
        ivln2_l = 1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

    double z, ax, z_h, z_l, p_h, p_l;
    double y1, t1, t2, r, s, t, u, v, w;
    int32_t i, j, k, yisint, n;
    int32_t hx, hy, ix, iy;
    uint32_t lx, ly;

    DMATH_OL_EXTRACT_WORDS(hx, lx, x);
    DMATH_OL_EXTRACT_WORDS(hy, ly, y);
    ix = hx & 0x7fffffff;
    iy = hy & 0x7fffffff;

    /* y==zero: x**0 = 1 */
    if((iy | ly) == 0) return one;

    /* x==1: 1**y = 1, even if y is NaN */
    if(hx == 0x3ff00000 && lx == 0) return one;

    /* y!=zero: result is NaN if either arg is NaN */
    if(ix > 0x7ff00000 || ((ix == 0x7ff00000) && (lx != 0)) || iy > 0x7ff00000 ||
       ((iy == 0x7ff00000) && (ly != 0)))
        return (x + 0.0) + (y + 0.0);

    /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
    yisint = 0;
    if(hx < 0) {
        if(iy >= 0x43400000)
            yisint = 2; /* even integer y */
        else if(iy >= 0x3ff00000) {
            k = (iy >> 20) - 0x3ff; /* exponent */
            if(k > 20) {
                j = dmath_ol_i32(ly >> (52 - k));
                if(((uint32_t)j << (52 - k)) == ly) yisint = 2 - (j & 1);
            } else if(ly == 0) {
                j = iy >> (20 - k);
                if((j << (20 - k)) == iy) yisint = 2 - (j & 1);
            }
        }
    }

    /* special value of y */
    if(ly == 0) {
        if(iy == 0x7ff00000) { /* y is +-inf */
            if(((ix - 0x3ff00000) | lx) == 0)
                return one;           /* (-1)**+-inf is 1 */
            else if(ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
                return (hy >= 0) ? y : zero;
            else /* (|x|<1)**-,+inf = inf,0 */
                return (hy < 0) ? -y : zero;
        }
        if(iy == 0x3ff00000) { /* y is  +-1 */
            if(hy < 0)
                return one / x;
            else
                return x;
        }
        if(hy == 0x40000000) return x * x;     /* y is  2 */
        if(hy == 0x40080000) return x * x * x; /* y is  3 */
        if(hy == 0x40100000) {                 /* y is  4 */
            u = x * x;
            return u * u;
        }
        if(hy == 0x3fe00000) { /* y is  0.5 */
            if(hx >= 0)        /* x >= +0 */
                return dmath_sqrt(x);
        }
    }

    ax = dmath_fabs(x);
    /* special value of x */
    if(lx == 0) {
        if(ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000) {
            z = ax;                 /*x is +-0,+-inf,+-1*/
            if(hy < 0) z = one / z; /* z = (1/|x|) */
            if(hx < 0) {
                if(((ix - 0x3ff00000) | yisint) == 0) {
                    z = (z - z) / (z - z); /* (-1)**non-int is NaN */
                } else if(yisint == 1)
                    z = -z; /* (x<0)**odd = -(|x|**odd) */
            }
            return z;
        }
    }

    /* CYGNUS LOCAL + fdlibm-5.3 fix: This used to be
        n = (hx>>31)+1;
       but ANSI C says a right shift of a signed negative quantity is
       implementation defined.  */
    n = (int32_t)((uint32_t)hx >> 31) - 1;

    /* (x<0)**(non-int) is NaN */
    if((n | yisint) == 0) return (x - x) / (x - x);

    s = one;                              /* s (sign of result -ve**odd) = -1 else = 1 */
    if((n | (yisint - 1)) == 0) s = -one; /* (-ve)**(odd int) */

    /* |y| is huge */
    if(iy > 0x41e00000) {     /* if |y| > 2**31 */
        if(iy > 0x43f00000) { /* if |y| > 2**64, must o/uflow */
            if(ix <= 0x3fefffff) return (hy < 0) ? huge * huge : tiny * tiny;
            if(ix >= 0x3ff00000) return (hy > 0) ? huge * huge : tiny * tiny;
        }
        /* over/underflow if x is not close to one */
        if(ix < 0x3fefffff) return (hy < 0) ? s * huge * huge : s * tiny * tiny;
        if(ix > 0x3ff00000) return (hy > 0) ? s * huge * huge : s * tiny * tiny;
        /* now |1-x| is tiny <= 2**-20, suffice to compute
           log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = ax - one; /* t has 20 trailing zeros */
        w = (t * t) * (0.5 - t * (0.3333333333333333333333 - t * 0.25));
        u = ivln2_h * t; /* ivln2_h has 21 sig. bits */
        v = t * ivln2_l - w * ivln2;
        t1 = u + v;
        DMATH_OL_SET_LOW_WORD(t1, 0);
        t2 = v - (t1 - u);
    } else {
        double ss, s2, s_h, s_l, t_h, t_l;
        n = 0;
        /* take care subnormal number */
        if(ix < 0x00100000) {
            ax *= two53;
            n -= 53;
            DMATH_OL_GET_HIGH_WORD(ix, ax);
        }
        n += ((ix) >> 20) - 0x3ff;
        j = ix & 0x000fffff;
        /* determine interval */
        ix = j | 0x3ff00000; /* normalize ix */
        if(j <= 0x3988E)
            k = 0; /* |x|<dmath_sqrt(3/2) */
        else if(j < 0xBB67A)
            k = 1; /* |x|<dmath_sqrt(3)   */
        else {
            k = 0;
            n += 1;
            ix -= 0x00100000;
        }
        DMATH_OL_SET_HIGH_WORD(ax, ix);

        /* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax - bp[k]; /* bp[0]=1.0, bp[1]=1.5 */
        v = one / (ax + bp[k]);
        ss = u * v;
        s_h = ss;
        DMATH_OL_SET_LOW_WORD(s_h, 0);
        /* t_h=ax+bp[k] High */
        t_h = zero;
        DMATH_OL_SET_HIGH_WORD(t_h, ((ix >> 1) | 0x20000000) + 0x00080000 + (k << 18));
        t_l = ax - (t_h - bp[k]);
        s_l = v * ((u - s_h * t_h) - s_h * t_l);
        /* compute log(ax) */
        s2 = ss * ss;
        r = s2 * s2 * (L1 + s2 * (L2 + s2 * (L3 + s2 * (L4 + s2 * (L5 + s2 * L6)))));
        r += s_l * (s_h + ss);
        s2 = s_h * s_h;
        t_h = 3.0 + s2 + r;
        DMATH_OL_SET_LOW_WORD(t_h, 0);
        t_l = r - ((t_h - 3.0) - s2);
        /* u+v = ss*(1+...) */
        u = s_h * t_h;
        v = s_l * t_h + t_l * ss;
        /* 2/(3log2)*(ss+...) */
        p_h = u + v;
        DMATH_OL_SET_LOW_WORD(p_h, 0);
        p_l = v - (p_h - u);
        z_h = cp_h * p_h; /* cp_h+cp_l = 2/(3*log2) */
        z_l = cp_l * p_h + p_l * cp + dp_l[k];
        /* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
        t = (double)n;
        t1 = (((z_h + z_l) + dp_h[k]) + t);
        DMATH_OL_SET_LOW_WORD(t1, 0);
        t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
    }

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
    y1 = y;
    DMATH_OL_SET_LOW_WORD(y1, 0);
    p_l = (y - y1) * t1 + y * t2;
    p_h = y1 * t1;
    z = p_l + p_h;
    DMATH_OL_EXTRACT_WORDS(j, i, z);
    if(j >= 0x40900000) {               /* z >= 1024 */
        if(((j - 0x40900000) | i) != 0) /* if z > 1024 */
            return s * huge * huge;     /* overflow */
        else {
            if(p_l + ovt > z - p_h) return s * huge * huge; /* overflow */
        }
    } else if((j & 0x7fffffff) >= 0x4090cc00) { /* z <= -1075 */
        if(((j - 0xc090cc00) | i) != 0)         /* z < -1075 */
            return s * tiny * tiny;             /* underflow */
        else {
            if(p_l <= z - p_h) return s * tiny * tiny; /* underflow */
        }
    }
    /*
     * compute 2**(p_h+p_l)
     */
    i = j & 0x7fffffff;
    k = (i >> 20) - 0x3ff;
    n = 0;
    if(i > 0x3fe00000) { /* if |z| > 0.5, set n = [z+0.5] */
        n = j + (0x00100000 >> (k + 1));
        k = ((n & 0x7fffffff) >> 20) - 0x3ff; /* new k for n */
        t = zero;
        DMATH_OL_SET_HIGH_WORD(t, n & ~(0x000fffff >> k));
        n = ((n & 0x000fffff) | 0x00100000) >> (20 - k);
        if(j < 0) n = -n;
        p_h -= t;
    }
    t = p_l + p_h;
    DMATH_OL_SET_LOW_WORD(t, 0);
    u = t * lg2_h;
    v = (p_l - (t - p_h)) * lg2 + t * lg2_l;
    z = u + v;
    w = v - (z - u);
    t = z * z;
    t1 = z - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
    r = (z * t1) / (t1 - two) - (w + z * w);
    z = one - (r - z);
    DMATH_OL_GET_HIGH_WORD(j, z);
    j += n * 0x00100000;
    if(j < 0x00100000)
        z = dmath_ol_scalbn(z, n); /* subnormal output */
    else
        DMATH_OL_SET_HIGH_WORD(z, j);
    return s * z;
}

// OpenLibm src/s_ceil.c (binary64 bit-mask adaptation)
/* @(#)s_ceil.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

// Use a single uint64_t instead of upstream's high/low 32-bit words. These
// rounding kernels avoid floating-to-integer casts and omit inexact-flag operations.
static double dmath_ol_ceil(double x) {
    uint64_t u = dmath_ol_bits(x);
    int e = (int)((u >> 52) & 0x7ff) - 1023;
    if(e >= 52) return x;
    if(e < 0) {
        if((u << 1) == 0) return x;
        return (u >> 63) ? dmath_ol_from_bits(UINT64_C(0x8000000000000000)) : 1.0;
    }
    uint64_t mask = (UINT64_C(1) << (52 - e)) - 1;
    if((u & mask) == 0) return x;
    if(!(u >> 63)) u += mask;
    return dmath_ol_from_bits(u & ~mask);
}

// OpenLibm src/s_floor.c (binary64 bit-mask adaptation)
/* @(#)s_floor.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static double dmath_ol_floor(double x) {
    uint64_t u = dmath_ol_bits(x);
    int e = (int)((u >> 52) & 0x7ff) - 1023;
    if(e >= 52) return x;
    if(e < 0) {
        if((u << 1) == 0) return x;
        return (u >> 63) ? -1.0 : 0.0;
    }
    uint64_t mask = (UINT64_C(1) << (52 - e)) - 1;
    if((u & mask) == 0) return x;
    if(u >> 63) u += mask;
    return dmath_ol_from_bits(u & ~mask);
}

// OpenLibm src/s_trunc.c (binary64 bit-mask adaptation)
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static double dmath_ol_trunc(double x) {
    uint64_t u = dmath_ol_bits(x);
    int e = (int)((u >> 52) & 0x7ff) - 1023;
    if(e >= 52) return x;
    if(e < 0) return dmath_ol_from_bits(u & UINT64_C(0x8000000000000000));
    uint64_t mask = (UINT64_C(1) << (52 - e)) - 1;
    return dmath_ol_from_bits(u & ~mask);
}

double dmath_ceil(double x) { return dmath_ol_result(dmath_ol_ceil(x)); }

double dmath_floor(double x) { return dmath_ol_result(dmath_ol_floor(x)); }

double dmath_trunc(double x) { return dmath_ol_result(dmath_ol_trunc(x)); }

// OpenLibm src/k_rem_pio2.c
/* @(#)k_rem_pio2.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * Binary64-only specialization of dmath_ol_kernel_rem_pio2 (upstream prec = 1).
 * x holds up to three positive 24-bit chunks; e0 is in [-3, 1000].
 * Return N modulo 8 and a two-double remainder y[0] + y[1] = x - N*pi/2.
 * The 24-bit chunks of 2/pi let us skip the large integer part of the
 * product while retaining the low quotient bits and an accurate remainder.
 * Adaptive recomputation retains additional chunks near multiples of pi/2.
 */

/*
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

/*
 * Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
 *
 *		integer array, contains the (24*i)-th to (24*i+23)-th
 *		bit of 2/pi after binary point. The corresponding
 *		floating value is
 *
 *			ipio2[i] * 2^(-24(i+1)).
 *
 * The first 66 words cover the complete binary64 exponent range.
 */
static int dmath_ol_kernel_rem_pio2(double* x, double* y, int e0, int nx) {
    static const int32_t ipio2[] = {
        0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62, 0x95993C, 0x439041, 0xFE5163,
        0xABDEBB, 0xC561B7, 0x246E3A, 0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
        0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41, 0x3991D6, 0x398353, 0x39F49C,
        0x845F8B, 0xBDF928, 0x3B1FF8, 0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
        0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5, 0xF17B3D, 0x0739F7, 0x8A5292,
        0xEA6BFB, 0x5FB11F, 0x8D5D08, 0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
        0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880, 0x4D7327, 0x310606, 0x1556CA,
        0x73A8C9, 0x60E27B, 0xC08C6B,

    };

    static const double PIo2[] = {
        1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
        7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
        5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
        3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
        1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
        1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
        2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
        2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
    };

    static const double zero = 0.0, one = 1.0,
                        two24 = 1.67772160000000000000e+07, /* 0x41700000, 0x00000000 */
        twon24 = 5.96046447753906250000e-08;                /* 0x3E700000, 0x00000000 */

    int32_t jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih;
    double z, fw, f[20], fq[20], q[20];

    /* initialize jk*/
    jk = 4; /* binary64 precision */
    jp = jk;

    /* determine jx,jv,q0, note that 3>q0 */
    jx = nx - 1;
    jv = (e0 - 3) / 24;
    if(jv < 0) jv = 0;
    q0 = e0 - 24 * (jv + 1);

    /* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
    j = jv - jx;
    m = jx + jk;
    for(i = 0; i <= m; i++, j++)
        f[i] = (j < 0) ? zero : (double)ipio2[j];

    /* compute q[0],q[1],...q[jk] */
    for(i = 0; i <= jk; i++) {
        for(j = 0, fw = 0.0; j <= jx; j++)
            fw += x[j] * f[jx + i - j];
        q[i] = fw;
    }

    jz = jk;
recompute:
    /* distill q[] into iq[] reversingly */
    for(i = 0, j = jz, z = q[jz]; j > 0; i++, j--) {
        fw = (double)((int32_t)(twon24 * z));
        iq[i] = (int32_t)(z - two24 * fw);
        z = q[j - 1] + fw;
    }

    /* compute n */
    z = dmath_ol_scalbn(z, q0);           /* actual value of z */
    z -= 8.0 * dmath_ol_floor(z * 0.125); /* trim off integer >= 8 */
    n = (int32_t)z;
    z -= (double)n;
    ih = 0;
    if(q0 > 0) { /* need iq[jz-1] to determine n */
        i = (iq[jz - 1] >> (24 - q0));
        n += i;
        iq[jz - 1] -= i << (24 - q0);
        ih = iq[jz - 1] >> (23 - q0);
    } else if(q0 == 0)
        ih = iq[jz - 1] >> 23;
    else if(z >= 0.5)
        ih = 2;

    if(ih > 0) { /* q > 0.5 */
        n += 1;
        carry = 0;
        for(i = 0; i < jz; i++) { /* compute 1-q */
            j = iq[i];
            if(carry == 0) {
                if(j != 0) {
                    carry = 1;
                    iq[i] = 0x1000000 - j;
                }
            } else
                iq[i] = 0xffffff - j;
        }
        if(q0 > 0) { /* rare case: chance is 1 in 12 */
            switch(q0) {
                case 1: iq[jz - 1] &= 0x7fffff; break;
                case 2: iq[jz - 1] &= 0x3fffff; break;
            }
        }
        if(ih == 2) {
            z = one - z;
            if(carry != 0) z -= dmath_ol_scalbn(one, q0);
        }
    }

    /* check if recomputation is needed */
    if(z == zero) {
        j = 0;
        for(i = jz - 1; i >= jk; i--)
            j |= iq[i];
        if(j == 0) { /* need recomputation */
            for(k = 1; iq[jk - k] == 0; k++)
                ; /* k = no. of terms needed */

            for(i = jz + 1; i <= jz + k; i++) { /* add q[jz+1] to q[jz+k] */
                f[jx + i] = (double)ipio2[jv + i];
                for(j = 0, fw = 0.0; j <= jx; j++)
                    fw += x[j] * f[jx + i - j];
                q[i] = fw;
            }
            jz += k;
            goto recompute;
        }
    }

    /* chop off zero terms */
    if(z == 0.0) {
        jz -= 1;
        q0 -= 24;
        while(iq[jz] == 0) {
            jz--;
            q0 -= 24;
        }
    } else { /* break z into 24-bit if necessary */
        z = dmath_ol_scalbn(z, -q0);
        if(z >= two24) {
            fw = (double)((int32_t)(twon24 * z));
            iq[jz] = (int32_t)(z - two24 * fw);
            jz += 1;
            q0 += 24;
            iq[jz] = (int32_t)fw;
        } else
            iq[jz] = (int32_t)z;
    }

    /* convert integer "bit" chunk to floating-point value */
    fw = dmath_ol_scalbn(one, q0);
    for(i = jz; i >= 0; i--) {
        q[i] = fw * (double)iq[i];
        fw *= twon24;
    }

    /* compute PIo2[0,...,jp]*q[jz,...,0] */
    for(i = jz; i >= 0; i--) {
        for(fw = 0.0, k = 0; k <= jp && k <= jz - i; k++)
            fw += PIo2[k] * q[i + k];
        fq[jz - i] = fw;
    }

    /* compress fq[] into y[] */
    fw = 0.0;
    for(i = jz; i >= 0; i--)
        fw += fq[i];
    DMATH_OL_STRICT_ASSIGN(double, fw, fw);
    y[0] = (ih == 0) ? fw : -fw;
    fw = fq[0] - fw;
    for(i = 1; i <= jz; i++)
        fw += fq[i];
    y[1] = (ih == 0) ? fw : -fw;
    return n & 7;
}

// OpenLibm src/e_rem_pio2.c
/* @(#)e_rem_pio2.c 1.4 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * Optimized by Bruce D. Evans.
 */

/* dmath_ol_rem_pio2(x,y)
 *
 * return the remainder of x rem pi/2 in y[0]+y[1]
 * use dmath_ol_kernel_rem_pio2()
 */

/*
 * invpio2:  53 bits of 2/pi
 * pio2_1:   first  33 bit of pi/2
 * pio2_1t:  pi/2 - pio2_1
 * pio2_2:   second 33 bit of pi/2
 * pio2_2t:  pi/2 - (pio2_1+pio2_2)
 * pio2_3:   third  33 bit of pi/2
 * pio2_3t:  pi/2 - (pio2_1+pio2_2+pio2_3)
 */

static int dmath_ol_rem_pio2(double x, double* y) {
    static const double zero = 0.00000000000000000000e+00, /* 0x00000000, 0x00000000 */
        two24 = 1.67772160000000000000e+07,                /* 0x41700000, 0x00000000 */
        invpio2 = 6.36619772367581382433e-01,              /* 0x3FE45F30, 0x6DC9C883 */
        pio2_1 = 1.57079632673412561417e+00,               /* 0x3FF921FB, 0x54400000 */
        pio2_1t = 6.07710050650619224932e-11,              /* 0x3DD0B461, 0x1A626331 */
        pio2_2 = 6.07710050630396597660e-11,               /* 0x3DD0B461, 0x1A600000 */
        pio2_2t = 2.02226624879595063154e-21,              /* 0x3BA3198A, 0x2E037073 */
        pio2_3 = 2.02226624871116645580e-21,               /* 0x3BA3198A, 0x2E000000 */
        pio2_3t = 8.47842766036889956997e-32;              /* 0x397B839A, 0x252049C1 */

    double z, w, t, r, fn;
    double tx[3], ty[2];
    int32_t e0, i, j, nx, n, ix, hx;
    uint32_t low;

    DMATH_OL_GET_HIGH_WORD(hx, x); /* high word of x */
    ix = hx & 0x7fffffff;
    if(ix <= 0x400f6a7a) {            /* |x| ~<= 5pi/4 */
        if((ix & 0xfffff) == 0x921fb) /* |x| ~= pi/2 or 2pi/2 */
            goto medium;              /* cancellation -- use medium case */
        if(ix <= 0x4002d97c) {        /* |x| ~<= 3pi/4 */
            if(hx > 0) {
                z = x - pio2_1; /* one round good to 85 bits */
                y[0] = z - pio2_1t;
                y[1] = (z - y[0]) - pio2_1t;
                return 1;
            } else {
                z = x + pio2_1;
                y[0] = z + pio2_1t;
                y[1] = (z - y[0]) + pio2_1t;
                return -1;
            }
        } else {
            if(hx > 0) {
                z = x - 2 * pio2_1;
                y[0] = z - 2 * pio2_1t;
                y[1] = (z - y[0]) - 2 * pio2_1t;
                return 2;
            } else {
                z = x + 2 * pio2_1;
                y[0] = z + 2 * pio2_1t;
                y[1] = (z - y[0]) + 2 * pio2_1t;
                return -2;
            }
        }
    }
    if(ix <= 0x401c463b) {       /* |x| ~<= 9pi/4 */
        if(ix <= 0x4015fdbc) {   /* |x| ~<= 7pi/4 */
            if(ix == 0x4012d97c) /* |x| ~= 3pi/2 */
                goto medium;
            if(hx > 0) {
                z = x - 3 * pio2_1;
                y[0] = z - 3 * pio2_1t;
                y[1] = (z - y[0]) - 3 * pio2_1t;
                return 3;
            } else {
                z = x + 3 * pio2_1;
                y[0] = z + 3 * pio2_1t;
                y[1] = (z - y[0]) + 3 * pio2_1t;
                return -3;
            }
        } else {
            if(ix == 0x401921fb) /* |x| ~= 4pi/2 */
                goto medium;
            if(hx > 0) {
                z = x - 4 * pio2_1;
                y[0] = z - 4 * pio2_1t;
                y[1] = (z - y[0]) - 4 * pio2_1t;
                return 4;
            } else {
                z = x + 4 * pio2_1;
                y[0] = z + 4 * pio2_1t;
                y[1] = (z - y[0]) + 4 * pio2_1t;
                return -4;
            }
        }
    }
    if(ix < 0x413921fb) { /* |x| ~< 2^20*(pi/2), medium size */
    medium:
        /* Use a specialized rint() to get fn.  Assume round-to-nearest. */
        DMATH_OL_STRICT_ASSIGN(double, fn, x* invpio2 + 0x1.8p52);
        fn = fn - 0x1.8p52;
        n = (int32_t)fn;
        r = x - fn * pio2_1;
        w = fn * pio2_1t; /* 1st round good to 85 bit */
        {
            uint32_t high;
            j = ix >> 20;
            y[0] = r - w;
            DMATH_OL_GET_HIGH_WORD(high, y[0]);
            i = j - ((high >> 20) & 0x7ff);
            if(i > 16) { /* 2nd iteration needed, good to 118 */
                t = r;
                w = fn * pio2_2;
                r = t - w;
                w = fn * pio2_2t - ((t - r) - w);
                y[0] = r - w;
                DMATH_OL_GET_HIGH_WORD(high, y[0]);
                i = j - ((high >> 20) & 0x7ff);
                if(i > 49) { /* 3rd iteration need, 151 bits acc */
                    t = r;   /* will cover all possible cases */
                    w = fn * pio2_3;
                    r = t - w;
                    w = fn * pio2_3t - ((t - r) - w);
                    y[0] = r - w;
                }
            }
        }
        y[1] = (r - y[0]) - w;
        return n;
    }
    /*
     * all other (large) arguments
     */
    if(ix >= 0x7ff00000) { /* x is inf or NaN */
        y[0] = y[1] = x - x;
        return 0;
    }
    /* Normalize |x| to [2^23, 2^24). e0 can be negative, so multiply
     * instead of left-shifting a signed exponent. */
    DMATH_OL_GET_LOW_WORD(low, x);
    e0 = (ix >> 20) - 1046; /* e0 = ilogb(x)-23; */
    DMATH_OL_INSERT_WORDS(z, ix - (e0 * 0x00100000), low);
    for(i = 0; i < 2; i++) {
        tx[i] = (double)((int32_t)(z));
        z = (z - tx[i]) * two24;
    }
    tx[2] = z;
    nx = 3;
    while(tx[nx - 1] == zero)
        nx--; /* skip zero term */
    n = dmath_ol_kernel_rem_pio2(tx, ty, e0, nx);
    if(hx < 0) {
        y[0] = -ty[0];
        y[1] = -ty[1];
        return -n;
    }
    y[0] = ty[0];
    y[1] = ty[1];
    return n;
}

// OpenLibm src/k_sin.c
/* @(#)k_sin.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_kernel_sin( x, y, iy)
 * kernel sin function on ~[-pi/4, pi/4] (except on -0), pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input iy indicates whether y is 0. (if iy=0, y assume to be 0).
 *
 * Algorithm
 *	1. Since sin(-x) = -sin(x), we need only to consider positive x.
 *	2. Callers must return sin(-0) = -0 without calling here since our
 *	   odd polynomial is not evaluated in a way that preserves -0.
 *	   Callers may do the optimization sin(x) ~ x for tiny x.
 *	3. sin(x) is approximated by a polynomial of degree 13 on
 *	   [0,pi/4]
 *		  	         3            13
 *	   	sin(x) ~ x + S1*x + ... + S6*x
 *	   where
 *
 * 	|sin(x)         2     4     6     8     10     12  |     -58
 * 	|----- - (1+S1*x +S2*x +S3*x +S4*x +S5*x  +S6*x   )| <= 2
 * 	|  x 					           |
 *
 *	4. sin(x+y) = sin(x) + sin'(x')*y
 *		    ~ sin(x) + (1-x*x/2)*y
 *	   For better accuracy, let
 *		     3      2      2      2      2
 *		r = x *(S2+x *(S3+x *(S4+x *(S5+x *S6))))
 *	   then                   3    2
 *		sin(x) = x + (S1*x + (x *(r-y/2)+y))
 */

static double dmath_ol_kernel_sin(double x, double y, int iy) {
    static const double half = 5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
        S1 = -1.66666666666666324348e-01,                  /* 0xBFC55555, 0x55555549 */
        S2 = 8.33333333332248946124e-03,                   /* 0x3F811111, 0x1110F8A6 */
        S3 = -1.98412698298579493134e-04,                  /* 0xBF2A01A0, 0x19C161D5 */
        S4 = 2.75573137070700676789e-06,                   /* 0x3EC71DE3, 0x57B1FE7D */
        S5 = -2.50507602534068634195e-08,                  /* 0xBE5AE5E6, 0x8A2B9CEB */
        S6 = 1.58969099521155010221e-10;                   /* 0x3DE5D93A, 0x5ACFD57C */

    double z, r, v, w;

    z = x * x;
    w = z * z;
    r = S2 + z * (S3 + z * S4) + z * w * (S5 + z * S6);
    v = z * x;
    if(iy == 0)
        return x + v * (S1 + z * r);
    else
        return x - ((z * (half * y - v * r) - y) - v * S1);
}

// OpenLibm src/k_cos.c
/* @(#)k_cos.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * dmath_ol_kernel_cos( x,  y )
 * kernel cos function on [-pi/4, pi/4], pi/4 ~ 0.785398164
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 *
 * Algorithm
 *	1. Since cos(-x) = cos(x), we need only to consider positive x.
 *	2. if x < 2^-27 (hx<0x3e400000 0), return 1 with inexact if x!=0.
 *	3. cos(x) is approximated by a polynomial of degree 14 on
 *	   [0,pi/4]
 *		  	                 4            14
 *	   	cos(x) ~ 1 - x*x/2 + C1*x + ... + C6*x
 *	   where the remez error is
 *
 * 	|              2     4     6     8     10    12     14 |     -58
 * 	|cos(x)-(1-.5*x +C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  )| <= 2
 * 	|    					               |
 *
 * 	               4     6     8     10    12     14
 *	4. let r = C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  , then
 *	       cos(x) ~ 1 - x*x/2 + r
 *	   since cos(x+y) ~ cos(x) - sin(x)*y
 *			  ~ cos(x) - x*y,
 *	   a correction term is necessary in cos(x) and hence
 *		cos(x+y) = 1 - (x*x/2 - (r - x*y))
 *	   For better accuracy, rearrange to
 *		cos(x+y) ~ w + (tmp + (r-x*y))
 *	   where w = 1 - x*x/2 and tmp is a tiny correction term
 *	   (1 - x*x/2 == w + tmp exactly in infinite precision).
 *	   The exactness of w + tmp in infinite precision depends on w
 *	   and tmp having the same precision as x.  If they have extra
 *	   precision due to compiler bugs, then the extra precision is
 *	   only good provided it is retained in all terms of the final
 *	   expression for cos().  Retention happens in all cases tested
 *	   under FreeBSD, so don't pessimize things by forcibly clipping
 *	   any extra precision in w.
 */

static double dmath_ol_kernel_cos(double x, double y) {
    static const double one = 1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
        C1 = 4.16666666666666019037e-02,                  /* 0x3FA55555, 0x5555554C */
        C2 = -1.38888888888741095749e-03,                 /* 0xBF56C16C, 0x16C15177 */
        C3 = 2.48015872894767294178e-05,                  /* 0x3EFA01A0, 0x19CB1590 */
        C4 = -2.75573143513906633035e-07,                 /* 0xBE927E4F, 0x809C52AD */
        C5 = 2.08757232129817482790e-09,                  /* 0x3E21EE9E, 0xBDB4B1C4 */
        C6 = -1.13596475577881948265e-11;                 /* 0xBDA8FAE9, 0xBE8838D4 */

    double hz, z, r, w;

    z = x * x;
    w = z * z;
    r = z * (C1 + z * (C2 + z * C3)) + w * w * (C4 + z * (C5 + z * C6));
    hz = 0.5 * z;
    w = one - hz;
    return w + (((one - w) - hz) + (z * r - x * y));
}

// OpenLibm src/k_tan.c
/* @(#)k_tan.c 1.5 04/04/22 SMI */

/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* INDENT OFF */

/* dmath_ol_kernel_tan( x, y, k )
 * kernel tan function on ~[-pi/4, pi/4] (except on -0), pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input k indicates whether tan (if k = 1) or -1/tan (if k = -1) is returned.
 *
 * Algorithm
 *	1. Since tan(-x) = -tan(x), we need only to consider positive x.
 *	2. Callers must return tan(-0) = -0 without calling here since our
 *	   odd polynomial is not evaluated in a way that preserves -0.
 *	   Callers may do the optimization tan(x) ~ x for tiny x.
 *	3. tan(x) is approximated by a odd polynomial of degree 27 on
 *	   [0,0.67434]
 *		  	         3             27
 *	   	tan(x) ~ x + T1*x + ... + T13*x
 *	   where
 *
 * 	        |tan(x)         2     4            26   |     -59.2
 * 	        |----- - (1+T1*x +T2*x +.... +T13*x    )| <= 2
 * 	        |  x 					|
 *
 *	   Note: tan(x+y) = tan(x) + tan'(x)*y
 *		          ~ tan(x) + (1+x*x)*y
 *	   Therefore, for better accuracy in computing tan(x+y), let
 *		     3      2      2       2       2
 *		r = x *(T2+x *(T3+x *(...+x *(T12+x *T13))))
 *	   then
 *		 		    3    2
 *		tan(x+y) = x + (T1*x + (x *(r+y)+y))
 *
 *      4. For x in [0.67434,pi/4],  let y = pi/4 - x, then
 *		tan(x) = tan(pi/4-y) = (1-tan(y))/(1+tan(y))
 *		       = 1 - 2*(tan(y) - (tan(y)^2)/(1+tan(y)))
 */

static double dmath_ol_kernel_tan(double x, double y, int iy) {
    static const double xxx[] = {
        3.33333333333334091986e-01,               /* 3FD55555, 55555563 */
        1.33333333333201242699e-01,               /* 3FC11111, 1110FE7A */
        5.39682539762260521377e-02,               /* 3FABA1BA, 1BB341FE */
        2.18694882948595424599e-02,               /* 3F9664F4, 8406D637 */
        8.86323982359930005737e-03,               /* 3F8226E3, E96E8493 */
        3.59207910759131235356e-03,               /* 3F6D6D22, C9560328 */
        1.45620945432529025516e-03,               /* 3F57DBC8, FEE08315 */
        5.88041240820264096874e-04,               /* 3F4344D8, F2F26501 */
        2.46463134818469906812e-04,               /* 3F3026F7, 1A8D1068 */
        7.81794442939557092300e-05,               /* 3F147E88, A03792A6 */
        7.14072491382608190305e-05,               /* 3F12B80F, 32F0A7E9 */
        -1.85586374855275456654e-05,              /* BEF375CB, DB605373 */
        2.59073051863633712884e-05,               /* 3EFB2A70, 74BF7AD4 */
        /* one */ 1.00000000000000000000e+00,     /* 3FF00000, 00000000 */
        /* xxx[14] */ 7.85398163397448278999e-01, /* 3FE921FB, 54442D18 */
        /* xxx[15] */ 3.06161699786838301793e-17  /* 3C81A626, 33145C07 */
    };
    /* INDENT ON */

    double z, r, v, w, s;
    int32_t ix, hx;

    DMATH_OL_GET_HIGH_WORD(hx, x);
    ix = hx & 0x7fffffff;  /* high word of |x| */
    if(ix >= 0x3FE59428) { /* |x| >= 0.6744 */
        if(hx < 0) {
            x = -x;
            y = -y;
        }
        z = xxx[14] - x;
        w = xxx[15] - y;
        x = z + w;
        y = 0.0;
    }
    z = x * x;
    w = z * z;
    /*
     * Break x^5*(xxx[1]+x^2*xxx[2]+...) into
     * x^5(xxx[1]+x^4*xxx[3]+...+x^20*xxx[11]) +
     * x^5(x^2*(xxx[2]+x^4*xxx[4]+...+x^22*[T12]))
     */
    r = xxx[1] + w * (xxx[3] + w * (xxx[5] + w * (xxx[7] + w * (xxx[9] + w * xxx[11]))));
    v = z * (xxx[2] + w * (xxx[4] + w * (xxx[6] + w * (xxx[8] + w * (xxx[10] + w * xxx[12])))));
    s = z * x;
    r = y + z * (s * (r + v) + y);
    r += xxx[0] * s;
    w = x + r;
    if(ix >= 0x3FE59428) {
        v = (double)iy;
        return (hx < 0 ? -1.0 : 1.0) * (v - 2.0 * (x - (w * w / (w + v) - r)));
    }
    if(iy == 1)
        return w;
    else {
        /*
         * if allow error up to 2 ulp, simply return
         * -1.0 / (x+r) here
         */
        /* compute -1.0 / (x+r) accurately */
        double a, t;
        z = w;
        DMATH_OL_SET_LOW_WORD(z, 0);
        v = r - (z - x);  /* z+v = r+x */
        t = a = -1.0 / w; /* a = -1.0/w */
        DMATH_OL_SET_LOW_WORD(t, 0);
        s = 1.0 + t * z;
        return t + a * (s + t * v);
    }
}

// OpenLibm src/s_sin.c
/* @(#)s_sin.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *	dmath_ol_kernel_sin		... sine function on [-pi/4,pi/4]
 *	dmath_ol_kernel_cos		... cose function on [-pi/4,pi/4]
 *	dmath_ol_rem_pio2	... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the dmath_ol_sin, cos and tan respectively on
 *	[-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *	in [-pi/4 , +pi/4], and let n = k mod 4.
 *	We have
 *
 *          n        dmath_ol_sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *	    0	       S	   C		 T
 *	    1	       C	  -S		-1/T
 *	    2	      -S	  -C		 T
 *	    3	      -C	   S		-1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of dmath_ol_sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *	TRIG(x) returns trig(x) nearly rounded
 */

static double dmath_ol_sin(double x) {

    double y[2], z = 0.0;
    int32_t n, ix;

    /* High word of x. */
    DMATH_OL_GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix < 0x3e500000) /* |x| < 2**-26 */
        {
            if((int)x == 0) return x;
        } /* generate inexact */
        return dmath_ol_kernel_sin(x, z, 0);
    }

    /* dmath_ol_sin(Inf or NaN) is NaN */
    else if(ix >= 0x7ff00000)
        return x - x;

    /* argument reduction needed */
    else {
        n = dmath_ol_rem_pio2(x, y);
        switch(n & 3) {
            case 0: return dmath_ol_kernel_sin(y[0], y[1], 1);
            case 1: return dmath_ol_kernel_cos(y[0], y[1]);
            case 2: return -dmath_ol_kernel_sin(y[0], y[1], 1);
            default: return -dmath_ol_kernel_cos(y[0], y[1]);
        }
    }
}

// OpenLibm src/s_cos.c
/* @(#)s_cos.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_cos(x)
 * Return cosine function of x.
 *
 * kernel function:
 *	dmath_ol_kernel_sin		... sine function on [-pi/4,pi/4]
 *	dmath_ol_kernel_cos		... cosine function on [-pi/4,pi/4]
 *	dmath_ol_rem_pio2	... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, dmath_ol_cos and tan respectively on
 *	[-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *	in [-pi/4 , +pi/4], and let n = k mod 4.
 *	We have
 *
 *          n        sin(x)      dmath_ol_cos(x)        tan(x)
 *     ----------------------------------------------------------
 *	    0	       S	   C		 T
 *	    1	       C	  -S		-1/T
 *	    2	      -S	  -C		 T
 *	    3	      -C	   S		-1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, dmath_ol_cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *	TRIG(x) returns trig(x) nearly rounded
 */

static double dmath_ol_cos(double x) {

    double y[2], z = 0.0;
    int32_t n, ix;

    /* High word of x. */
    DMATH_OL_GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix < 0x3e46a09e)               /* if x < 2**-27 * sqrt(2) */
            if(((int)x) == 0) return 1.0; /* generate inexact */
        return dmath_ol_kernel_cos(x, z);
    }

    /* dmath_ol_cos(Inf or NaN) is NaN */
    else if(ix >= 0x7ff00000)
        return x - x;

    /* argument reduction needed */
    else {
        n = dmath_ol_rem_pio2(x, y);
        switch(n & 3) {
            case 0: return dmath_ol_kernel_cos(y[0], y[1]);
            case 1: return -dmath_ol_kernel_sin(y[0], y[1], 1);
            case 2: return -dmath_ol_kernel_cos(y[0], y[1]);
            default: return dmath_ol_kernel_sin(y[0], y[1], 1);
        }
    }
}

// OpenLibm src/s_tan.c
/* @(#)s_tan.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* dmath_ol_tan(x)
 * Return tangent function of x.
 *
 * kernel function:
 *	dmath_ol_kernel_tan		... tangent function on [-pi/4,pi/4]
 *	dmath_ol_rem_pio2	... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and dmath_ol_tan respectively on
 *	[-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *	in [-pi/4 , +pi/4], and let n = k mod 4.
 *	We have
 *
 *          n        sin(x)      cos(x)        dmath_ol_tan(x)
 *     ----------------------------------------------------------
 *	    0	       S	   C		 T
 *	    1	       C	  -S		-1/T
 *	    2	      -S	  -C		 T
 *	    3	      -C	   S		-1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or dmath_ol_tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *	TRIG(x) returns trig(x) nearly rounded
 */

static double dmath_ol_tan(double x) {

    double y[2], z = 0.0;
    int32_t n, ix;

    /* High word of x. */
    DMATH_OL_GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        if(ix < 0x3e400000)           /* x < 2**-27 */
            if((int)x == 0) return x; /* generate inexact */
        return dmath_ol_kernel_tan(x, z, 1);
    }

    /* dmath_ol_tan(Inf or NaN) is NaN */
    else if(ix >= 0x7ff00000)
        return x - x; /* NaN */

    /* argument reduction needed */
    else {
        n = dmath_ol_rem_pio2(x, y);
        return dmath_ol_kernel_tan(y[0], y[1], 1 - ((n & 1) << 1)); /*   1 -- n even
                                                            -1 -- n odd */
    }
}

double dmath_sin(double x) { return dmath_ol_result(dmath_ol_sin(x)); }

double dmath_cos(double x) { return dmath_ol_result(dmath_ol_cos(x)); }

double dmath_tan(double x) { return dmath_ol_result(dmath_ol_tan(x)); }

// OpenLibm src/s_sincos.c (reuse the separate kernels and their tiny-input cutoffs)
/* @(#)s_sincos.c 5.1 13/07/15 */
/* See openlibm LICENSE.md for full license details.
 *
 * ====================================================
 * This file is derived from fdlibm:
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 *
 * ====================================================
 * Copyright (C) 2013 Elliot Saba. All rights reserved.
 *
 * Developed at the University of Washington.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

void dmath_sincos(double x, double* sin, double* cos) {
    uint32_t ix = (uint32_t)(dmath_ol_bits(x) >> 32) & 0x7fffffff;
    if(ix <= 0x3fe921fb) {
        *sin = ix < 0x3e500000 ? x : dmath_ol_kernel_sin(x, 0.0, 0);
        *cos = ix < 0x3e46a09e ? 1.0 : dmath_ol_kernel_cos(x, 0.0);
        return;
    }
    if(ix >= 0x7ff00000) {
        *sin = *cos = dmath_ol_result(x - x);
        return;
    }
    double y[2];
    int n = dmath_ol_rem_pio2(x, y);
    double s = dmath_ol_kernel_sin(y[0], y[1], 1);
    double c = dmath_ol_kernel_cos(y[0], y[1]);
    switch((unsigned)n & 3) {
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
        default:
            *sin = -c;
            *cos = s;
            break;
    }
}

double dmath_exp(double x) { return dmath_ol_result(dmath_ol_exp(x)); }

double dmath_exp2(double x) { return dmath_ol_result(dmath_ol_exp2(x)); }

double dmath_log(double x) { return dmath_ol_result(dmath_ol_log(x)); }

double dmath_log2(double x) { return dmath_ol_result(dmath_ol_log2(x)); }

double dmath_log10(double x) { return dmath_ol_result(dmath_ol_log10(x)); }

double dmath_pow(double x, double y) { return dmath_ol_result(dmath_ol_pow(x, y)); }

double dmath_exp10(double x) { return dmath_pow(10.0, x); }

double dmath_log_base(double x, double base) {
    return dmath_ol_result(dmath_ol_log(x) / dmath_ol_log(base));
}

#undef DMATH_OL_GET_HIGH_WORD
#undef DMATH_OL_GET_LOW_WORD
#undef DMATH_OL_EXTRACT_WORDS
#undef DMATH_OL_INSERT_WORDS
#undef DMATH_OL_SET_HIGH_WORD
#undef DMATH_OL_SET_LOW_WORD
#undef DMATH_OL_STRICT_ASSIGN

