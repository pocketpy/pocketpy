#include "pocketpy/common/dmath.h"
#include "pocketpy/common/algorithm.h"
#include <stdint.h>

// hardware sqrt, see `dmath_sqrt`
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <emmintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#endif

union Float64Bits {
    double f;
    uint64_t i;
};

// IEEE 754 requires sqrt to be correctly rounded, so the hardware instruction
// returns the same bits on every platform (and matches CPython).
// libm is never used: every hardware branch below is guaranteed to emit the instruction,
// and other targets use a software sqrt which is also correctly rounded (same bits, but slow).
double dmath_sqrt(double x) {
    if(x < 0) return DMATH_NAN;
// PK_DMATH_SOFT_SQRT exercises the fallback on hardware-sqrt hosts in tests.
#if !defined(PK_DMATH_SOFT_SQRT) && \
    (defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2))
    // x86 with sse2: sqrtsd
    __m128d v = _mm_set_sd(x);
    return _mm_cvtsd_f64(_mm_sqrt_sd(v, v));
#elif !defined(PK_DMATH_SOFT_SQRT) && (defined(__aarch64__) || defined(_M_ARM64))
    // aarch64: fsqrt
    return vget_lane_f64(vsqrt_f64(vdup_n_f64(x)), 0);
#elif !defined(PK_DMATH_SOFT_SQRT) && defined(__arm__) && defined(__ARM_FP) && (__ARM_FP & 8)
    // arm32 with a double precision vfp: vsqrt
    // (`__builtin_sqrt` is not used because it may call libm to set errno)
    register double d0 __asm__("d0") = x;
    __asm__("vsqrt.f64 d0, d0" : "=w"(d0) : "w"(d0));
    return d0;
#else
    // software fallback (e.g. x86 without sse2, wasm):
    // digit-by-digit sqrt in integer arithmetic (as in fdlibm's e_sqrt.c)
    union Float64Bits u = { .f = x };
    int e = (int)(u.i >> 52) & 0x7ff;
    uint64_t m = u.i & (-1ULL >> 12);
    if(e == 0x7ff) return x + x;  // inf or nan
    if(e == 0) {
        if(m == 0) return x;  // +-0
        for(e = 1; m >> 52 == 0; e--) m <<= 1;  // subnormal
    } else {
        m |= 1ULL << 52;
    }
    // x = (m / 2^52) * 2^(e - 1023), make the exponent even so it can be halved
    if((e & 1) == 0) {
        m <<= 1;
        e--;
    }
    // q = floor(sqrt(m / 2^52) * 2^53), one bit per iteration:
    // 53 bits of the result and 1 more bit for rounding
    uint64_t q = 0, s = 0;
    m <<= 1;
    for(uint64_t r = 1ULL << 53; r != 0; r >>= 1) {
        uint64_t t = s + r;
        if(t <= m) {
            s = t + r;
            m -= t;
            q += r;
        }
        m <<= 1;
    }
    // sqrt of a double is never exactly halfway between two doubles, so round half up is enough
    q = (q + 1) >> 1;
    // the leading bit of q carries into the exponent
    u.i = ((uint64_t)((e + 1023) / 2 - 1) << 52) + q;
    return u.f;
#endif
}

// sin/cos/tan/sincos live in dmath_openlibm.c.
// dmath_asin / dmath_acos / dmath_atan / dmath_atan2 live in dmath_zig.c

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
	if(y == 0) return DMATH_NAN;
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

// https://github.com/kraj/musl/blob/kraj/master/src/math/fabs.c
double dmath_fabs(double x) {
	union Float64Bits u = { .f = x };
	u.i &= -1ULL/2;
	return u.f;
}

// ceil/floor/trunc live in dmath_openlibm.c.

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
    if(dmath_isnan(x)) return y;
    if(dmath_isnan(y)) return x;
    if(x == y) return (pk_dmath_bits(x) >> 63) ? x : y;
    return (x < y) ? x : y;
}

double dmath_fmax(double x, double y) {
    if(dmath_isnan(x)) return y;
    if(dmath_isnan(y)) return x;
    if(x == y) return (pk_dmath_bits(x) >> 63) ? y : x;
    return (x > y) ? x : y;
}
