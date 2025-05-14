#ifndef _MATH_H
#define _MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#define __NEED_float_t
#define __NEED_double_t

#ifndef _HUGE_ENUF
    #define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VALF INFINITY
#define HUGE_VAL  ((double)INFINITY)
#define HUGE_VALL ((long double)INFINITY)
#define NAN        ((float)(INFINITY * 0.0F))

#define MATH_ERRNO  1
#define MATH_ERREXCEPT 2
#define math_errhandling 2

#define FP_ILOGBNAN (-1-0x7fffffff)
#define FP_ILOGB0 FP_ILOGBNAN
#define ULLONG_NSHIFT 0xFFFFFFFFFFFFFFFF
#define ULLONG_SHIFT1 0x7FFFFFFFFFFFFFFF

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#ifdef __FP_FAST_FMA
#define FP_FAST_FMA 1
#endif

#ifdef __FP_FAST_FMAF
#define FP_FAST_FMAF 1
#endif

#ifdef __FP_FAST_FMAL
#define FP_FAST_FMAL 1
#endif

#define FLT_EVAL_METHOD 0

/* Support non-nearest rounding mode.  */
#define WANT_ROUNDING 1
/* Support signaling NaNs.  */
#define WANT_SNAN 0

#if WANT_SNAN
#error SNaN is unsupported
#else
#define issignalingf_inline(x) 0
#define issignaling_inline(x) 0
#endif

#define predict_true(x) (x)
#define predict_false(x) (x)

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
typedef float  float_t;
typedef double double_t;

int __fpclassify(double);
int __fpclassifyf(float);
int __fpclassifyl(long double);

static __inline unsigned __FLOAT_BITS(float __f)
{
	union {float __f; unsigned __i;} __u;
	__u.__f = __f;
	return __u.__i;
}
static __inline unsigned long long __DOUBLE_BITS(double __f)
{
	union {double __f; unsigned long long __i;} __u;
	__u.__f = __f;
	return __u.__i;
}

#define fpclassify(x) ( \
	sizeof(x) == sizeof(float) ? __fpclassifyf(x) : \
	sizeof(x) == sizeof(double) ? __fpclassify(x) : \
	__fpclassifyl(x) )

#define isinf(x) ( \
	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) == 0x7f800000 : \
	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & ULLONG_SHIFT1) == 0x7ffULL<<52 : \
	__fpclassifyl(x) == FP_INFINITE)

#define isnan(x) ( \
	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000 : \
	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & ULLONG_SHIFT1) > 0x7ffULL<<52 : \
	__fpclassifyl(x) == FP_NAN)

#define isnormal(x) ( \
	sizeof(x) == sizeof(float) ? ((__FLOAT_BITS(x)+0x00800000) & 0x7fffffff) >= 0x01000000 : \
	sizeof(x) == sizeof(double) ? ((__DOUBLE_BITS(x)+(1ULL<<52)) & -1ULL>>1) >= 1ULL<<53 : \
	__fpclassifyl(x) == FP_NORMAL)

#define isfinite(x) ( \
	sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000 : \
	sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & ULLONG_SHIFT1) < 0x7ffULL<<52 : \
	__fpclassifyl(x) > FP_INFINITE)

int __signbit(double);
int __signbitf(float);
int __signbitl(long double);

#define signbit(x) ( \
	sizeof(x) == sizeof(float) ? (int)(__FLOAT_BITS(x)>>31) : \
	sizeof(x) == sizeof(double) ? (int)(__DOUBLE_BITS(x)>>63) : \
	__signbitl(x) )

#define isunordered(x,y) (isnan((x)) ? ((void)(y),1) : isnan((y)))

#define __ISREL_DEF(rel, op, type) \
static __inline int __is##rel(type __x, type __y) \
{ return !isunordered(__x,__y) && __x op __y; }

__ISREL_DEF(lessf, <, float_t)
__ISREL_DEF(less, <, double_t)
__ISREL_DEF(lessl, <, long double)
__ISREL_DEF(lessequalf, <=, float_t)
__ISREL_DEF(lessequal, <=, double_t)
__ISREL_DEF(lessequall, <=, long double)
__ISREL_DEF(lessgreaterf, !=, float_t)
__ISREL_DEF(lessgreater, !=, double_t)
__ISREL_DEF(lessgreaterl, !=, long double)
__ISREL_DEF(greaterf, >, float_t)
__ISREL_DEF(greater, >, double_t)
__ISREL_DEF(greaterl, >, long double)
__ISREL_DEF(greaterequalf, >=, float_t)
__ISREL_DEF(greaterequal, >=, double_t)
__ISREL_DEF(greaterequall, >=, long double)

#define __tg_pred_2(x, y, p) ( \
	sizeof((x)+(y)) == sizeof(float) ? p##f(x, y) : \
	sizeof((x)+(y)) == sizeof(double) ? p(x, y) : \
	p##l(x, y) )

#define isless(x, y)            __tg_pred_2(x, y, __isless)
#define islessequal(x, y)       __tg_pred_2(x, y, __islessequal)
#define islessgreater(x, y)     __tg_pred_2(x, y, __islessgreater)
#define isgreater(x, y)         __tg_pred_2(x, y, __isgreater)
#define isgreaterequal(x, y)    __tg_pred_2(x, y, __isgreaterequal)

/* Evaluate an expression as the specified type. With standard excess
   precision handling a type cast or assignment is enough (with
   -ffloat-store an assignment is required, in old compilers argument
   passing and return statement may not drop excess precision).  */

static inline float eval_as_float(float x)
{
	float y = x;
	return y;
}

static inline double eval_as_double(double x)
{
	double y = x;
	return y;
}

/* fp_barrier returns its input, but limits code transformations
	as if it had a side-effect (e.g. observable io) and returned
	an arbitrary value.  */

#ifndef fp_barrierf
#define fp_barrierf fp_barrierf
static inline float fp_barrierf(float x)
{
	volatile float y = x;
	return y;
}
#endif

#ifndef fp_barrier
#define fp_barrier fp_barrier
static inline double fp_barrier(double x)
{
	volatile double y = x;
	return y;
}
#endif

#ifndef fp_barrierl
#define fp_barrierl fp_barrierl
static inline long double fp_barrierl(long double x)
{
	volatile long double y = x;
	return y;
}
#endif

/* fp_force_eval ensures that the input value is computed when that's
	otherwise unused.  To prevent the constant folding of the input
	expression, an additional fp_barrier may be needed or a compilation
	mode that does so (e.g. -frounding-math in gcc). Then it can be
	used to evaluate an expression for its fenv side-effects only.   */

#ifndef fp_force_evalf
#define fp_force_evalf fp_force_evalf
static inline void fp_force_evalf(float x)
{
	volatile float y;
	y = x;
}
#endif

#ifndef fp_force_eval
#define fp_force_eval fp_force_eval
static inline void fp_force_eval(double x)
{
	volatile double y;
	y = x;
}
#endif

#ifndef fp_force_evall
#define fp_force_evall fp_force_evall
static inline void fp_force_evall(long double x)
{
	volatile long double y;
	y = x;
}
#endif

#define FORCE_EVAL(x) do {                        \
	if (sizeof(x) == sizeof(float)) {         \
		fp_force_evalf(x);                \
	} else if (sizeof(x) == sizeof(double)) { \
		fp_force_eval(x);                 \
	} else {                                  \
		fp_force_evall(x);                \
	}                                         \
} while(0)

typedef union {float _f; uint32_t _i;}asuint_union;
typedef union {uint32_t _i; float _f;}asfloat_union;
typedef union {double _f; uint64_t _i;}asuint64_union;
typedef union {uint64_t _i; double _f;}asdouble_union;
#define asuint(f) ((asuint_union){f})._i
#define asfloat(i) ((asfloat_union){i})._f
#define asuint64(f) ((asuint64_union){f})._i
#define asdouble(i) ((asdouble_union){i})._f

#define EXTRACT_WORDS(hi,lo,d)                    \
do {                                              \
	uint64_t __u = asuint64(d);                     \
	(hi) = __u >> 32;                               \
	(lo) = (uint32_t)__u;                           \
} while (0)

#define GET_HIGH_WORD(hi,d)                       \
do {                                              \
	(hi) = asuint64(d) >> 32;                       \
} while (0)

#define GET_LOW_WORD(lo,d)                        \
do {                                              \
	(lo) = (uint32_t)asuint64(d);                   \
} while (0)

#define INSERT_WORDS(d,hi,lo)                     \
do {                                              \
	(d) = asdouble(((uint64_t)(hi)<<32) | (uint32_t)(lo)); \
} while (0)

#define SET_HIGH_WORD(d,hi)                       \
	INSERT_WORDS(d, hi, (uint32_t)asuint64(d))

#define SET_LOW_WORD(d,lo)                        \
	INSERT_WORDS(d, asuint64(d)>>32, lo)

#define GET_FLOAT_WORD(w,d)                       \
do {                                              \
	(w) = asuint(d);                                \
} while (0)

#define SET_FLOAT_WORD(d,w)                       \
do {                                              \
	(d) = asfloat(w);                               \
} while (0)
   
int    __rem_pio2_large(double*, double*, int, int, int);
int    __rem_pio2(double, double*);
double __sin(double, double, int);
double __cos(double, double);
double __tan(double, double, int);

/* error handling functions */
float __math_xflowf(uint32_t, float);
float __math_uflowf(uint32_t);
float __math_oflowf(uint32_t);
float __math_divzerof(uint32_t);
float __math_invalidf(float);
double __math_xflow(uint32_t, double);
double __math_uflow(uint32_t);
double __math_oflow(uint32_t);
double __math_divzero(uint32_t);
double __math_invalid(double);
#if LDBL_MANT_DIG != DBL_MANT_DIG
long double __math_invalidl(long double);
#endif

double      acos(double);
double      asin(double);
double      atan(double);
double      atan2(double, double);
double      cbrt(double);
double      ceil(double);
double      cos(double);
double		degrees(double);
double      exp(double);
double      fabs(double);
int			factorial(int);
double      floor(double);
double      fmax(double, double);
double      fmin(double, double);
double      fmod(double, double);
double		fsum(double*, int);
int			gcd(int, int);
double      log(double);
double      log10(double);
double      log2(double);
double      modf(double, double *);
double      pow(double, double);
double		radians(double);
double      scalbn(double, int);
double      sin(double);
double      sqrt(double);
double      tan(double);
double      trunc(double);

#define FLT_TRUE_MIN 1.40129846432481707092e-45F
#define FLT_MIN 1.17549435082228750797e-38F
#define FLT_MAX 3.40282346638528859812e+38F
#define FLT_EPSILON 1.1920928955078125e-07F

#define FLT_MANT_DIG 24
#define FLT_MIN_EXP (-125)
#define FLT_MAX_EXP 128
#define FLT_HAS_SUBNORM 1

#define FLT_DIG 6
#define FLT_DECIMAL_DIG 9
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_10_EXP 38

#define DBL_TRUE_MIN 4.94065645841246544177e-324
#define DBL_MIN 2.22507385850720138309e-308
#define DBL_MAX 1.79769313486231570815e+308
#define DBL_EPSILON 2.22044604925031308085e-16

#define DBL_MANT_DIG 53
#define DBL_MIN_EXP (-1021)
#define DBL_MAX_EXP 1024
#define DBL_HAS_SUBNORM 1

#define DBL_DIG 15
#define DBL_DECIMAL_DIG 17
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_10_EXP 308

#define LDBL_HAS_SUBNORM 1
#define LDBL_DECIMAL_DIG DBL_DECIMAL_DIG
#define LDBL_MANT_DIG    DBL_MANT_DIG            
#define LDBL_MAX_EXP     DBL_MAX_EXP             

#undef  MAXFLOAT
#define MAXFLOAT        3.40282346638528859812e+38F
#define HUGE            3.40282346638528859812e+38F

#define M_DEG2RAD		0.017453292519943295	/* pi/180 */
#define M_E             2.7182818284590452354   /* e */
#define M_LOG2E         1.4426950408889634074   /* log_2 e */
#define M_LOG10E        0.43429448190325182765  /* log_10 e */
#define M_LN2           0.69314718055994530942  /* log_e 2 */
#define M_LN10          2.30258509299404568402  /* log_e 10 */
#define M_PI            3.14159265358979323846  /* pi */
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#define M_PI_4          0.78539816339744830962  /* pi/4 */
#define M_1_PI          0.31830988618379067154  /* 1/pi */
#define M_2_PI          0.63661977236758134308  /* 2/pi */
#define M_RAD2DEG		57.29577951308232		/* 180/pi */
#define M_2_SQRTPI      1.12837916709551257390  /* 2/sqrt(pi) */
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */

// extern int signgam;

// double      j0(double);
// double      j1(double);
// double      jn(int, double);

// double      y0(double);
// double      y1(double);
// double      yn(int, double);

#ifdef __cplusplus
}
#endif

#endif
