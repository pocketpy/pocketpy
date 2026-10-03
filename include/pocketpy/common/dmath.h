#pragma once

#include <float.h>
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(double) == 8 && FLT_RADIX == 2 && DBL_MANT_DIG == 53 &&
                   DBL_MAX_EXP == 1024 && DBL_MIN_EXP == -1021,
               "dmath requires IEEE 754 binary64");
#if defined(FLT_EVAL_METHOD) && FLT_EVAL_METHOD != 0
#error "dmath requires binary64 evaluation; use SSE2 (-msse2 -mfpmath=sse) on x86"
#endif
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0) || \
    defined(_M_FP_FAST)
#error "dmath cannot be compiled with fast-math or finite-math-only"
#endif

static inline uint64_t pk_dmath_bits(double x) {
    uint64_t u;
    memcpy(&u, &x, sizeof(u));
    return u;
}

static inline double pk_dmath_from_bits(uint64_t u) {
    double x;
    memcpy(&x, &u, sizeof(x));
    return x;
}

// Construct constants without arithmetic: directed rounding can turn an
// overflowing product into a finite number, making the old infinity * 0 zero.
// The chosen NaN encoding is an implementation detail, not a result contract.
#define DMATH_INFINITY (pk_dmath_from_bits(UINT64_C(0x7ff0000000000000)))
#define DMATH_NAN (pk_dmath_from_bits(UINT64_C(0x7ff8000000000000)))
#define DMATH_PI 3.1415926535897932384
#define DMATH_E 2.7182818284590452354
#define DMATH_DEG2RAD 0.017453292519943295
#define DMATH_RAD2DEG 57.29577951308232
#define DMATH_EPSILON 1e-10
#define DMATH_LOG2_E 1.4426950408889634

// All dmath functions require round-to-nearest-even, gradual
// underflow (FTZ/DAZ disabled), and no fast-math or excess intermediate precision.
// Determinism requires identical finite result bits, including signed zero.
// NaNs are checked by classification, infinities by classification and sign;
// nonfinite bit patterns and NaN payload/signaling propagation are not promised.
// Classification functions inspect bits without floating-point arithmetic.
// Python exceptions belong to the bindings. See 3rd/openlibm/README.md.
double dmath_exp2(double x);
double dmath_log2(double x);

double dmath_exp(double x);
double dmath_exp10(double x);
double dmath_log(double x);
double dmath_log10(double x);
double dmath_log_base(double x, double base);
double dmath_pow(double base, double exp);
double dmath_sqrt(double x);
double dmath_cbrt(double x);

void dmath_sincos(double x, double* sin, double* cos);
double dmath_sin(double x);
double dmath_cos(double x);
double dmath_tan(double x);
double dmath_asin(double x);
double dmath_acos(double x);
double dmath_atan(double x);
double dmath_atan2(double y, double x);

int dmath_isinf(double x);
int dmath_isnan(double x);
int dmath_isnormal(double x);
int dmath_isfinite(double x);

double dmath_fmod(double x, double y);
double dmath_copysign(double x, double y);

double dmath_fabs(double x);
// Fixed OpenLibm binary64 rounding: preserve signed zero and infinity,
// canonicalize NaNs, and return exact integral doubles without integer casts.
double dmath_ceil(double x);
double dmath_floor(double x);
double dmath_trunc(double x);
double dmath_modf(double x, double* intpart);

// Ignore a single NaN; two NaNs produce NaN. For a zero tie,
// fmin returns -0 if either operand is -0; fmax returns +0 if either is +0.
double dmath_fmin(double x, double y);
double dmath_fmax(double x, double y);
