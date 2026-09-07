/* Deterministic conversion between `double` and its decimal text form.
 *
 * The bulk of this file is vendored verbatim from Wuffs so that it stays cheap
 * to diff against upstream when picking up fixes:
 *
 *   https://github.com/google/wuffs
 *     internal/cgen/base/floatconv-submodule-data.c
 *     internal/cgen/base/floatconv-submodule-code.c
 *
 * Copyright 2020 The Wuffs Authors.
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 * Parsing is Eisel-Lemire with an exact high-precision-decimal fallback;
 * rendering runs the same decimal machinery backwards. Both are correctly
 * rounded, locale independent and (with the one exception guarded below) use
 * integer arithmetic only, which is what makes them deterministic.
 *
 * Deviations from upstream are tagged `[pocketpy]`:
 *   1. the f16/f32 entry points are dropped, as is the wuffs_base__ machinery
 *      they need; what little remains is reimplemented in the shim below;
 *   2. every entry point is `static`, since pocketpy exposes its own API at
 *      the bottom of this file;
 *   3. the shortest-round-trip renderer switches to exponent notation on
 *      CPython's threshold rather than C's "%g" one;
 *   4. the sole floating-point fast path is compiled out on targets with
 *      excess intermediate precision.
 *
 * Do not run clang-format over the vendored region; `scripts/format.py` skips
 * this file on purpose.
 */

#include "pocketpy/common/floatconv.h"

#include <float.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ---------------- [pocketpy] wuffs_base__ shim ----------------
 *
 * Just enough of Wuffs' base module for the vendored code to compile. These
 * are copied from internal/cgen/base/fundamental-public.h and
 * internal/cgen/base/strconv-public.h.
 */

#define WUFFS_BASE__MAYBE_STATIC static

typedef struct wuffs_base__slice_u8__struct {
  uint8_t* ptr;
  size_t len;
} wuffs_base__slice_u8;

typedef struct wuffs_base__status__struct {
  const char* repr;
} wuffs_base__status;

typedef struct wuffs_base__result_f64__struct {
  wuffs_base__status status;
  double value;
} wuffs_base__result_f64;

static const char wuffs_base__error__bad_argument[] = "#base: bad argument";
static const char wuffs_base__error__bad_receiver[] = "#base: bad receiver";

static inline wuffs_base__status  //
wuffs_base__make_status(const char* repr) {
  wuffs_base__status z;
  z.repr = repr;
  return z;
}

static inline int32_t  //
wuffs_base__i32__max(int32_t x, int32_t y) {
  return x > y ? x : y;
}

static inline uint32_t  //
wuffs_base__u32__min(uint32_t x, uint32_t y) {
  return x < y ? x : y;
}

#if (defined(__GNUC__) || defined(__clang__)) && (__SIZEOF_LONG__ == 8)

static inline uint32_t  //
wuffs_base__count_leading_zeroes_u64(uint64_t u) {
  return u ? ((uint32_t)(__builtin_clzl(u))) : 64u;
}

#else

static inline uint32_t  //
wuffs_base__count_leading_zeroes_u64(uint64_t u) {
  if (u == 0) {
    return 64;
  }

  uint32_t n = 0;
  if ((u >> 32) == 0) {
    n |= 32;
    u <<= 32;
  }
  if ((u >> 48) == 0) {
    n |= 16;
    u <<= 16;
  }
  if ((u >> 56) == 0) {
    n |= 8;
    u <<= 8;
  }
  if ((u >> 60) == 0) {
    n |= 4;
    u <<= 4;
  }
  if ((u >> 62) == 0) {
    n |= 2;
    u <<= 2;
  }
  if ((u >> 63) == 0) {
    n |= 1;
    u <<= 1;
  }
  return n;
}

#endif

typedef struct wuffs_base__multiply_u64__output__struct {
  uint64_t lo;
  uint64_t hi;
} wuffs_base__multiply_u64__output;

static inline wuffs_base__multiply_u64__output  //
wuffs_base__multiply_u64(uint64_t x, uint64_t y) {
#if defined(__SIZEOF_INT128__)
  __uint128_t z = ((__uint128_t)x) * ((__uint128_t)y);
  wuffs_base__multiply_u64__output o;
  o.lo = ((uint64_t)(z));
  o.hi = ((uint64_t)(z >> 64));
  return o;
#else
  uint64_t x0 = x & 0xFFFFFFFF;
  uint64_t x1 = x >> 32;
  uint64_t y0 = y & 0xFFFFFFFF;
  uint64_t y1 = y >> 32;
  uint64_t w0 = x0 * y0;
  uint64_t t = (x1 * y0) + (w0 >> 32);
  uint64_t w1 = t & 0xFFFFFFFF;
  uint64_t w2 = t >> 32;
  w1 += x0 * y1;
  wuffs_base__multiply_u64__output o;
  o.lo = x * y;
  o.hi = (x1 * y1) + w2 + (w1 >> 32);
  return o;
#endif
}

static inline void  //
wuffs_base__poke_u24le__no_bounds_check(uint8_t* p, uint32_t x) {
  p[0] = (uint8_t)(x >> 0);
  p[1] = (uint8_t)(x >> 8);
  p[2] = (uint8_t)(x >> 16);
}

static inline void  //
wuffs_base__poke_u32le__no_bounds_check(uint8_t* p, uint32_t x) {
  p[0] = (uint8_t)(x >> 0);
  p[1] = (uint8_t)(x >> 8);
  p[2] = (uint8_t)(x >> 16);
  p[3] = (uint8_t)(x >> 24);
}

static inline uint64_t  //
wuffs_base__ieee_754_bit_representation__from_f64_to_u64(double f) {
  uint64_t u = 0;
  if (sizeof(uint64_t) == sizeof(double)) {
    memcpy(&u, &f, sizeof(uint64_t));
  }
  return u;
}

static inline double  //
wuffs_base__ieee_754_bit_representation__from_u64_to_f64(uint64_t u) {
  double f = 0;
  if (sizeof(uint64_t) == sizeof(double)) {
    memcpy(&f, &u, sizeof(uint64_t));
  }
  return f;
}

// Options for wuffs_base__parse_number_f64.
#define WUFFS_BASE__PARSE_NUMBER_XXX__DEFAULT_OPTIONS ((uint32_t)0x00000000)
#define WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_MULTIPLE_LEADING_ZEROES \
  ((uint32_t)0x00000001)
#define WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES ((uint32_t)0x00000002)
#define WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA \
  ((uint32_t)0x00000010)
#define WUFFS_BASE__PARSE_NUMBER_FXX__REJECT_INF_AND_NAN ((uint32_t)0x00000020)

// Options for wuffs_base__render_number_f64.
#define WUFFS_BASE__RENDER_NUMBER_XXX__DEFAULT_OPTIONS ((uint32_t)0x00000000)
#define WUFFS_BASE__RENDER_NUMBER_XXX__ALIGN_RIGHT ((uint32_t)0x00000100)
#define WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN ((uint32_t)0x00000200)
#define WUFFS_BASE__RENDER_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA \
  ((uint32_t)0x00001000)
#define WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_ABSENT ((uint32_t)0x00002000)
#define WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_PRESENT ((uint32_t)0x00004000)
#define WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION \
  ((uint32_t)0x00008000)

/* [pocketpy] Deviation 3.
 *
 * Wuffs' "%g" notation follows C and switches to an exponent once the decimal
 * point moves past 6 digits, so 1e6 would render as "1e+06". CPython's repr()
 * instead switches once `decimal_point > 16`, which is what
 * `wuffs_private_impl__high_prec_dec__render_*` calls an `e_threshold` of 16.
 * See `format_float_short` in CPython's Python/pystrtod.c.
 */
#define PK_FLOATCONV_REPR_E_THRESHOLD 16

/* [pocketpy] Deviation 4.
 *
 * Everything below is integer arithmetic except for one `d *= power_of_10`
 * fast path in wuffs_base__parse_number_f64. That multiply is exact and
 * correctly rounded on an IEEE-754 target, but on a target that evaluates
 * doubles in a wider format -- 32-bit x86 using the x87 stack is the one that
 * still matters -- it rounds twice and can land one ulp away from what the
 * integer path computes. Compile it out there and let Eisel-Lemire handle
 * those inputs instead; the answer is the same, just a few ns slower.
 */
#if !defined(FLT_EVAL_METHOD) || (FLT_EVAL_METHOD == 0)
#define PK_FLOATCONV_HAS_EXACT_DOUBLE_ARITHMETIC 1
#else
#define PK_FLOATCONV_HAS_EXACT_DOUBLE_ARITHMETIC 0
#endif

/* ---------------- end of the [pocketpy] shim ---------------- */

// ---------------- IEEE 754 Floating Point

// The etc__hpd_left_shift and etc__powers_of_5 tables were printed by
// script/print-hpd-left-shift.go. That script has an optional -comments flag,
// whose output is not copied here, which prints further detail.
//
// These tables are used in
// wuffs_private_impl__high_prec_dec__lshift_num_new_digits.

// wuffs_private_impl__hpd_left_shift[i] encodes the number of new digits
// created after multiplying a positive integer by (1 << i): the additional
// length in the decimal representation. For example, shifting "234" by 3
// (equivalent to multiplying by 8) will produce "1872". Going from a 3-length
// string to a 4-length string means that 1 new digit was added (and existing
// digits may have changed).
//
// Shifting by i can add either N or N-1 new digits, depending on whether the
// original positive integer compares >= or < to the i'th power of 5 (as 10
// equals 2 * 5). Comparison is lexicographic, not numerical.
//
// For example, shifting by 4 (i.e. multiplying by 16) can add 1 or 2 new
// digits, depending on a lexicographic comparison to (5 ** 4), i.e. "625":
//  - ("1"      << 4) is "16",       which adds 1 new digit.
//  - ("5678"   << 4) is "90848",    which adds 1 new digit.
//  - ("624"    << 4) is "9984",     which adds 1 new digit.
//  - ("62498"  << 4) is "999968",   which adds 1 new digit.
//  - ("625"    << 4) is "10000",    which adds 2 new digits.
//  - ("625001" << 4) is "10000016", which adds 2 new digits.
//  - ("7008"   << 4) is "112128",   which adds 2 new digits.
//  - ("99"     << 4) is "1584",     which adds 2 new digits.
//
// Thus, when i is 4, N is 2 and (5 ** i) is "625". This etc__hpd_left_shift
// array encodes this as:
//  - etc__hpd_left_shift[4] is 0x1006 = (2 << 11) | 0x0006.
//  - etc__hpd_left_shift[5] is 0x1009 = (? << 11) | 0x0009.
// where the ? isn't relevant for i == 4.
//
// The high 5 bits of etc__hpd_left_shift[i] is N, the higher of the two
// possible number of new digits. The low 11 bits are an offset into the
// etc__powers_of_5 array (of length 0x051C, so offsets fit in 11 bits). When i
// is 4, its offset and the next one is 6 and 9, and etc__powers_of_5[6 .. 9]
// is the string "\x06\x02\x05", so the relevant power of 5 is "625".
//
// Thanks to Ken Thompson for the original idea.
static const uint16_t wuffs_private_impl__hpd_left_shift[65] = {
    0x0000, 0x0800, 0x0801, 0x0803, 0x1006, 0x1009, 0x100D, 0x1812, 0x1817,
    0x181D, 0x2024, 0x202B, 0x2033, 0x203C, 0x2846, 0x2850, 0x285B, 0x3067,
    0x3073, 0x3080, 0x388E, 0x389C, 0x38AB, 0x38BB, 0x40CC, 0x40DD, 0x40EF,
    0x4902, 0x4915, 0x4929, 0x513E, 0x5153, 0x5169, 0x5180, 0x5998, 0x59B0,
    0x59C9, 0x61E3, 0x61FD, 0x6218, 0x6A34, 0x6A50, 0x6A6D, 0x6A8B, 0x72AA,
    0x72C9, 0x72E9, 0x7B0A, 0x7B2B, 0x7B4D, 0x8370, 0x8393, 0x83B7, 0x83DC,
    0x8C02, 0x8C28, 0x8C4F, 0x9477, 0x949F, 0x94C8, 0x9CF2, 0x051C, 0x051C,
    0x051C, 0x051C,
};

// wuffs_private_impl__powers_of_5 contains the powers of 5, concatenated
// together: "5", "25", "125", "625", "3125", etc.
static const uint8_t wuffs_private_impl__powers_of_5[0x051C] = {
    5, 2, 5, 1, 2, 5, 6, 2, 5, 3, 1, 2, 5, 1, 5, 6, 2, 5, 7, 8, 1, 2, 5, 3, 9,
    0, 6, 2, 5, 1, 9, 5, 3, 1, 2, 5, 9, 7, 6, 5, 6, 2, 5, 4, 8, 8, 2, 8, 1, 2,
    5, 2, 4, 4, 1, 4, 0, 6, 2, 5, 1, 2, 2, 0, 7, 0, 3, 1, 2, 5, 6, 1, 0, 3, 5,
    1, 5, 6, 2, 5, 3, 0, 5, 1, 7, 5, 7, 8, 1, 2, 5, 1, 5, 2, 5, 8, 7, 8, 9, 0,
    6, 2, 5, 7, 6, 2, 9, 3, 9, 4, 5, 3, 1, 2, 5, 3, 8, 1, 4, 6, 9, 7, 2, 6, 5,
    6, 2, 5, 1, 9, 0, 7, 3, 4, 8, 6, 3, 2, 8, 1, 2, 5, 9, 5, 3, 6, 7, 4, 3, 1,
    6, 4, 0, 6, 2, 5, 4, 7, 6, 8, 3, 7, 1, 5, 8, 2, 0, 3, 1, 2, 5, 2, 3, 8, 4,
    1, 8, 5, 7, 9, 1, 0, 1, 5, 6, 2, 5, 1, 1, 9, 2, 0, 9, 2, 8, 9, 5, 5, 0, 7,
    8, 1, 2, 5, 5, 9, 6, 0, 4, 6, 4, 4, 7, 7, 5, 3, 9, 0, 6, 2, 5, 2, 9, 8, 0,
    2, 3, 2, 2, 3, 8, 7, 6, 9, 5, 3, 1, 2, 5, 1, 4, 9, 0, 1, 1, 6, 1, 1, 9, 3,
    8, 4, 7, 6, 5, 6, 2, 5, 7, 4, 5, 0, 5, 8, 0, 5, 9, 6, 9, 2, 3, 8, 2, 8, 1,
    2, 5, 3, 7, 2, 5, 2, 9, 0, 2, 9, 8, 4, 6, 1, 9, 1, 4, 0, 6, 2, 5, 1, 8, 6,
    2, 6, 4, 5, 1, 4, 9, 2, 3, 0, 9, 5, 7, 0, 3, 1, 2, 5, 9, 3, 1, 3, 2, 2, 5,
    7, 4, 6, 1, 5, 4, 7, 8, 5, 1, 5, 6, 2, 5, 4, 6, 5, 6, 6, 1, 2, 8, 7, 3, 0,
    7, 7, 3, 9, 2, 5, 7, 8, 1, 2, 5, 2, 3, 2, 8, 3, 0, 6, 4, 3, 6, 5, 3, 8, 6,
    9, 6, 2, 8, 9, 0, 6, 2, 5, 1, 1, 6, 4, 1, 5, 3, 2, 1, 8, 2, 6, 9, 3, 4, 8,
    1, 4, 4, 5, 3, 1, 2, 5, 5, 8, 2, 0, 7, 6, 6, 0, 9, 1, 3, 4, 6, 7, 4, 0, 7,
    2, 2, 6, 5, 6, 2, 5, 2, 9, 1, 0, 3, 8, 3, 0, 4, 5, 6, 7, 3, 3, 7, 0, 3, 6,
    1, 3, 2, 8, 1, 2, 5, 1, 4, 5, 5, 1, 9, 1, 5, 2, 2, 8, 3, 6, 6, 8, 5, 1, 8,
    0, 6, 6, 4, 0, 6, 2, 5, 7, 2, 7, 5, 9, 5, 7, 6, 1, 4, 1, 8, 3, 4, 2, 5, 9,
    0, 3, 3, 2, 0, 3, 1, 2, 5, 3, 6, 3, 7, 9, 7, 8, 8, 0, 7, 0, 9, 1, 7, 1, 2,
    9, 5, 1, 6, 6, 0, 1, 5, 6, 2, 5, 1, 8, 1, 8, 9, 8, 9, 4, 0, 3, 5, 4, 5, 8,
    5, 6, 4, 7, 5, 8, 3, 0, 0, 7, 8, 1, 2, 5, 9, 0, 9, 4, 9, 4, 7, 0, 1, 7, 7,
    2, 9, 2, 8, 2, 3, 7, 9, 1, 5, 0, 3, 9, 0, 6, 2, 5, 4, 5, 4, 7, 4, 7, 3, 5,
    0, 8, 8, 6, 4, 6, 4, 1, 1, 8, 9, 5, 7, 5, 1, 9, 5, 3, 1, 2, 5, 2, 2, 7, 3,
    7, 3, 6, 7, 5, 4, 4, 3, 2, 3, 2, 0, 5, 9, 4, 7, 8, 7, 5, 9, 7, 6, 5, 6, 2,
    5, 1, 1, 3, 6, 8, 6, 8, 3, 7, 7, 2, 1, 6, 1, 6, 0, 2, 9, 7, 3, 9, 3, 7, 9,
    8, 8, 2, 8, 1, 2, 5, 5, 6, 8, 4, 3, 4, 1, 8, 8, 6, 0, 8, 0, 8, 0, 1, 4, 8,
    6, 9, 6, 8, 9, 9, 4, 1, 4, 0, 6, 2, 5, 2, 8, 4, 2, 1, 7, 0, 9, 4, 3, 0, 4,
    0, 4, 0, 0, 7, 4, 3, 4, 8, 4, 4, 9, 7, 0, 7, 0, 3, 1, 2, 5, 1, 4, 2, 1, 0,
    8, 5, 4, 7, 1, 5, 2, 0, 2, 0, 0, 3, 7, 1, 7, 4, 2, 2, 4, 8, 5, 3, 5, 1, 5,
    6, 2, 5, 7, 1, 0, 5, 4, 2, 7, 3, 5, 7, 6, 0, 1, 0, 0, 1, 8, 5, 8, 7, 1, 1,
    2, 4, 2, 6, 7, 5, 7, 8, 1, 2, 5, 3, 5, 5, 2, 7, 1, 3, 6, 7, 8, 8, 0, 0, 5,
    0, 0, 9, 2, 9, 3, 5, 5, 6, 2, 1, 3, 3, 7, 8, 9, 0, 6, 2, 5, 1, 7, 7, 6, 3,
    5, 6, 8, 3, 9, 4, 0, 0, 2, 5, 0, 4, 6, 4, 6, 7, 7, 8, 1, 0, 6, 6, 8, 9, 4,
    5, 3, 1, 2, 5, 8, 8, 8, 1, 7, 8, 4, 1, 9, 7, 0, 0, 1, 2, 5, 2, 3, 2, 3, 3,
    8, 9, 0, 5, 3, 3, 4, 4, 7, 2, 6, 5, 6, 2, 5, 4, 4, 4, 0, 8, 9, 2, 0, 9, 8,
    5, 0, 0, 6, 2, 6, 1, 6, 1, 6, 9, 4, 5, 2, 6, 6, 7, 2, 3, 6, 3, 2, 8, 1, 2,
    5, 2, 2, 2, 0, 4, 4, 6, 0, 4, 9, 2, 5, 0, 3, 1, 3, 0, 8, 0, 8, 4, 7, 2, 6,
    3, 3, 3, 6, 1, 8, 1, 6, 4, 0, 6, 2, 5, 1, 1, 1, 0, 2, 2, 3, 0, 2, 4, 6, 2,
    5, 1, 5, 6, 5, 4, 0, 4, 2, 3, 6, 3, 1, 6, 6, 8, 0, 9, 0, 8, 2, 0, 3, 1, 2,
    5, 5, 5, 5, 1, 1, 1, 5, 1, 2, 3, 1, 2, 5, 7, 8, 2, 7, 0, 2, 1, 1, 8, 1, 5,
    8, 3, 4, 0, 4, 5, 4, 1, 0, 1, 5, 6, 2, 5, 2, 7, 7, 5, 5, 5, 7, 5, 6, 1, 5,
    6, 2, 8, 9, 1, 3, 5, 1, 0, 5, 9, 0, 7, 9, 1, 7, 0, 2, 2, 7, 0, 5, 0, 7, 8,
    1, 2, 5, 1, 3, 8, 7, 7, 7, 8, 7, 8, 0, 7, 8, 1, 4, 4, 5, 6, 7, 5, 5, 2, 9,
    5, 3, 9, 5, 8, 5, 1, 1, 3, 5, 2, 5, 3, 9, 0, 6, 2, 5, 6, 9, 3, 8, 8, 9, 3,
    9, 0, 3, 9, 0, 7, 2, 2, 8, 3, 7, 7, 6, 4, 7, 6, 9, 7, 9, 2, 5, 5, 6, 7, 6,
    2, 6, 9, 5, 3, 1, 2, 5, 3, 4, 6, 9, 4, 4, 6, 9, 5, 1, 9, 5, 3, 6, 1, 4, 1,
    8, 8, 8, 2, 3, 8, 4, 8, 9, 6, 2, 7, 8, 3, 8, 1, 3, 4, 7, 6, 5, 6, 2, 5, 1,
    7, 3, 4, 7, 2, 3, 4, 7, 5, 9, 7, 6, 8, 0, 7, 0, 9, 4, 4, 1, 1, 9, 2, 4, 4,
    8, 1, 3, 9, 1, 9, 0, 6, 7, 3, 8, 2, 8, 1, 2, 5, 8, 6, 7, 3, 6, 1, 7, 3, 7,
    9, 8, 8, 4, 0, 3, 5, 4, 7, 2, 0, 5, 9, 6, 2, 2, 4, 0, 6, 9, 5, 9, 5, 3, 3,
    6, 9, 1, 4, 0, 6, 2, 5,
};

// --------

// wuffs_private_impl__powers_of_10 contains truncated approximations to the
// powers of 10, ranging from 1e-307 to 1e+288 inclusive, as 596 pairs of
// uint64_t values (a 128-bit mantissa).
//
// There's also an implicit third column (implied by a linear formula involving
// the base-10 exponent) that is the base-2 exponent, biased by a magic
// constant. That constant (1214 or 0x04BE) equals 1023 + 191. 1023 is the bias
// for IEEE 754 double-precision floating point. 191 is ((3 * 64) - 1) and
// wuffs_private_impl__parse_number_f64_eisel_lemire works with
// multiples-of-64-bit mantissas.
//
// For example, the third row holds the approximation to 1e-305:
//   0xE0B62E29_29ABA83C_331ACDAB_FE94DE87 * (2 ** (0x0049 - 0x04BE))
//
// Similarly, 1e+4 is approximated by:
//   0x9C400000_00000000_00000000_00000000 * (2 ** (0x044C - 0x04BE))
//
// Similarly, 1e+68 is approximated by:
//   0xED63A231_D4C4FB27_4CA7AAA8_63EE4BDD * (2 ** (0x0520 - 0x04BE))
//
// This table was generated by by script/print-mpb-powers-of-10.go
static const uint64_t wuffs_private_impl__powers_of_10[596][2] = {
    {0xA5D3B6D479F8E056, 0x8FD0C16206306BAB},  // 1e-307
    {0x8F48A4899877186C, 0xB3C4F1BA87BC8696},  // 1e-306
    {0x331ACDABFE94DE87, 0xE0B62E2929ABA83C},  // 1e-305
    {0x9FF0C08B7F1D0B14, 0x8C71DCD9BA0B4925},  // 1e-304
    {0x07ECF0AE5EE44DD9, 0xAF8E5410288E1B6F},  // 1e-303
    {0xC9E82CD9F69D6150, 0xDB71E91432B1A24A},  // 1e-302
    {0xBE311C083A225CD2, 0x892731AC9FAF056E},  // 1e-301
    {0x6DBD630A48AAF406, 0xAB70FE17C79AC6CA},  // 1e-300
    {0x092CBBCCDAD5B108, 0xD64D3D9DB981787D},  // 1e-299
    {0x25BBF56008C58EA5, 0x85F0468293F0EB4E},  // 1e-298
    {0xAF2AF2B80AF6F24E, 0xA76C582338ED2621},  // 1e-297
    {0x1AF5AF660DB4AEE1, 0xD1476E2C07286FAA},  // 1e-296
    {0x50D98D9FC890ED4D, 0x82CCA4DB847945CA},  // 1e-295
    {0xE50FF107BAB528A0, 0xA37FCE126597973C},  // 1e-294
    {0x1E53ED49A96272C8, 0xCC5FC196FEFD7D0C},  // 1e-293
    {0x25E8E89C13BB0F7A, 0xFF77B1FCBEBCDC4F},  // 1e-292
    {0x77B191618C54E9AC, 0x9FAACF3DF73609B1},  // 1e-291
    {0xD59DF5B9EF6A2417, 0xC795830D75038C1D},  // 1e-290
    {0x4B0573286B44AD1D, 0xF97AE3D0D2446F25},  // 1e-289
    {0x4EE367F9430AEC32, 0x9BECCE62836AC577},  // 1e-288
    {0x229C41F793CDA73F, 0xC2E801FB244576D5},  // 1e-287
    {0x6B43527578C1110F, 0xF3A20279ED56D48A},  // 1e-286
    {0x830A13896B78AAA9, 0x9845418C345644D6},  // 1e-285
    {0x23CC986BC656D553, 0xBE5691EF416BD60C},  // 1e-284
    {0x2CBFBE86B7EC8AA8, 0xEDEC366B11C6CB8F},  // 1e-283
    {0x7BF7D71432F3D6A9, 0x94B3A202EB1C3F39},  // 1e-282
    {0xDAF5CCD93FB0CC53, 0xB9E08A83A5E34F07},  // 1e-281
    {0xD1B3400F8F9CFF68, 0xE858AD248F5C22C9},  // 1e-280
    {0x23100809B9C21FA1, 0x91376C36D99995BE},  // 1e-279
    {0xABD40A0C2832A78A, 0xB58547448FFFFB2D},  // 1e-278
    {0x16C90C8F323F516C, 0xE2E69915B3FFF9F9},  // 1e-277
    {0xAE3DA7D97F6792E3, 0x8DD01FAD907FFC3B},  // 1e-276
    {0x99CD11CFDF41779C, 0xB1442798F49FFB4A},  // 1e-275
    {0x40405643D711D583, 0xDD95317F31C7FA1D},  // 1e-274
    {0x482835EA666B2572, 0x8A7D3EEF7F1CFC52},  // 1e-273
    {0xDA3243650005EECF, 0xAD1C8EAB5EE43B66},  // 1e-272
    {0x90BED43E40076A82, 0xD863B256369D4A40},  // 1e-271
    {0x5A7744A6E804A291, 0x873E4F75E2224E68},  // 1e-270
    {0x711515D0A205CB36, 0xA90DE3535AAAE202},  // 1e-269
    {0x0D5A5B44CA873E03, 0xD3515C2831559A83},  // 1e-268
    {0xE858790AFE9486C2, 0x8412D9991ED58091},  // 1e-267
    {0x626E974DBE39A872, 0xA5178FFF668AE0B6},  // 1e-266
    {0xFB0A3D212DC8128F, 0xCE5D73FF402D98E3},  // 1e-265
    {0x7CE66634BC9D0B99, 0x80FA687F881C7F8E},  // 1e-264
    {0x1C1FFFC1EBC44E80, 0xA139029F6A239F72},  // 1e-263
    {0xA327FFB266B56220, 0xC987434744AC874E},  // 1e-262
    {0x4BF1FF9F0062BAA8, 0xFBE9141915D7A922},  // 1e-261
    {0x6F773FC3603DB4A9, 0x9D71AC8FADA6C9B5},  // 1e-260
    {0xCB550FB4384D21D3, 0xC4CE17B399107C22},  // 1e-259
    {0x7E2A53A146606A48, 0xF6019DA07F549B2B},  // 1e-258
    {0x2EDA7444CBFC426D, 0x99C102844F94E0FB},  // 1e-257
    {0xFA911155FEFB5308, 0xC0314325637A1939},  // 1e-256
    {0x793555AB7EBA27CA, 0xF03D93EEBC589F88},  // 1e-255
    {0x4BC1558B2F3458DE, 0x96267C7535B763B5},  // 1e-254
    {0x9EB1AAEDFB016F16, 0xBBB01B9283253CA2},  // 1e-253
    {0x465E15A979C1CADC, 0xEA9C227723EE8BCB},  // 1e-252
    {0x0BFACD89EC191EC9, 0x92A1958A7675175F},  // 1e-251
    {0xCEF980EC671F667B, 0xB749FAED14125D36},  // 1e-250
    {0x82B7E12780E7401A, 0xE51C79A85916F484},  // 1e-249
    {0xD1B2ECB8B0908810, 0x8F31CC0937AE58D2},  // 1e-248
    {0x861FA7E6DCB4AA15, 0xB2FE3F0B8599EF07},  // 1e-247
    {0x67A791E093E1D49A, 0xDFBDCECE67006AC9},  // 1e-246
    {0xE0C8BB2C5C6D24E0, 0x8BD6A141006042BD},  // 1e-245
    {0x58FAE9F773886E18, 0xAECC49914078536D},  // 1e-244
    {0xAF39A475506A899E, 0xDA7F5BF590966848},  // 1e-243
    {0x6D8406C952429603, 0x888F99797A5E012D},  // 1e-242
    {0xC8E5087BA6D33B83, 0xAAB37FD7D8F58178},  // 1e-241
    {0xFB1E4A9A90880A64, 0xD5605FCDCF32E1D6},  // 1e-240
    {0x5CF2EEA09A55067F, 0x855C3BE0A17FCD26},  // 1e-239
    {0xF42FAA48C0EA481E, 0xA6B34AD8C9DFC06F},  // 1e-238
    {0xF13B94DAF124DA26, 0xD0601D8EFC57B08B},  // 1e-237
    {0x76C53D08D6B70858, 0x823C12795DB6CE57},  // 1e-236
    {0x54768C4B0C64CA6E, 0xA2CB1717B52481ED},  // 1e-235
    {0xA9942F5DCF7DFD09, 0xCB7DDCDDA26DA268},  // 1e-234
    {0xD3F93B35435D7C4C, 0xFE5D54150B090B02},  // 1e-233
    {0xC47BC5014A1A6DAF, 0x9EFA548D26E5A6E1},  // 1e-232
    {0x359AB6419CA1091B, 0xC6B8E9B0709F109A},  // 1e-231
    {0xC30163D203C94B62, 0xF867241C8CC6D4C0},  // 1e-230
    {0x79E0DE63425DCF1D, 0x9B407691D7FC44F8},  // 1e-229
    {0x985915FC12F542E4, 0xC21094364DFB5636},  // 1e-228
    {0x3E6F5B7B17B2939D, 0xF294B943E17A2BC4},  // 1e-227
    {0xA705992CEECF9C42, 0x979CF3CA6CEC5B5A},  // 1e-226
    {0x50C6FF782A838353, 0xBD8430BD08277231},  // 1e-225
    {0xA4F8BF5635246428, 0xECE53CEC4A314EBD},  // 1e-224
    {0x871B7795E136BE99, 0x940F4613AE5ED136},  // 1e-223
    {0x28E2557B59846E3F, 0xB913179899F68584},  // 1e-222
    {0x331AEADA2FE589CF, 0xE757DD7EC07426E5},  // 1e-221
    {0x3FF0D2C85DEF7621, 0x9096EA6F3848984F},  // 1e-220
    {0x0FED077A756B53A9, 0xB4BCA50B065ABE63},  // 1e-219
    {0xD3E8495912C62894, 0xE1EBCE4DC7F16DFB},  // 1e-218
    {0x64712DD7ABBBD95C, 0x8D3360F09CF6E4BD},  // 1e-217
    {0xBD8D794D96AACFB3, 0xB080392CC4349DEC},  // 1e-216
    {0xECF0D7A0FC5583A0, 0xDCA04777F541C567},  // 1e-215
    {0xF41686C49DB57244, 0x89E42CAAF9491B60},  // 1e-214
    {0x311C2875C522CED5, 0xAC5D37D5B79B6239},  // 1e-213
    {0x7D633293366B828B, 0xD77485CB25823AC7},  // 1e-212
    {0xAE5DFF9C02033197, 0x86A8D39EF77164BC},  // 1e-211
    {0xD9F57F830283FDFC, 0xA8530886B54DBDEB},  // 1e-210
    {0xD072DF63C324FD7B, 0xD267CAA862A12D66},  // 1e-209
    {0x4247CB9E59F71E6D, 0x8380DEA93DA4BC60},  // 1e-208
    {0x52D9BE85F074E608, 0xA46116538D0DEB78},  // 1e-207
    {0x67902E276C921F8B, 0xCD795BE870516656},  // 1e-206
    {0x00BA1CD8A3DB53B6, 0x806BD9714632DFF6},  // 1e-205
    {0x80E8A40ECCD228A4, 0xA086CFCD97BF97F3},  // 1e-204
    {0x6122CD128006B2CD, 0xC8A883C0FDAF7DF0},  // 1e-203
    {0x796B805720085F81, 0xFAD2A4B13D1B5D6C},  // 1e-202
    {0xCBE3303674053BB0, 0x9CC3A6EEC6311A63},  // 1e-201
    {0xBEDBFC4411068A9C, 0xC3F490AA77BD60FC},  // 1e-200
    {0xEE92FB5515482D44, 0xF4F1B4D515ACB93B},  // 1e-199
    {0x751BDD152D4D1C4A, 0x991711052D8BF3C5},  // 1e-198
    {0xD262D45A78A0635D, 0xBF5CD54678EEF0B6},  // 1e-197
    {0x86FB897116C87C34, 0xEF340A98172AACE4},  // 1e-196
    {0xD45D35E6AE3D4DA0, 0x9580869F0E7AAC0E},  // 1e-195
    {0x8974836059CCA109, 0xBAE0A846D2195712},  // 1e-194
    {0x2BD1A438703FC94B, 0xE998D258869FACD7},  // 1e-193
    {0x7B6306A34627DDCF, 0x91FF83775423CC06},  // 1e-192
    {0x1A3BC84C17B1D542, 0xB67F6455292CBF08},  // 1e-191
    {0x20CABA5F1D9E4A93, 0xE41F3D6A7377EECA},  // 1e-190
    {0x547EB47B7282EE9C, 0x8E938662882AF53E},  // 1e-189
    {0xE99E619A4F23AA43, 0xB23867FB2A35B28D},  // 1e-188
    {0x6405FA00E2EC94D4, 0xDEC681F9F4C31F31},  // 1e-187
    {0xDE83BC408DD3DD04, 0x8B3C113C38F9F37E},  // 1e-186
    {0x9624AB50B148D445, 0xAE0B158B4738705E},  // 1e-185
    {0x3BADD624DD9B0957, 0xD98DDAEE19068C76},  // 1e-184
    {0xE54CA5D70A80E5D6, 0x87F8A8D4CFA417C9},  // 1e-183
    {0x5E9FCF4CCD211F4C, 0xA9F6D30A038D1DBC},  // 1e-182
    {0x7647C3200069671F, 0xD47487CC8470652B},  // 1e-181
    {0x29ECD9F40041E073, 0x84C8D4DFD2C63F3B},  // 1e-180
    {0xF468107100525890, 0xA5FB0A17C777CF09},  // 1e-179
    {0x7182148D4066EEB4, 0xCF79CC9DB955C2CC},  // 1e-178
    {0xC6F14CD848405530, 0x81AC1FE293D599BF},  // 1e-177
    {0xB8ADA00E5A506A7C, 0xA21727DB38CB002F},  // 1e-176
    {0xA6D90811F0E4851C, 0xCA9CF1D206FDC03B},  // 1e-175
    {0x908F4A166D1DA663, 0xFD442E4688BD304A},  // 1e-174
    {0x9A598E4E043287FE, 0x9E4A9CEC15763E2E},  // 1e-173
    {0x40EFF1E1853F29FD, 0xC5DD44271AD3CDBA},  // 1e-172
    {0xD12BEE59E68EF47C, 0xF7549530E188C128},  // 1e-171
    {0x82BB74F8301958CE, 0x9A94DD3E8CF578B9},  // 1e-170
    {0xE36A52363C1FAF01, 0xC13A148E3032D6E7},  // 1e-169
    {0xDC44E6C3CB279AC1, 0xF18899B1BC3F8CA1},  // 1e-168
    {0x29AB103A5EF8C0B9, 0x96F5600F15A7B7E5},  // 1e-167
    {0x7415D448F6B6F0E7, 0xBCB2B812DB11A5DE},  // 1e-166
    {0x111B495B3464AD21, 0xEBDF661791D60F56},  // 1e-165
    {0xCAB10DD900BEEC34, 0x936B9FCEBB25C995},  // 1e-164
    {0x3D5D514F40EEA742, 0xB84687C269EF3BFB},  // 1e-163
    {0x0CB4A5A3112A5112, 0xE65829B3046B0AFA},  // 1e-162
    {0x47F0E785EABA72AB, 0x8FF71A0FE2C2E6DC},  // 1e-161
    {0x59ED216765690F56, 0xB3F4E093DB73A093},  // 1e-160
    {0x306869C13EC3532C, 0xE0F218B8D25088B8},  // 1e-159
    {0x1E414218C73A13FB, 0x8C974F7383725573},  // 1e-158
    {0xE5D1929EF90898FA, 0xAFBD2350644EEACF},  // 1e-157
    {0xDF45F746B74ABF39, 0xDBAC6C247D62A583},  // 1e-156
    {0x6B8BBA8C328EB783, 0x894BC396CE5DA772},  // 1e-155
    {0x066EA92F3F326564, 0xAB9EB47C81F5114F},  // 1e-154
    {0xC80A537B0EFEFEBD, 0xD686619BA27255A2},  // 1e-153
    {0xBD06742CE95F5F36, 0x8613FD0145877585},  // 1e-152
    {0x2C48113823B73704, 0xA798FC4196E952E7},  // 1e-151
    {0xF75A15862CA504C5, 0xD17F3B51FCA3A7A0},  // 1e-150
    {0x9A984D73DBE722FB, 0x82EF85133DE648C4},  // 1e-149
    {0xC13E60D0D2E0EBBA, 0xA3AB66580D5FDAF5},  // 1e-148
    {0x318DF905079926A8, 0xCC963FEE10B7D1B3},  // 1e-147
    {0xFDF17746497F7052, 0xFFBBCFE994E5C61F},  // 1e-146
    {0xFEB6EA8BEDEFA633, 0x9FD561F1FD0F9BD3},  // 1e-145
    {0xFE64A52EE96B8FC0, 0xC7CABA6E7C5382C8},  // 1e-144
    {0x3DFDCE7AA3C673B0, 0xF9BD690A1B68637B},  // 1e-143
    {0x06BEA10CA65C084E, 0x9C1661A651213E2D},  // 1e-142
    {0x486E494FCFF30A62, 0xC31BFA0FE5698DB8},  // 1e-141
    {0x5A89DBA3C3EFCCFA, 0xF3E2F893DEC3F126},  // 1e-140
    {0xF89629465A75E01C, 0x986DDB5C6B3A76B7},  // 1e-139
    {0xF6BBB397F1135823, 0xBE89523386091465},  // 1e-138
    {0x746AA07DED582E2C, 0xEE2BA6C0678B597F},  // 1e-137
    {0xA8C2A44EB4571CDC, 0x94DB483840B717EF},  // 1e-136
    {0x92F34D62616CE413, 0xBA121A4650E4DDEB},  // 1e-135
    {0x77B020BAF9C81D17, 0xE896A0D7E51E1566},  // 1e-134
    {0x0ACE1474DC1D122E, 0x915E2486EF32CD60},  // 1e-133
    {0x0D819992132456BA, 0xB5B5ADA8AAFF80B8},  // 1e-132
    {0x10E1FFF697ED6C69, 0xE3231912D5BF60E6},  // 1e-131
    {0xCA8D3FFA1EF463C1, 0x8DF5EFABC5979C8F},  // 1e-130
    {0xBD308FF8A6B17CB2, 0xB1736B96B6FD83B3},  // 1e-129
    {0xAC7CB3F6D05DDBDE, 0xDDD0467C64BCE4A0},  // 1e-128
    {0x6BCDF07A423AA96B, 0x8AA22C0DBEF60EE4},  // 1e-127
    {0x86C16C98D2C953C6, 0xAD4AB7112EB3929D},  // 1e-126
    {0xE871C7BF077BA8B7, 0xD89D64D57A607744},  // 1e-125
    {0x11471CD764AD4972, 0x87625F056C7C4A8B},  // 1e-124
    {0xD598E40D3DD89BCF, 0xA93AF6C6C79B5D2D},  // 1e-123
    {0x4AFF1D108D4EC2C3, 0xD389B47879823479},  // 1e-122
    {0xCEDF722A585139BA, 0x843610CB4BF160CB},  // 1e-121
    {0xC2974EB4EE658828, 0xA54394FE1EEDB8FE},  // 1e-120
    {0x733D226229FEEA32, 0xCE947A3DA6A9273E},  // 1e-119
    {0x0806357D5A3F525F, 0x811CCC668829B887},  // 1e-118
    {0xCA07C2DCB0CF26F7, 0xA163FF802A3426A8},  // 1e-117
    {0xFC89B393DD02F0B5, 0xC9BCFF6034C13052},  // 1e-116
    {0xBBAC2078D443ACE2, 0xFC2C3F3841F17C67},  // 1e-115
    {0xD54B944B84AA4C0D, 0x9D9BA7832936EDC0},  // 1e-114
    {0x0A9E795E65D4DF11, 0xC5029163F384A931},  // 1e-113
    {0x4D4617B5FF4A16D5, 0xF64335BCF065D37D},  // 1e-112
    {0x504BCED1BF8E4E45, 0x99EA0196163FA42E},  // 1e-111
    {0xE45EC2862F71E1D6, 0xC06481FB9BCF8D39},  // 1e-110
    {0x5D767327BB4E5A4C, 0xF07DA27A82C37088},  // 1e-109
    {0x3A6A07F8D510F86F, 0x964E858C91BA2655},  // 1e-108
    {0x890489F70A55368B, 0xBBE226EFB628AFEA},  // 1e-107
    {0x2B45AC74CCEA842E, 0xEADAB0ABA3B2DBE5},  // 1e-106
    {0x3B0B8BC90012929D, 0x92C8AE6B464FC96F},  // 1e-105
    {0x09CE6EBB40173744, 0xB77ADA0617E3BBCB},  // 1e-104
    {0xCC420A6A101D0515, 0xE55990879DDCAABD},  // 1e-103
    {0x9FA946824A12232D, 0x8F57FA54C2A9EAB6},  // 1e-102
    {0x47939822DC96ABF9, 0xB32DF8E9F3546564},  // 1e-101
    {0x59787E2B93BC56F7, 0xDFF9772470297EBD},  // 1e-100
    {0x57EB4EDB3C55B65A, 0x8BFBEA76C619EF36},  // 1e-99
    {0xEDE622920B6B23F1, 0xAEFAE51477A06B03},  // 1e-98
    {0xE95FAB368E45ECED, 0xDAB99E59958885C4},  // 1e-97
    {0x11DBCB0218EBB414, 0x88B402F7FD75539B},  // 1e-96
    {0xD652BDC29F26A119, 0xAAE103B5FCD2A881},  // 1e-95
    {0x4BE76D3346F0495F, 0xD59944A37C0752A2},  // 1e-94
    {0x6F70A4400C562DDB, 0x857FCAE62D8493A5},  // 1e-93
    {0xCB4CCD500F6BB952, 0xA6DFBD9FB8E5B88E},  // 1e-92
    {0x7E2000A41346A7A7, 0xD097AD07A71F26B2},  // 1e-91
    {0x8ED400668C0C28C8, 0x825ECC24C873782F},  // 1e-90
    {0x728900802F0F32FA, 0xA2F67F2DFA90563B},  // 1e-89
    {0x4F2B40A03AD2FFB9, 0xCBB41EF979346BCA},  // 1e-88
    {0xE2F610C84987BFA8, 0xFEA126B7D78186BC},  // 1e-87
    {0x0DD9CA7D2DF4D7C9, 0x9F24B832E6B0F436},  // 1e-86
    {0x91503D1C79720DBB, 0xC6EDE63FA05D3143},  // 1e-85
    {0x75A44C6397CE912A, 0xF8A95FCF88747D94},  // 1e-84
    {0xC986AFBE3EE11ABA, 0x9B69DBE1B548CE7C},  // 1e-83
    {0xFBE85BADCE996168, 0xC24452DA229B021B},  // 1e-82
    {0xFAE27299423FB9C3, 0xF2D56790AB41C2A2},  // 1e-81
    {0xDCCD879FC967D41A, 0x97C560BA6B0919A5},  // 1e-80
    {0x5400E987BBC1C920, 0xBDB6B8E905CB600F},  // 1e-79
    {0x290123E9AAB23B68, 0xED246723473E3813},  // 1e-78
    {0xF9A0B6720AAF6521, 0x9436C0760C86E30B},  // 1e-77
    {0xF808E40E8D5B3E69, 0xB94470938FA89BCE},  // 1e-76
    {0xB60B1D1230B20E04, 0xE7958CB87392C2C2},  // 1e-75
    {0xB1C6F22B5E6F48C2, 0x90BD77F3483BB9B9},  // 1e-74
    {0x1E38AEB6360B1AF3, 0xB4ECD5F01A4AA828},  // 1e-73
    {0x25C6DA63C38DE1B0, 0xE2280B6C20DD5232},  // 1e-72
    {0x579C487E5A38AD0E, 0x8D590723948A535F},  // 1e-71
    {0x2D835A9DF0C6D851, 0xB0AF48EC79ACE837},  // 1e-70
    {0xF8E431456CF88E65, 0xDCDB1B2798182244},  // 1e-69
    {0x1B8E9ECB641B58FF, 0x8A08F0F8BF0F156B},  // 1e-68
    {0xE272467E3D222F3F, 0xAC8B2D36EED2DAC5},  // 1e-67
    {0x5B0ED81DCC6ABB0F, 0xD7ADF884AA879177},  // 1e-66
    {0x98E947129FC2B4E9, 0x86CCBB52EA94BAEA},  // 1e-65
    {0x3F2398D747B36224, 0xA87FEA27A539E9A5},  // 1e-64
    {0x8EEC7F0D19A03AAD, 0xD29FE4B18E88640E},  // 1e-63
    {0x1953CF68300424AC, 0x83A3EEEEF9153E89},  // 1e-62
    {0x5FA8C3423C052DD7, 0xA48CEAAAB75A8E2B},  // 1e-61
    {0x3792F412CB06794D, 0xCDB02555653131B6},  // 1e-60
    {0xE2BBD88BBEE40BD0, 0x808E17555F3EBF11},  // 1e-59
    {0x5B6ACEAEAE9D0EC4, 0xA0B19D2AB70E6ED6},  // 1e-58
    {0xF245825A5A445275, 0xC8DE047564D20A8B},  // 1e-57
    {0xEED6E2F0F0D56712, 0xFB158592BE068D2E},  // 1e-56
    {0x55464DD69685606B, 0x9CED737BB6C4183D},  // 1e-55
    {0xAA97E14C3C26B886, 0xC428D05AA4751E4C},  // 1e-54
    {0xD53DD99F4B3066A8, 0xF53304714D9265DF},  // 1e-53
    {0xE546A8038EFE4029, 0x993FE2C6D07B7FAB},  // 1e-52
    {0xDE98520472BDD033, 0xBF8FDB78849A5F96},  // 1e-51
    {0x963E66858F6D4440, 0xEF73D256A5C0F77C},  // 1e-50
    {0xDDE7001379A44AA8, 0x95A8637627989AAD},  // 1e-49
    {0x5560C018580D5D52, 0xBB127C53B17EC159},  // 1e-48
    {0xAAB8F01E6E10B4A6, 0xE9D71B689DDE71AF},  // 1e-47
    {0xCAB3961304CA70E8, 0x9226712162AB070D},  // 1e-46
    {0x3D607B97C5FD0D22, 0xB6B00D69BB55C8D1},  // 1e-45
    {0x8CB89A7DB77C506A, 0xE45C10C42A2B3B05},  // 1e-44
    {0x77F3608E92ADB242, 0x8EB98A7A9A5B04E3},  // 1e-43
    {0x55F038B237591ED3, 0xB267ED1940F1C61C},  // 1e-42
    {0x6B6C46DEC52F6688, 0xDF01E85F912E37A3},  // 1e-41
    {0x2323AC4B3B3DA015, 0x8B61313BBABCE2C6},  // 1e-40
    {0xABEC975E0A0D081A, 0xAE397D8AA96C1B77},  // 1e-39
    {0x96E7BD358C904A21, 0xD9C7DCED53C72255},  // 1e-38
    {0x7E50D64177DA2E54, 0x881CEA14545C7575},  // 1e-37
    {0xDDE50BD1D5D0B9E9, 0xAA242499697392D2},  // 1e-36
    {0x955E4EC64B44E864, 0xD4AD2DBFC3D07787},  // 1e-35
    {0xBD5AF13BEF0B113E, 0x84EC3C97DA624AB4},  // 1e-34
    {0xECB1AD8AEACDD58E, 0xA6274BBDD0FADD61},  // 1e-33
    {0x67DE18EDA5814AF2, 0xCFB11EAD453994BA},  // 1e-32
    {0x80EACF948770CED7, 0x81CEB32C4B43FCF4},  // 1e-31
    {0xA1258379A94D028D, 0xA2425FF75E14FC31},  // 1e-30
    {0x096EE45813A04330, 0xCAD2F7F5359A3B3E},  // 1e-29
    {0x8BCA9D6E188853FC, 0xFD87B5F28300CA0D},  // 1e-28
    {0x775EA264CF55347D, 0x9E74D1B791E07E48},  // 1e-27
    {0x95364AFE032A819D, 0xC612062576589DDA},  // 1e-26
    {0x3A83DDBD83F52204, 0xF79687AED3EEC551},  // 1e-25
    {0xC4926A9672793542, 0x9ABE14CD44753B52},  // 1e-24
    {0x75B7053C0F178293, 0xC16D9A0095928A27},  // 1e-23
    {0x5324C68B12DD6338, 0xF1C90080BAF72CB1},  // 1e-22
    {0xD3F6FC16EBCA5E03, 0x971DA05074DA7BEE},  // 1e-21
    {0x88F4BB1CA6BCF584, 0xBCE5086492111AEA},  // 1e-20
    {0x2B31E9E3D06C32E5, 0xEC1E4A7DB69561A5},  // 1e-19
    {0x3AFF322E62439FCF, 0x9392EE8E921D5D07},  // 1e-18
    {0x09BEFEB9FAD487C2, 0xB877AA3236A4B449},  // 1e-17
    {0x4C2EBE687989A9B3, 0xE69594BEC44DE15B},  // 1e-16
    {0x0F9D37014BF60A10, 0x901D7CF73AB0ACD9},  // 1e-15
    {0x538484C19EF38C94, 0xB424DC35095CD80F},  // 1e-14
    {0x2865A5F206B06FB9, 0xE12E13424BB40E13},  // 1e-13
    {0xF93F87B7442E45D3, 0x8CBCCC096F5088CB},  // 1e-12
    {0xF78F69A51539D748, 0xAFEBFF0BCB24AAFE},  // 1e-11
    {0xB573440E5A884D1B, 0xDBE6FECEBDEDD5BE},  // 1e-10
    {0x31680A88F8953030, 0x89705F4136B4A597},  // 1e-9
    {0xFDC20D2B36BA7C3D, 0xABCC77118461CEFC},  // 1e-8
    {0x3D32907604691B4C, 0xD6BF94D5E57A42BC},  // 1e-7
    {0xA63F9A49C2C1B10F, 0x8637BD05AF6C69B5},  // 1e-6
    {0x0FCF80DC33721D53, 0xA7C5AC471B478423},  // 1e-5
    {0xD3C36113404EA4A8, 0xD1B71758E219652B},  // 1e-4
    {0x645A1CAC083126E9, 0x83126E978D4FDF3B},  // 1e-3
    {0x3D70A3D70A3D70A3, 0xA3D70A3D70A3D70A},  // 1e-2
    {0xCCCCCCCCCCCCCCCC, 0xCCCCCCCCCCCCCCCC},  // 1e-1
    {0x0000000000000000, 0x8000000000000000},  // 1e0
    {0x0000000000000000, 0xA000000000000000},  // 1e1
    {0x0000000000000000, 0xC800000000000000},  // 1e2
    {0x0000000000000000, 0xFA00000000000000},  // 1e3
    {0x0000000000000000, 0x9C40000000000000},  // 1e4
    {0x0000000000000000, 0xC350000000000000},  // 1e5
    {0x0000000000000000, 0xF424000000000000},  // 1e6
    {0x0000000000000000, 0x9896800000000000},  // 1e7
    {0x0000000000000000, 0xBEBC200000000000},  // 1e8
    {0x0000000000000000, 0xEE6B280000000000},  // 1e9
    {0x0000000000000000, 0x9502F90000000000},  // 1e10
    {0x0000000000000000, 0xBA43B74000000000},  // 1e11
    {0x0000000000000000, 0xE8D4A51000000000},  // 1e12
    {0x0000000000000000, 0x9184E72A00000000},  // 1e13
    {0x0000000000000000, 0xB5E620F480000000},  // 1e14
    {0x0000000000000000, 0xE35FA931A0000000},  // 1e15
    {0x0000000000000000, 0x8E1BC9BF04000000},  // 1e16
    {0x0000000000000000, 0xB1A2BC2EC5000000},  // 1e17
    {0x0000000000000000, 0xDE0B6B3A76400000},  // 1e18
    {0x0000000000000000, 0x8AC7230489E80000},  // 1e19
    {0x0000000000000000, 0xAD78EBC5AC620000},  // 1e20
    {0x0000000000000000, 0xD8D726B7177A8000},  // 1e21
    {0x0000000000000000, 0x878678326EAC9000},  // 1e22
    {0x0000000000000000, 0xA968163F0A57B400},  // 1e23
    {0x0000000000000000, 0xD3C21BCECCEDA100},  // 1e24
    {0x0000000000000000, 0x84595161401484A0},  // 1e25
    {0x0000000000000000, 0xA56FA5B99019A5C8},  // 1e26
    {0x0000000000000000, 0xCECB8F27F4200F3A},  // 1e27
    {0x4000000000000000, 0x813F3978F8940984},  // 1e28
    {0x5000000000000000, 0xA18F07D736B90BE5},  // 1e29
    {0xA400000000000000, 0xC9F2C9CD04674EDE},  // 1e30
    {0x4D00000000000000, 0xFC6F7C4045812296},  // 1e31
    {0xF020000000000000, 0x9DC5ADA82B70B59D},  // 1e32
    {0x6C28000000000000, 0xC5371912364CE305},  // 1e33
    {0xC732000000000000, 0xF684DF56C3E01BC6},  // 1e34
    {0x3C7F400000000000, 0x9A130B963A6C115C},  // 1e35
    {0x4B9F100000000000, 0xC097CE7BC90715B3},  // 1e36
    {0x1E86D40000000000, 0xF0BDC21ABB48DB20},  // 1e37
    {0x1314448000000000, 0x96769950B50D88F4},  // 1e38
    {0x17D955A000000000, 0xBC143FA4E250EB31},  // 1e39
    {0x5DCFAB0800000000, 0xEB194F8E1AE525FD},  // 1e40
    {0x5AA1CAE500000000, 0x92EFD1B8D0CF37BE},  // 1e41
    {0xF14A3D9E40000000, 0xB7ABC627050305AD},  // 1e42
    {0x6D9CCD05D0000000, 0xE596B7B0C643C719},  // 1e43
    {0xE4820023A2000000, 0x8F7E32CE7BEA5C6F},  // 1e44
    {0xDDA2802C8A800000, 0xB35DBF821AE4F38B},  // 1e45
    {0xD50B2037AD200000, 0xE0352F62A19E306E},  // 1e46
    {0x4526F422CC340000, 0x8C213D9DA502DE45},  // 1e47
    {0x9670B12B7F410000, 0xAF298D050E4395D6},  // 1e48
    {0x3C0CDD765F114000, 0xDAF3F04651D47B4C},  // 1e49
    {0xA5880A69FB6AC800, 0x88D8762BF324CD0F},  // 1e50
    {0x8EEA0D047A457A00, 0xAB0E93B6EFEE0053},  // 1e51
    {0x72A4904598D6D880, 0xD5D238A4ABE98068},  // 1e52
    {0x47A6DA2B7F864750, 0x85A36366EB71F041},  // 1e53
    {0x999090B65F67D924, 0xA70C3C40A64E6C51},  // 1e54
    {0xFFF4B4E3F741CF6D, 0xD0CF4B50CFE20765},  // 1e55
    {0xBFF8F10E7A8921A4, 0x82818F1281ED449F},  // 1e56
    {0xAFF72D52192B6A0D, 0xA321F2D7226895C7},  // 1e57
    {0x9BF4F8A69F764490, 0xCBEA6F8CEB02BB39},  // 1e58
    {0x02F236D04753D5B4, 0xFEE50B7025C36A08},  // 1e59
    {0x01D762422C946590, 0x9F4F2726179A2245},  // 1e60
    {0x424D3AD2B7B97EF5, 0xC722F0EF9D80AAD6},  // 1e61
    {0xD2E0898765A7DEB2, 0xF8EBAD2B84E0D58B},  // 1e62
    {0x63CC55F49F88EB2F, 0x9B934C3B330C8577},  // 1e63
    {0x3CBF6B71C76B25FB, 0xC2781F49FFCFA6D5},  // 1e64
    {0x8BEF464E3945EF7A, 0xF316271C7FC3908A},  // 1e65
    {0x97758BF0E3CBB5AC, 0x97EDD871CFDA3A56},  // 1e66
    {0x3D52EEED1CBEA317, 0xBDE94E8E43D0C8EC},  // 1e67
    {0x4CA7AAA863EE4BDD, 0xED63A231D4C4FB27},  // 1e68
    {0x8FE8CAA93E74EF6A, 0x945E455F24FB1CF8},  // 1e69
    {0xB3E2FD538E122B44, 0xB975D6B6EE39E436},  // 1e70
    {0x60DBBCA87196B616, 0xE7D34C64A9C85D44},  // 1e71
    {0xBC8955E946FE31CD, 0x90E40FBEEA1D3A4A},  // 1e72
    {0x6BABAB6398BDBE41, 0xB51D13AEA4A488DD},  // 1e73
    {0xC696963C7EED2DD1, 0xE264589A4DCDAB14},  // 1e74
    {0xFC1E1DE5CF543CA2, 0x8D7EB76070A08AEC},  // 1e75
    {0x3B25A55F43294BCB, 0xB0DE65388CC8ADA8},  // 1e76
    {0x49EF0EB713F39EBE, 0xDD15FE86AFFAD912},  // 1e77
    {0x6E3569326C784337, 0x8A2DBF142DFCC7AB},  // 1e78
    {0x49C2C37F07965404, 0xACB92ED9397BF996},  // 1e79
    {0xDC33745EC97BE906, 0xD7E77A8F87DAF7FB},  // 1e80
    {0x69A028BB3DED71A3, 0x86F0AC99B4E8DAFD},  // 1e81
    {0xC40832EA0D68CE0C, 0xA8ACD7C0222311BC},  // 1e82
    {0xF50A3FA490C30190, 0xD2D80DB02AABD62B},  // 1e83
    {0x792667C6DA79E0FA, 0x83C7088E1AAB65DB},  // 1e84
    {0x577001B891185938, 0xA4B8CAB1A1563F52},  // 1e85
    {0xED4C0226B55E6F86, 0xCDE6FD5E09ABCF26},  // 1e86
    {0x544F8158315B05B4, 0x80B05E5AC60B6178},  // 1e87
    {0x696361AE3DB1C721, 0xA0DC75F1778E39D6},  // 1e88
    {0x03BC3A19CD1E38E9, 0xC913936DD571C84C},  // 1e89
    {0x04AB48A04065C723, 0xFB5878494ACE3A5F},  // 1e90
    {0x62EB0D64283F9C76, 0x9D174B2DCEC0E47B},  // 1e91
    {0x3BA5D0BD324F8394, 0xC45D1DF942711D9A},  // 1e92
    {0xCA8F44EC7EE36479, 0xF5746577930D6500},  // 1e93
    {0x7E998B13CF4E1ECB, 0x9968BF6ABBE85F20},  // 1e94
    {0x9E3FEDD8C321A67E, 0xBFC2EF456AE276E8},  // 1e95
    {0xC5CFE94EF3EA101E, 0xEFB3AB16C59B14A2},  // 1e96
    {0xBBA1F1D158724A12, 0x95D04AEE3B80ECE5},  // 1e97
    {0x2A8A6E45AE8EDC97, 0xBB445DA9CA61281F},  // 1e98
    {0xF52D09D71A3293BD, 0xEA1575143CF97226},  // 1e99
    {0x593C2626705F9C56, 0x924D692CA61BE758},  // 1e100
    {0x6F8B2FB00C77836C, 0xB6E0C377CFA2E12E},  // 1e101
    {0x0B6DFB9C0F956447, 0xE498F455C38B997A},  // 1e102
    {0x4724BD4189BD5EAC, 0x8EDF98B59A373FEC},  // 1e103
    {0x58EDEC91EC2CB657, 0xB2977EE300C50FE7},  // 1e104
    {0x2F2967B66737E3ED, 0xDF3D5E9BC0F653E1},  // 1e105
    {0xBD79E0D20082EE74, 0x8B865B215899F46C},  // 1e106
    {0xECD8590680A3AA11, 0xAE67F1E9AEC07187},  // 1e107
    {0xE80E6F4820CC9495, 0xDA01EE641A708DE9},  // 1e108
    {0x3109058D147FDCDD, 0x884134FE908658B2},  // 1e109
    {0xBD4B46F0599FD415, 0xAA51823E34A7EEDE},  // 1e110
    {0x6C9E18AC7007C91A, 0xD4E5E2CDC1D1EA96},  // 1e111
    {0x03E2CF6BC604DDB0, 0x850FADC09923329E},  // 1e112
    {0x84DB8346B786151C, 0xA6539930BF6BFF45},  // 1e113
    {0xE612641865679A63, 0xCFE87F7CEF46FF16},  // 1e114
    {0x4FCB7E8F3F60C07E, 0x81F14FAE158C5F6E},  // 1e115
    {0xE3BE5E330F38F09D, 0xA26DA3999AEF7749},  // 1e116
    {0x5CADF5BFD3072CC5, 0xCB090C8001AB551C},  // 1e117
    {0x73D9732FC7C8F7F6, 0xFDCB4FA002162A63},  // 1e118
    {0x2867E7FDDCDD9AFA, 0x9E9F11C4014DDA7E},  // 1e119
    {0xB281E1FD541501B8, 0xC646D63501A1511D},  // 1e120
    {0x1F225A7CA91A4226, 0xF7D88BC24209A565},  // 1e121
    {0x3375788DE9B06958, 0x9AE757596946075F},  // 1e122
    {0x0052D6B1641C83AE, 0xC1A12D2FC3978937},  // 1e123
    {0xC0678C5DBD23A49A, 0xF209787BB47D6B84},  // 1e124
    {0xF840B7BA963646E0, 0x9745EB4D50CE6332},  // 1e125
    {0xB650E5A93BC3D898, 0xBD176620A501FBFF},  // 1e126
    {0xA3E51F138AB4CEBE, 0xEC5D3FA8CE427AFF},  // 1e127
    {0xC66F336C36B10137, 0x93BA47C980E98CDF},  // 1e128
    {0xB80B0047445D4184, 0xB8A8D9BBE123F017},  // 1e129
    {0xA60DC059157491E5, 0xE6D3102AD96CEC1D},  // 1e130
    {0x87C89837AD68DB2F, 0x9043EA1AC7E41392},  // 1e131
    {0x29BABE4598C311FB, 0xB454E4A179DD1877},  // 1e132
    {0xF4296DD6FEF3D67A, 0xE16A1DC9D8545E94},  // 1e133
    {0x1899E4A65F58660C, 0x8CE2529E2734BB1D},  // 1e134
    {0x5EC05DCFF72E7F8F, 0xB01AE745B101E9E4},  // 1e135
    {0x76707543F4FA1F73, 0xDC21A1171D42645D},  // 1e136
    {0x6A06494A791C53A8, 0x899504AE72497EBA},  // 1e137
    {0x0487DB9D17636892, 0xABFA45DA0EDBDE69},  // 1e138
    {0x45A9D2845D3C42B6, 0xD6F8D7509292D603},  // 1e139
    {0x0B8A2392BA45A9B2, 0x865B86925B9BC5C2},  // 1e140
    {0x8E6CAC7768D7141E, 0xA7F26836F282B732},  // 1e141
    {0x3207D795430CD926, 0xD1EF0244AF2364FF},  // 1e142
    {0x7F44E6BD49E807B8, 0x8335616AED761F1F},  // 1e143
    {0x5F16206C9C6209A6, 0xA402B9C5A8D3A6E7},  // 1e144
    {0x36DBA887C37A8C0F, 0xCD036837130890A1},  // 1e145
    {0xC2494954DA2C9789, 0x802221226BE55A64},  // 1e146
    {0xF2DB9BAA10B7BD6C, 0xA02AA96B06DEB0FD},  // 1e147
    {0x6F92829494E5ACC7, 0xC83553C5C8965D3D},  // 1e148
    {0xCB772339BA1F17F9, 0xFA42A8B73ABBF48C},  // 1e149
    {0xFF2A760414536EFB, 0x9C69A97284B578D7},  // 1e150
    {0xFEF5138519684ABA, 0xC38413CF25E2D70D},  // 1e151
    {0x7EB258665FC25D69, 0xF46518C2EF5B8CD1},  // 1e152
    {0xEF2F773FFBD97A61, 0x98BF2F79D5993802},  // 1e153
    {0xAAFB550FFACFD8FA, 0xBEEEFB584AFF8603},  // 1e154
    {0x95BA2A53F983CF38, 0xEEAABA2E5DBF6784},  // 1e155
    {0xDD945A747BF26183, 0x952AB45CFA97A0B2},  // 1e156
    {0x94F971119AEEF9E4, 0xBA756174393D88DF},  // 1e157
    {0x7A37CD5601AAB85D, 0xE912B9D1478CEB17},  // 1e158
    {0xAC62E055C10AB33A, 0x91ABB422CCB812EE},  // 1e159
    {0x577B986B314D6009, 0xB616A12B7FE617AA},  // 1e160
    {0xED5A7E85FDA0B80B, 0xE39C49765FDF9D94},  // 1e161
    {0x14588F13BE847307, 0x8E41ADE9FBEBC27D},  // 1e162
    {0x596EB2D8AE258FC8, 0xB1D219647AE6B31C},  // 1e163
    {0x6FCA5F8ED9AEF3BB, 0xDE469FBD99A05FE3},  // 1e164
    {0x25DE7BB9480D5854, 0x8AEC23D680043BEE},  // 1e165
    {0xAF561AA79A10AE6A, 0xADA72CCC20054AE9},  // 1e166
    {0x1B2BA1518094DA04, 0xD910F7FF28069DA4},  // 1e167
    {0x90FB44D2F05D0842, 0x87AA9AFF79042286},  // 1e168
    {0x353A1607AC744A53, 0xA99541BF57452B28},  // 1e169
    {0x42889B8997915CE8, 0xD3FA922F2D1675F2},  // 1e170
    {0x69956135FEBADA11, 0x847C9B5D7C2E09B7},  // 1e171
    {0x43FAB9837E699095, 0xA59BC234DB398C25},  // 1e172
    {0x94F967E45E03F4BB, 0xCF02B2C21207EF2E},  // 1e173
    {0x1D1BE0EEBAC278F5, 0x8161AFB94B44F57D},  // 1e174
    {0x6462D92A69731732, 0xA1BA1BA79E1632DC},  // 1e175
    {0x7D7B8F7503CFDCFE, 0xCA28A291859BBF93},  // 1e176
    {0x5CDA735244C3D43E, 0xFCB2CB35E702AF78},  // 1e177
    {0x3A0888136AFA64A7, 0x9DEFBF01B061ADAB},  // 1e178
    {0x088AAA1845B8FDD0, 0xC56BAEC21C7A1916},  // 1e179
    {0x8AAD549E57273D45, 0xF6C69A72A3989F5B},  // 1e180
    {0x36AC54E2F678864B, 0x9A3C2087A63F6399},  // 1e181
    {0x84576A1BB416A7DD, 0xC0CB28A98FCF3C7F},  // 1e182
    {0x656D44A2A11C51D5, 0xF0FDF2D3F3C30B9F},  // 1e183
    {0x9F644AE5A4B1B325, 0x969EB7C47859E743},  // 1e184
    {0x873D5D9F0DDE1FEE, 0xBC4665B596706114},  // 1e185
    {0xA90CB506D155A7EA, 0xEB57FF22FC0C7959},  // 1e186
    {0x09A7F12442D588F2, 0x9316FF75DD87CBD8},  // 1e187
    {0x0C11ED6D538AEB2F, 0xB7DCBF5354E9BECE},  // 1e188
    {0x8F1668C8A86DA5FA, 0xE5D3EF282A242E81},  // 1e189
    {0xF96E017D694487BC, 0x8FA475791A569D10},  // 1e190
    {0x37C981DCC395A9AC, 0xB38D92D760EC4455},  // 1e191
    {0x85BBE253F47B1417, 0xE070F78D3927556A},  // 1e192
    {0x93956D7478CCEC8E, 0x8C469AB843B89562},  // 1e193
    {0x387AC8D1970027B2, 0xAF58416654A6BABB},  // 1e194
    {0x06997B05FCC0319E, 0xDB2E51BFE9D0696A},  // 1e195
    {0x441FECE3BDF81F03, 0x88FCF317F22241E2},  // 1e196
    {0xD527E81CAD7626C3, 0xAB3C2FDDEEAAD25A},  // 1e197
    {0x8A71E223D8D3B074, 0xD60B3BD56A5586F1},  // 1e198
    {0xF6872D5667844E49, 0x85C7056562757456},  // 1e199
    {0xB428F8AC016561DB, 0xA738C6BEBB12D16C},  // 1e200
    {0xE13336D701BEBA52, 0xD106F86E69D785C7},  // 1e201
    {0xECC0024661173473, 0x82A45B450226B39C},  // 1e202
    {0x27F002D7F95D0190, 0xA34D721642B06084},  // 1e203
    {0x31EC038DF7B441F4, 0xCC20CE9BD35C78A5},  // 1e204
    {0x7E67047175A15271, 0xFF290242C83396CE},  // 1e205
    {0x0F0062C6E984D386, 0x9F79A169BD203E41},  // 1e206
    {0x52C07B78A3E60868, 0xC75809C42C684DD1},  // 1e207
    {0xA7709A56CCDF8A82, 0xF92E0C3537826145},  // 1e208
    {0x88A66076400BB691, 0x9BBCC7A142B17CCB},  // 1e209
    {0x6ACFF893D00EA435, 0xC2ABF989935DDBFE},  // 1e210
    {0x0583F6B8C4124D43, 0xF356F7EBF83552FE},  // 1e211
    {0xC3727A337A8B704A, 0x98165AF37B2153DE},  // 1e212
    {0x744F18C0592E4C5C, 0xBE1BF1B059E9A8D6},  // 1e213
    {0x1162DEF06F79DF73, 0xEDA2EE1C7064130C},  // 1e214
    {0x8ADDCB5645AC2BA8, 0x9485D4D1C63E8BE7},  // 1e215
    {0x6D953E2BD7173692, 0xB9A74A0637CE2EE1},  // 1e216
    {0xC8FA8DB6CCDD0437, 0xE8111C87C5C1BA99},  // 1e217
    {0x1D9C9892400A22A2, 0x910AB1D4DB9914A0},  // 1e218
    {0x2503BEB6D00CAB4B, 0xB54D5E4A127F59C8},  // 1e219
    {0x2E44AE64840FD61D, 0xE2A0B5DC971F303A},  // 1e220
    {0x5CEAECFED289E5D2, 0x8DA471A9DE737E24},  // 1e221
    {0x7425A83E872C5F47, 0xB10D8E1456105DAD},  // 1e222
    {0xD12F124E28F77719, 0xDD50F1996B947518},  // 1e223
    {0x82BD6B70D99AAA6F, 0x8A5296FFE33CC92F},  // 1e224
    {0x636CC64D1001550B, 0xACE73CBFDC0BFB7B},  // 1e225
    {0x3C47F7E05401AA4E, 0xD8210BEFD30EFA5A},  // 1e226
    {0x65ACFAEC34810A71, 0x8714A775E3E95C78},  // 1e227
    {0x7F1839A741A14D0D, 0xA8D9D1535CE3B396},  // 1e228
    {0x1EDE48111209A050, 0xD31045A8341CA07C},  // 1e229
    {0x934AED0AAB460432, 0x83EA2B892091E44D},  // 1e230
    {0xF81DA84D5617853F, 0xA4E4B66B68B65D60},  // 1e231
    {0x36251260AB9D668E, 0xCE1DE40642E3F4B9},  // 1e232
    {0xC1D72B7C6B426019, 0x80D2AE83E9CE78F3},  // 1e233
    {0xB24CF65B8612F81F, 0xA1075A24E4421730},  // 1e234
    {0xDEE033F26797B627, 0xC94930AE1D529CFC},  // 1e235
    {0x169840EF017DA3B1, 0xFB9B7CD9A4A7443C},  // 1e236
    {0x8E1F289560EE864E, 0x9D412E0806E88AA5},  // 1e237
    {0xF1A6F2BAB92A27E2, 0xC491798A08A2AD4E},  // 1e238
    {0xAE10AF696774B1DB, 0xF5B5D7EC8ACB58A2},  // 1e239
    {0xACCA6DA1E0A8EF29, 0x9991A6F3D6BF1765},  // 1e240
    {0x17FD090A58D32AF3, 0xBFF610B0CC6EDD3F},  // 1e241
    {0xDDFC4B4CEF07F5B0, 0xEFF394DCFF8A948E},  // 1e242
    {0x4ABDAF101564F98E, 0x95F83D0A1FB69CD9},  // 1e243
    {0x9D6D1AD41ABE37F1, 0xBB764C4CA7A4440F},  // 1e244
    {0x84C86189216DC5ED, 0xEA53DF5FD18D5513},  // 1e245
    {0x32FD3CF5B4E49BB4, 0x92746B9BE2F8552C},  // 1e246
    {0x3FBC8C33221DC2A1, 0xB7118682DBB66A77},  // 1e247
    {0x0FABAF3FEAA5334A, 0xE4D5E82392A40515},  // 1e248
    {0x29CB4D87F2A7400E, 0x8F05B1163BA6832D},  // 1e249
    {0x743E20E9EF511012, 0xB2C71D5BCA9023F8},  // 1e250
    {0x914DA9246B255416, 0xDF78E4B2BD342CF6},  // 1e251
    {0x1AD089B6C2F7548E, 0x8BAB8EEFB6409C1A},  // 1e252
    {0xA184AC2473B529B1, 0xAE9672ABA3D0C320},  // 1e253
    {0xC9E5D72D90A2741E, 0xDA3C0F568CC4F3E8},  // 1e254
    {0x7E2FA67C7A658892, 0x8865899617FB1871},  // 1e255
    {0xDDBB901B98FEEAB7, 0xAA7EEBFB9DF9DE8D},  // 1e256
    {0x552A74227F3EA565, 0xD51EA6FA85785631},  // 1e257
    {0xD53A88958F87275F, 0x8533285C936B35DE},  // 1e258
    {0x8A892ABAF368F137, 0xA67FF273B8460356},  // 1e259
    {0x2D2B7569B0432D85, 0xD01FEF10A657842C},  // 1e260
    {0x9C3B29620E29FC73, 0x8213F56A67F6B29B},  // 1e261
    {0x8349F3BA91B47B8F, 0xA298F2C501F45F42},  // 1e262
    {0x241C70A936219A73, 0xCB3F2F7642717713},  // 1e263
    {0xED238CD383AA0110, 0xFE0EFB53D30DD4D7},  // 1e264
    {0xF4363804324A40AA, 0x9EC95D1463E8A506},  // 1e265
    {0xB143C6053EDCD0D5, 0xC67BB4597CE2CE48},  // 1e266
    {0xDD94B7868E94050A, 0xF81AA16FDC1B81DA},  // 1e267
    {0xCA7CF2B4191C8326, 0x9B10A4E5E9913128},  // 1e268
    {0xFD1C2F611F63A3F0, 0xC1D4CE1F63F57D72},  // 1e269
    {0xBC633B39673C8CEC, 0xF24A01A73CF2DCCF},  // 1e270
    {0xD5BE0503E085D813, 0x976E41088617CA01},  // 1e271
    {0x4B2D8644D8A74E18, 0xBD49D14AA79DBC82},  // 1e272
    {0xDDF8E7D60ED1219E, 0xEC9C459D51852BA2},  // 1e273
    {0xCABB90E5C942B503, 0x93E1AB8252F33B45},  // 1e274
    {0x3D6A751F3B936243, 0xB8DA1662E7B00A17},  // 1e275
    {0x0CC512670A783AD4, 0xE7109BFBA19C0C9D},  // 1e276
    {0x27FB2B80668B24C5, 0x906A617D450187E2},  // 1e277
    {0xB1F9F660802DEDF6, 0xB484F9DC9641E9DA},  // 1e278
    {0x5E7873F8A0396973, 0xE1A63853BBD26451},  // 1e279
    {0xDB0B487B6423E1E8, 0x8D07E33455637EB2},  // 1e280
    {0x91CE1A9A3D2CDA62, 0xB049DC016ABC5E5F},  // 1e281
    {0x7641A140CC7810FB, 0xDC5C5301C56B75F7},  // 1e282
    {0xA9E904C87FCB0A9D, 0x89B9B3E11B6329BA},  // 1e283
    {0x546345FA9FBDCD44, 0xAC2820D9623BF429},  // 1e284
    {0xA97C177947AD4095, 0xD732290FBACAF133},  // 1e285
    {0x49ED8EABCCCC485D, 0x867F59A9D4BED6C0},  // 1e286
    {0x5C68F256BFFF5A74, 0xA81F301449EE8C70},  // 1e287
    {0x73832EEC6FFF3111, 0xD226FC195C6A2F8C},  // 1e288
};

#if PK_FLOATCONV_HAS_EXACT_DOUBLE_ARITHMETIC  // [pocketpy] Deviation 4.
// wuffs_private_impl__f64_powers_of_10 holds powers of 10 that can be exactly
// represented by a float64 (what C calls a double).
static const double wuffs_private_impl__f64_powers_of_10[23] = {
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
    1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22,
};
#endif

// --------

#define WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE 2047
#define WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION 800

// WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL is the largest N such that
// ((10 << N) < (1 << 64)).
#define WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL 60

// wuffs_private_impl__high_prec_dec (abbreviated as HPD) is a fixed precision
// floating point decimal number, augmented with ±infinity values, but it
// cannot represent NaN (Not a Number).
//
// "High precision" means that the mantissa holds 800 decimal digits. 800 is
// WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION.
//
// An HPD isn't for general purpose arithmetic, only for conversions to and
// from IEEE 754 double-precision floating point, where the largest and
// smallest positive, finite values are approximately 1.8e+308 and 4.9e-324.
// HPD exponents above +2047 mean infinity, below -2047 mean zero. The ±2047
// bounds are further away from zero than ±(324 + 800), where 800 and 2047 is
// WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION and
// WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE.
//
// digits[.. num_digits] are the number's digits in big-endian order. The
// uint8_t values are in the range [0 ..= 9], not ['0' ..= '9'], where e.g. '7'
// is the ASCII value 0x37.
//
// decimal_point is the index (within digits) of the decimal point. It may be
// negative or be larger than num_digits, in which case the explicit digits are
// padded with implicit zeroes.
//
// For example, if num_digits is 3 and digits is "\x07\x08\x09":
//  - A decimal_point of -2 means ".00789"
//  - A decimal_point of -1 means ".0789"
//  - A decimal_point of +0 means ".789"
//  - A decimal_point of +1 means "7.89"
//  - A decimal_point of +2 means "78.9"
//  - A decimal_point of +3 means "789."
//  - A decimal_point of +4 means "7890."
//  - A decimal_point of +5 means "78900."
//
// As above, a decimal_point higher than +2047 means that the overall value is
// infinity, lower than -2047 means zero.
//
// negative is a sign bit. An HPD can distinguish positive and negative zero.
//
// truncated is whether there are more than
// WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION digits, and at least one of those
// extra digits are non-zero. The existence of long-tail digits can affect
// rounding.
//
// The "all fields are zero" value is valid, and represents the number +0.
typedef struct wuffs_private_impl__high_prec_dec__struct {
  uint32_t num_digits;
  int32_t decimal_point;
  bool negative;
  bool truncated;
  uint8_t digits[WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION];
} wuffs_private_impl__high_prec_dec;

// wuffs_private_impl__high_prec_dec__trim trims trailing zeroes from the
// h->digits[.. h->num_digits] slice. They have no benefit, since we explicitly
// track h->decimal_point.
//
// Preconditions:
//  - h is non-NULL.
static inline void  //
wuffs_private_impl__high_prec_dec__trim(wuffs_private_impl__high_prec_dec* h) {
  while ((h->num_digits > 0) && (h->digits[h->num_digits - 1] == 0)) {
    h->num_digits--;
  }
}

// wuffs_private_impl__high_prec_dec__assign sets h to represent the number x.
//
// Preconditions:
//  - h is non-NULL.
static void  //
wuffs_private_impl__high_prec_dec__assign(wuffs_private_impl__high_prec_dec* h,
                                          uint64_t x,
                                          bool negative) {
  uint32_t n = 0;

  // Set h->digits.
  if (x > 0) {
    // Calculate the digits, working right-to-left. After we determine n (how
    // many digits there are), copy from buf to h->digits.
    //
    // UINT64_MAX, 18446744073709551615, is 20 digits long. It can be faster to
    // copy a constant number of bytes than a variable number (20 instead of
    // n). Make buf large enough (and start writing to it from the middle) so
    // that can we always copy 20 bytes: the slice buf[(20-n) .. (40-n)].
    uint8_t buf[40] = {0};
    uint8_t* ptr = &buf[20];
    do {
      uint64_t remaining = x / 10;
      x -= remaining * 10;
      ptr--;
      *ptr = (uint8_t)x;
      n++;
      x = remaining;
    } while (x > 0);
    memcpy(h->digits, ptr, 20);
  }

  // Set h's other fields.
  h->num_digits = n;
  h->decimal_point = (int32_t)n;
  h->negative = negative;
  h->truncated = false;
  wuffs_private_impl__high_prec_dec__trim(h);
}

static wuffs_base__status  //
wuffs_private_impl__high_prec_dec__parse(wuffs_private_impl__high_prec_dec* h,
                                         wuffs_base__slice_u8 s,
                                         uint32_t options) {
  if (!h) {
    return wuffs_base__make_status(wuffs_base__error__bad_receiver);
  }
  h->num_digits = 0;
  h->decimal_point = 0;
  h->negative = false;
  h->truncated = false;

  uint8_t* p = s.ptr;
  uint8_t* q = s.ptr + s.len;

  if (options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES) {
    for (;; p++) {
      if (p >= q) {
        return wuffs_base__make_status(wuffs_base__error__bad_argument);
      } else if (*p != '_') {
        break;
      }
    }
  }

  // Parse sign.
  do {
    if (*p == '+') {
      p++;
    } else if (*p == '-') {
      h->negative = true;
      p++;
    } else {
      break;
    }
    if (options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES) {
      for (;; p++) {
        if (p >= q) {
          return wuffs_base__make_status(wuffs_base__error__bad_argument);
        } else if (*p != '_') {
          break;
        }
      }
    }
  } while (0);

  // Parse digits, up to (and including) a '.', 'E' or 'e'. Examples for each
  // limb in this if-else chain:
  //  - "0.789"
  //  - "1002.789"
  //  - ".789"
  //  - Other (invalid input).
  uint32_t nd = 0;
  int32_t dp = 0;
  bool no_digits_before_separator = false;
  if (('0' == *p) &&
      !(options &
        WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_MULTIPLE_LEADING_ZEROES)) {
    p++;
    for (;; p++) {
      if (p >= q) {
        goto after_all;
      } else if (*p ==
                 ((options &
                   WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
                      ? ','
                      : '.')) {
        p++;
        goto after_sep;
      } else if ((*p == 'E') || (*p == 'e')) {
        p++;
        goto after_exp;
      } else if ((*p != '_') ||
                 !(options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES)) {
        return wuffs_base__make_status(wuffs_base__error__bad_argument);
      }
    }

  } else if (('0' <= *p) && (*p <= '9')) {
    if (*p == '0') {
      for (; (p < q) && (*p == '0'); p++) {
      }
    } else {
      h->digits[nd++] = (uint8_t)(*p - '0');
      dp = (int32_t)nd;
      p++;
    }

    for (;; p++) {
      if (p >= q) {
        goto after_all;
      } else if (('0' <= *p) && (*p <= '9')) {
        if (nd < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
          h->digits[nd++] = (uint8_t)(*p - '0');
          dp = (int32_t)nd;
        } else if ('0' != *p) {
          // Long-tail non-zeroes set the truncated bit.
          h->truncated = true;
        }
      } else if (*p ==
                 ((options &
                   WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
                      ? ','
                      : '.')) {
        p++;
        goto after_sep;
      } else if ((*p == 'E') || (*p == 'e')) {
        p++;
        goto after_exp;
      } else if ((*p != '_') ||
                 !(options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES)) {
        return wuffs_base__make_status(wuffs_base__error__bad_argument);
      }
    }

  } else if (*p == ((options &
                     WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
                        ? ','
                        : '.')) {
    p++;
    no_digits_before_separator = true;

  } else {
    return wuffs_base__make_status(wuffs_base__error__bad_argument);
  }

after_sep:
  for (;; p++) {
    if (p >= q) {
      goto after_all;
    } else if ('0' == *p) {
      if (nd == 0) {
        // Track leading zeroes implicitly.
        dp--;
      } else if (nd < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
        h->digits[nd++] = (uint8_t)(*p - '0');
      }
    } else if (('0' < *p) && (*p <= '9')) {
      if (nd < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
        h->digits[nd++] = (uint8_t)(*p - '0');
      } else {
        // Long-tail non-zeroes set the truncated bit.
        h->truncated = true;
      }
    } else if ((*p == 'E') || (*p == 'e')) {
      p++;
      goto after_exp;
    } else if ((*p != '_') ||
               !(options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES)) {
      return wuffs_base__make_status(wuffs_base__error__bad_argument);
    }
  }

after_exp:
  do {
    if (options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES) {
      for (;; p++) {
        if (p >= q) {
          return wuffs_base__make_status(wuffs_base__error__bad_argument);
        } else if (*p != '_') {
          break;
        }
      }
    }

    int32_t exp_sign = +1;
    if (*p == '+') {
      p++;
    } else if (*p == '-') {
      exp_sign = -1;
      p++;
    }

    int32_t exp = 0;
    const int32_t exp_large = WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE +
                              WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION;
    bool saw_exp_digits = false;
    for (; p < q; p++) {
      if ((*p == '_') &&
          (options & WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES)) {
        // No-op.
      } else if (('0' <= *p) && (*p <= '9')) {
        saw_exp_digits = true;
        if (exp < exp_large) {
          exp = (10 * exp) + ((int32_t)(*p - '0'));
        }
      } else {
        break;
      }
    }
    if (!saw_exp_digits) {
      return wuffs_base__make_status(wuffs_base__error__bad_argument);
    }
    dp += exp_sign * exp;
  } while (0);

after_all:
  if (p != q) {
    return wuffs_base__make_status(wuffs_base__error__bad_argument);
  }
  h->num_digits = nd;
  if (nd == 0) {
    if (no_digits_before_separator) {
      return wuffs_base__make_status(wuffs_base__error__bad_argument);
    }
    h->decimal_point = 0;
  } else if (dp < -WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE) {
    h->decimal_point = -WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE - 1;
  } else if (dp > +WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE) {
    h->decimal_point = +WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE + 1;
  } else {
    h->decimal_point = dp;
  }
  wuffs_private_impl__high_prec_dec__trim(h);
  return wuffs_base__make_status(NULL);
}

// --------

// wuffs_private_impl__high_prec_dec__lshift_num_new_digits returns the number
// of additional decimal digits when left-shifting by shift.
//
// See below for preconditions.
static uint32_t  //
wuffs_private_impl__high_prec_dec__lshift_num_new_digits(
    wuffs_private_impl__high_prec_dec* h,
    uint32_t shift) {
  // Masking with 0x3F should be unnecessary (assuming the preconditions) but
  // it's cheap and ensures that we don't overflow the
  // wuffs_private_impl__hpd_left_shift array.
  shift &= 63;

  uint32_t x_a = wuffs_private_impl__hpd_left_shift[shift];
  uint32_t x_b = wuffs_private_impl__hpd_left_shift[shift + 1];
  uint32_t num_new_digits = x_a >> 11;
  uint32_t pow5_a = 0x7FF & x_a;
  uint32_t pow5_b = 0x7FF & x_b;

  const uint8_t* pow5 = &wuffs_private_impl__powers_of_5[pow5_a];
  uint32_t i = 0;
  uint32_t n = pow5_b - pow5_a;
  for (; i < n; i++) {
    if (i >= h->num_digits) {
      return num_new_digits - 1;
    } else if (h->digits[i] == pow5[i]) {
      continue;
    } else if (h->digits[i] < pow5[i]) {
      return num_new_digits - 1;
    } else {
      return num_new_digits;
    }
  }
  return num_new_digits;
}

// --------

// wuffs_private_impl__high_prec_dec__rounded_integer returns the integral
// (non-fractional) part of h, provided that it is 18 or fewer decimal digits.
// For 19 or more digits, it returns UINT64_MAX. Note that:
//  - (1 << 53) is    9007199254740992, which has 16 decimal digits.
//  - (1 << 56) is   72057594037927936, which has 17 decimal digits.
//  - (1 << 59) is  576460752303423488, which has 18 decimal digits.
//  - (1 << 63) is 9223372036854775808, which has 19 decimal digits.
// and that IEEE 754 double precision has 52 mantissa bits.
//
// That integral part is rounded-to-even: rounding 7.5 or 8.5 both give 8.
//
// h's negative bit is ignored: rounding -8.6 returns 9.
//
// See below for preconditions.
static uint64_t  //
wuffs_private_impl__high_prec_dec__rounded_integer(
    wuffs_private_impl__high_prec_dec* h) {
  if ((h->num_digits == 0) || (h->decimal_point < 0)) {
    return 0;
  } else if (h->decimal_point > 18) {
    return UINT64_MAX;
  }

  uint32_t dp = (uint32_t)(h->decimal_point);
  uint64_t n = 0;
  uint32_t i = 0;
  for (; i < dp; i++) {
    n = (10 * n) + ((i < h->num_digits) ? h->digits[i] : 0);
  }

  bool round_up = false;
  if (dp < h->num_digits) {
    round_up = h->digits[dp] >= 5;
    if ((h->digits[dp] == 5) && (dp + 1 == h->num_digits)) {
      // We are exactly halfway. If we're truncated, round up, otherwise round
      // to even.
      round_up = h->truncated ||  //
                 ((dp > 0) && (1 & h->digits[dp - 1]));
    }
  }
  if (round_up) {
    n++;
  }

  return n;
}

// wuffs_private_impl__high_prec_dec__small_xshift shifts h's number (where 'x'
// is 'l' or 'r' for left or right) by a small shift value.
//
// Preconditions:
//  - h is non-NULL.
//  - h->decimal_point is "not extreme".
//  - shift is non-zero.
//  - shift is "a small shift".
//
// "Not extreme" means within ±WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE.
//
// "A small shift" means not more than
// WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL.
//
// wuffs_private_impl__high_prec_dec__rounded_integer and
// wuffs_private_impl__high_prec_dec__lshift_num_new_digits have the same
// preconditions.
//
// wuffs_private_impl__high_prec_dec__lshift keeps the first two preconditions
// but not the last two. Its shift argument is signed and does not need to be
// "small": zero is a no-op, positive means left shift and negative means right
// shift.

static void  //
wuffs_private_impl__high_prec_dec__small_lshift(
    wuffs_private_impl__high_prec_dec* h,
    uint32_t shift) {
  if (h->num_digits == 0) {
    return;
  }
  uint32_t num_new_digits =
      wuffs_private_impl__high_prec_dec__lshift_num_new_digits(h, shift);
  uint32_t rx = h->num_digits - 1;                   // Read  index.
  uint32_t wx = h->num_digits - 1 + num_new_digits;  // Write index.
  uint64_t n = 0;

  // Repeat: pick up a digit, put down a digit, right to left.
  while (((int32_t)rx) >= 0) {
    n += ((uint64_t)(h->digits[rx])) << shift;
    uint64_t quo = n / 10;
    uint64_t rem = n - (10 * quo);
    if (wx < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
      h->digits[wx] = (uint8_t)rem;
    } else if (rem > 0) {
      h->truncated = true;
    }
    n = quo;
    wx--;
    rx--;
  }

  // Put down leading digits, right to left.
  while (n > 0) {
    uint64_t quo = n / 10;
    uint64_t rem = n - (10 * quo);
    if (wx < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
      h->digits[wx] = (uint8_t)rem;
    } else if (rem > 0) {
      h->truncated = true;
    }
    n = quo;
    wx--;
  }

  // Finish.
  h->num_digits += num_new_digits;
  if (h->num_digits > WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
    h->num_digits = WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION;
  }
  h->decimal_point += (int32_t)num_new_digits;
  wuffs_private_impl__high_prec_dec__trim(h);
}

static void  //
wuffs_private_impl__high_prec_dec__small_rshift(
    wuffs_private_impl__high_prec_dec* h,
    uint32_t shift) {
  uint32_t rx = 0;  // Read  index.
  uint32_t wx = 0;  // Write index.
  uint64_t n = 0;

  // Pick up enough leading digits to cover the first shift.
  while ((n >> shift) == 0) {
    if (rx < h->num_digits) {
      // Read a digit.
      n = (10 * n) + h->digits[rx++];
    } else if (n == 0) {
      // h's number used to be zero and remains zero.
      return;
    } else {
      // Read sufficient implicit trailing zeroes.
      while ((n >> shift) == 0) {
        n = 10 * n;
        rx++;
      }
      break;
    }
  }
  h->decimal_point -= ((int32_t)(rx - 1));
  if (h->decimal_point < -WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE) {
    // After the shift, h's number is effectively zero.
    h->num_digits = 0;
    h->decimal_point = 0;
    h->truncated = false;
    return;
  }

  // Repeat: pick up a digit, put down a digit, left to right.
  uint64_t mask = (((uint64_t)(1)) << shift) - 1;
  while (rx < h->num_digits) {
    uint8_t new_digit = ((uint8_t)(n >> shift));
    n = (10 * (n & mask)) + h->digits[rx++];
    h->digits[wx++] = new_digit;
  }

  // Put down trailing digits, left to right.
  while (n > 0) {
    uint8_t new_digit = ((uint8_t)(n >> shift));
    n = 10 * (n & mask);
    if (wx < WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION) {
      h->digits[wx++] = new_digit;
    } else if (new_digit > 0) {
      h->truncated = true;
    }
  }

  // Finish.
  h->num_digits = wx;
  wuffs_private_impl__high_prec_dec__trim(h);
}

static void  //
wuffs_private_impl__high_prec_dec__lshift(wuffs_private_impl__high_prec_dec* h,
                                          int32_t shift) {
  if (shift > 0) {
    while (shift > +WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL) {
      wuffs_private_impl__high_prec_dec__small_lshift(
          h, WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL);
      shift -= WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL;
    }
    wuffs_private_impl__high_prec_dec__small_lshift(h, ((uint32_t)(+shift)));
  } else if (shift < 0) {
    while (shift < -WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL) {
      wuffs_private_impl__high_prec_dec__small_rshift(
          h, WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL);
      shift += WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL;
    }
    wuffs_private_impl__high_prec_dec__small_rshift(h, ((uint32_t)(-shift)));
  }
}

// --------

// wuffs_private_impl__high_prec_dec__round_etc rounds h's number. For those
// functions that take an n argument, rounding produces at most n digits (which
// is not necessarily at most n decimal places). Negative n values are ignored,
// as well as any n greater than or equal to h's number of digits. The
// etc__round_just_enough function implicitly chooses an n to implement
// WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION.
//
// Preconditions:
//  - h is non-NULL.
//  - h->decimal_point is "not extreme".
//
// "Not extreme" means within ±WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE.

static void  //
wuffs_private_impl__high_prec_dec__round_down(
    wuffs_private_impl__high_prec_dec* h,
    int32_t n) {
  if ((n < 0) || (h->num_digits <= (uint32_t)n)) {
    return;
  }
  h->num_digits = (uint32_t)(n);
  wuffs_private_impl__high_prec_dec__trim(h);
}

static void  //
wuffs_private_impl__high_prec_dec__round_up(
    wuffs_private_impl__high_prec_dec* h,
    int32_t n) {
  if ((n < 0) || (h->num_digits <= (uint32_t)n)) {
    return;
  }

  for (n--; n >= 0; n--) {
    if (h->digits[n] < 9) {
      h->digits[n]++;
      h->num_digits = (uint32_t)(n + 1);
      return;
    }
  }

  // The number is all 9s. Change to a single 1 and adjust the decimal point.
  h->digits[0] = 1;
  h->num_digits = 1;
  h->decimal_point++;
}

static void  //
wuffs_private_impl__high_prec_dec__round_nearest(
    wuffs_private_impl__high_prec_dec* h,
    int32_t n) {
  if ((n < 0) || (h->num_digits <= (uint32_t)n)) {
    return;
  }
  bool up = h->digits[n] >= 5;
  if ((h->digits[n] == 5) && ((n + 1) == ((int32_t)(h->num_digits)))) {
    up = h->truncated ||  //
         ((n > 0) && ((h->digits[n - 1] & 1) != 0));
  }

  if (up) {
    wuffs_private_impl__high_prec_dec__round_up(h, n);
  } else {
    wuffs_private_impl__high_prec_dec__round_down(h, n);
  }
}

static void  //
wuffs_private_impl__high_prec_dec__round_just_enough(
    wuffs_private_impl__high_prec_dec* h,
    int32_t exp2,
    uint64_t mantissa) {
  // The magic numbers 52 and 53 in this function are because IEEE 754 double
  // precision has 52 mantissa bits.
  //
  // Let f be the floating point number represented by exp2 and mantissa (and
  // also the number in h): the number (mantissa * (2 ** (exp2 - 52))).
  //
  // If f is zero or a small integer, we can return early.
  if ((mantissa == 0) ||
      ((exp2 < 53) && (h->decimal_point >= ((int32_t)(h->num_digits))))) {
    return;
  }

  // The smallest normal f has an exp2 of -1022 and a mantissa of (1 << 52).
  // Subnormal numbers have the same exp2 but a smaller mantissa.
  static const int32_t min_incl_normal_exp2 = -1022;
  static const uint64_t min_incl_normal_mantissa = 0x0010000000000000ul;

  // Compute lower and upper bounds such that any number between them (possibly
  // inclusive) will round to f. First, the lower bound. Our number f is:
  //   ((mantissa + 0)         * (2 ** (  exp2 - 52)))
  //
  // The next lowest floating point number is:
  //   ((mantissa - 1)         * (2 ** (  exp2 - 52)))
  // unless (mantissa - 1) drops the (1 << 52) bit and exp2 is not the
  // min_incl_normal_exp2. Either way, call it:
  //   ((l_mantissa)           * (2 ** (l_exp2 - 52)))
  //
  // The lower bound is halfway between them (noting that 52 became 53):
  //   (((2 * l_mantissa) + 1) * (2 ** (l_exp2 - 53)))
  int32_t l_exp2 = exp2;
  uint64_t l_mantissa = mantissa - 1;
  if ((exp2 > min_incl_normal_exp2) && (mantissa <= min_incl_normal_mantissa)) {
    l_exp2 = exp2 - 1;
    l_mantissa = (2 * mantissa) - 1;
  }
  wuffs_private_impl__high_prec_dec lower;
  wuffs_private_impl__high_prec_dec__assign(&lower, (2 * l_mantissa) + 1,
                                            false);
  wuffs_private_impl__high_prec_dec__lshift(&lower, l_exp2 - 53);

  // Next, the upper bound. Our number f is:
  //   ((mantissa + 0)       * (2 ** (exp2 - 52)))
  //
  // The next highest floating point number is:
  //   ((mantissa + 1)       * (2 ** (exp2 - 52)))
  //
  // The upper bound is halfway between them (noting that 52 became 53):
  //   (((2 * mantissa) + 1) * (2 ** (exp2 - 53)))
  wuffs_private_impl__high_prec_dec upper;
  wuffs_private_impl__high_prec_dec__assign(&upper, (2 * mantissa) + 1, false);
  wuffs_private_impl__high_prec_dec__lshift(&upper, exp2 - 53);

  // The lower and upper bounds are possible outputs only if the original
  // mantissa is even, so that IEEE round-to-even would round to the original
  // mantissa and not its neighbors.
  bool inclusive = (mantissa & 1) == 0;

  // As we walk the digits, we want to know whether rounding up would fall
  // within the upper bound. This is tracked by upper_delta:
  //  - When -1, the digits of h and upper are the same so far.
  //  - When +0, we saw a difference of 1 between h and upper on a previous
  //    digit and subsequently only 9s for h and 0s for upper. Thus, rounding
  //    up may fall outside of the bound if !inclusive.
  //  - When +1, the difference is greater than 1 and we know that rounding up
  //    falls within the bound.
  //
  // This is a state machine with three states. The numerical value for each
  // state (-1, +0 or +1) isn't important, other than their order.
  int upper_delta = -1;

  // We can now figure out the shortest number of digits required. Walk the
  // digits until h has distinguished itself from lower or upper.
  //
  // The zi and zd variables are indexes and digits, for z in l (lower), h (the
  // number) and u (upper).
  //
  // The lower, h and upper numbers may have their decimal points at different
  // places. In this case, upper is the longest, so we iterate ui starting from
  // 0 and iterate li and hi starting from either 0 or -1.
  int32_t ui = 0;
  for (;; ui++) {
    // Calculate hd, the middle number's digit.
    int32_t hi = ui - upper.decimal_point + h->decimal_point;
    if (hi >= ((int32_t)(h->num_digits))) {
      break;
    }
    uint8_t hd = (((uint32_t)hi) < h->num_digits) ? h->digits[hi] : 0;

    // Calculate ld, the lower bound's digit.
    int32_t li = ui - upper.decimal_point + lower.decimal_point;
    uint8_t ld = (((uint32_t)li) < lower.num_digits) ? lower.digits[li] : 0;

    // We can round down (truncate) if lower has a different digit than h or if
    // lower is inclusive and is exactly the result of rounding down (i.e. we
    // have reached the final digit of lower).
    bool can_round_down =
        (ld != hd) ||  //
        (inclusive && ((li + 1) == ((int32_t)(lower.num_digits))));

    // Calculate ud, the upper bound's digit, and update upper_delta.
    uint8_t ud = (((uint32_t)ui) < upper.num_digits) ? upper.digits[ui] : 0;
    if (upper_delta < 0) {
      if ((hd + 1) < ud) {
        // For example:
        // h     = 12345???
        // upper = 12347???
        upper_delta = +1;
      } else if (hd != ud) {
        // For example:
        // h     = 12345???
        // upper = 12346???
        upper_delta = +0;
      }
    } else if (upper_delta == 0) {
      if ((hd != 9) || (ud != 0)) {
        // For example:
        // h     = 1234598?
        // upper = 1234600?
        upper_delta = +1;
      }
    }

    // We can round up if upper has a different digit than h and either upper
    // is inclusive or upper is bigger than the result of rounding up.
    bool can_round_up =
        (upper_delta > 0) ||    //
        ((upper_delta == 0) &&  //
         (inclusive || ((ui + 1) < ((int32_t)(upper.num_digits)))));

    // If we can round either way, round to nearest. If we can round only one
    // way, do it. If we can't round, continue the loop.
    if (can_round_down) {
      if (can_round_up) {
        wuffs_private_impl__high_prec_dec__round_nearest(h, hi + 1);
        return;
      } else {
        wuffs_private_impl__high_prec_dec__round_down(h, hi + 1);
        return;
      }
    } else {
      if (can_round_up) {
        wuffs_private_impl__high_prec_dec__round_up(h, hi + 1);
        return;
      }
    }
  }
}

// --------

// wuffs_private_impl__parse_number_f64_eisel_lemire produces the IEEE 754
// double-precision value for an exact mantissa and base-10 exponent. For
// example:
//  - when parsing "12345.678e+02", man is 12345678 and exp10 is -1.
//  - when parsing "-12", man is 12 and exp10 is 0. Processing the leading
//    minus sign is the responsibility of the caller, not this function.
//
// On success, it returns a non-negative int64_t such that the low 63 bits hold
// the 11-bit exponent and 52-bit mantissa.
//
// On failure, it returns a negative value.
//
// The algorithm is based on an original idea by Michael Eisel that was refined
// by Daniel Lemire. See
// https://lemire.me/blog/2020/03/10/fast-float-parsing-in-practice/
// and
// https://nigeltao.github.io/blog/2020/eisel-lemire.html
//
// Preconditions:
//  - man is non-zero.
//  - exp10 is in the range [-307 ..= 288], the same range of the
//    wuffs_private_impl__powers_of_10 array.
//
// The exp10 range (and the fact that man is in the range [1 ..= UINT64_MAX],
// approximately [1 ..= 1.85e+19]) means that (man * (10 ** exp10)) is in the
// range [1e-307 ..= 1.85e+307]. This is entirely within the range of normal
// (neither subnormal nor non-finite) f64 values: DBL_MIN and DBL_MAX are
// approximately 2.23e–308 and 1.80e+308.
static int64_t  //
wuffs_private_impl__parse_number_f64_eisel_lemire(uint64_t man, int32_t exp10) {
  // Look up the (possibly truncated) base-2 representation of (10 ** exp10).
  // The look-up table was constructed so that it is already normalized: the
  // table entry's mantissa's MSB (most significant bit) is on.
  const uint64_t* po10 = &wuffs_private_impl__powers_of_10[exp10 + 307][0];

  // Normalize the man argument. The (man != 0) precondition means that a
  // non-zero bit exists.
  uint32_t clz = wuffs_base__count_leading_zeroes_u64(man);
  man <<= clz;

  // Calculate the return value's base-2 exponent. We might tweak it by ±1
  // later, but its initial value comes from a linear scaling of exp10,
  // converting from power-of-10 to power-of-2, and adjusting by clz.
  //
  // The magic constants are:
  //  - 1087 = 1023 + 64. The 1023 is the f64 exponent bias. The 64 is because
  //    the look-up table uses 64-bit mantissas.
  //  - 217706 is such that the ratio 217706 / 65536 ≈ 3.321930 is close enough
  //    (over the practical range of exp10) to log(10) / log(2) ≈ 3.321928.
  //  - 65536 = 1<<16 is arbitrary but a power of 2, so division is a shift.
  //
  // Equality of the linearly-scaled value and the actual power-of-2, over the
  // range of exp10 arguments that this function accepts, is confirmed by
  // script/print-mpb-powers-of-10.go
  uint64_t ret_exp2 =
      ((uint64_t)(((217706 * exp10) >> 16) + 1087)) - ((uint64_t)clz);

  // Multiply the two mantissas. Normalization means that both mantissas are at
  // least (1<<63), so the 128-bit product must be at least (1<<126). The high
  // 64 bits of the product, x_hi, must therefore be at least (1<<62).
  //
  // As a consequence, x_hi has either 0 or 1 leading zeroes. Shifting x_hi
  // right by either 9 or 10 bits (depending on x_hi's MSB) will therefore
  // leave the top 10 MSBs (bits 54 ..= 63) off and the 11th MSB (bit 53) on.
  wuffs_base__multiply_u64__output x = wuffs_base__multiply_u64(man, po10[1]);
  uint64_t x_hi = x.hi;
  uint64_t x_lo = x.lo;

  // Before we shift right by at least 9 bits, recall that the look-up table
  // entry was possibly truncated. We have so far only calculated a lower bound
  // for the product (man * e), where e is (10 ** exp10). The upper bound would
  // add a further (man * 1) to the 128-bit product, which overflows the lower
  // 64-bit limb if ((x_lo + man) < man).
  //
  // If overflow occurs, that adds 1 to x_hi. Since we're about to shift right
  // by at least 9 bits, that carried 1 can be ignored unless the higher 64-bit
  // limb's low 9 bits are all on.
  //
  // For example, parsing "9999999999999999999" will take the if-true branch
  // here, since:
  //  - x_hi = 0x4563918244F3FFFF
  //  - x_lo = 0x8000000000000000
  //  - man  = 0x8AC7230489E7FFFF
  if (((x_hi & 0x1FF) == 0x1FF) && ((x_lo + man) < man)) {
    // Refine our calculation of (man * e). Before, our approximation of e used
    // a "low resolution" 64-bit mantissa. Now use a "high resolution" 128-bit
    // mantissa. We've already calculated x = (man * bits_0_to_63_incl_of_e).
    // Now calculate y = (man * bits_64_to_127_incl_of_e).
    wuffs_base__multiply_u64__output y = wuffs_base__multiply_u64(man, po10[0]);
    uint64_t y_hi = y.hi;
    uint64_t y_lo = y.lo;

    // Merge the 128-bit x and 128-bit y, which overlap by 64 bits, to
    // calculate the 192-bit product of the 64-bit man by the 128-bit e.
    // As we exit this if-block, we only care about the high 128 bits
    // (merged_hi and merged_lo) of that 192-bit product.
    //
    // For example, parsing "1.234e-45" will take the if-true branch here,
    // since:
    //  - x_hi = 0x70B7E3696DB29FFF
    //  - x_lo = 0xE040000000000000
    //  - y_hi = 0x33718BBEAB0E0D7A
    //  - y_lo = 0xA880000000000000
    uint64_t merged_hi = x_hi;
    uint64_t merged_lo = x_lo + y_hi;
    if (merged_lo < x_lo) {
      merged_hi++;  // Carry the overflow bit.
    }

    // The "high resolution" approximation of e is still a lower bound. Once
    // again, see if the upper bound is large enough to produce a different
    // result. This time, if it does, give up instead of reaching for an even
    // more precise approximation to e.
    //
    // This three-part check is similar to the two-part check that guarded the
    // if block that we're now in, but it has an extra term for the middle 64
    // bits (checking that adding 1 to merged_lo would overflow).
    //
    // For example, parsing "5.9604644775390625e-8" will take the if-true
    // branch here, since:
    //  - merged_hi = 0x7FFFFFFFFFFFFFFF
    //  - merged_lo = 0xFFFFFFFFFFFFFFFF
    //  - y_lo      = 0x4DB3FFC120988200
    //  - man       = 0xD3C21BCECCEDA100
    if (((merged_hi & 0x1FF) == 0x1FF) && ((merged_lo + 1) == 0) &&
        (y_lo + man < man)) {
      return -1;
    }

    // Replace the 128-bit x with merged.
    x_hi = merged_hi;
    x_lo = merged_lo;
  }

  // As mentioned above, shifting x_hi right by either 9 or 10 bits will leave
  // the top 10 MSBs (bits 54 ..= 63) off and the 11th MSB (bit 53) on. If the
  // MSB (before shifting) was on, adjust ret_exp2 for the larger shift.
  //
  // Having bit 53 on (and higher bits off) means that ret_mantissa is a 54-bit
  // number.
  uint64_t msb = x_hi >> 63;
  uint64_t ret_mantissa = x_hi >> (msb + 9);
  ret_exp2 -= 1 ^ msb;

  // IEEE 754 rounds to-nearest with ties rounded to-even. Rounding to-even can
  // be tricky. If we're half-way between two exactly representable numbers
  // (x's low 73 bits are zero and the next 2 bits that matter are "01"), give
  // up instead of trying to pick the winner.
  //
  // Technically, we could tighten the condition by changing "73" to "73 or 74,
  // depending on msb", but a flat "73" is simpler.
  //
  // For example, parsing "1e+23" will take the if-true branch here, since:
  //  - x_hi          = 0x54B40B1F852BDA00
  //  - ret_mantissa  = 0x002A5A058FC295ED
  if ((x_lo == 0) && ((x_hi & 0x1FF) == 0) && ((ret_mantissa & 3) == 1)) {
    return -1;
  }

  // If we're not halfway then it's rounding to-nearest. Starting with a 54-bit
  // number, carry the lowest bit (bit 0) up if it's on. Regardless of whether
  // it was on or off, shifting right by one then produces a 53-bit number. If
  // carrying up overflowed, shift again.
  ret_mantissa += ret_mantissa & 1;
  ret_mantissa >>= 1;
  // This if block is equivalent to (but benchmarks slightly faster than) the
  // following branchless form:
  //    uint64_t overflow_adjustment = ret_mantissa >> 53;
  //    ret_mantissa >>= overflow_adjustment;
  //    ret_exp2 += overflow_adjustment;
  //
  // For example, parsing "7.2057594037927933e+16" will take the if-true
  // branch here, since:
  //  - x_hi          = 0x7FFFFFFFFFFFFE80
  //  - ret_mantissa  = 0x0020000000000000
  if ((ret_mantissa >> 53) > 0) {
    ret_mantissa >>= 1;
    ret_exp2++;
  }

  // Starting with a 53-bit number, IEEE 754 double-precision normal numbers
  // have an implicit mantissa bit. Mask that away and keep the low 52 bits.
  ret_mantissa &= 0x000FFFFFFFFFFFFF;

  // Pack the bits and return.
  return ((int64_t)(ret_mantissa | (ret_exp2 << 52)));
}

// --------

static wuffs_base__result_f64  //
wuffs_private_impl__parse_number_f64_special(wuffs_base__slice_u8 s,
                                             uint32_t options) {
  do {
    if (options & WUFFS_BASE__PARSE_NUMBER_FXX__REJECT_INF_AND_NAN) {
      goto fail;
    }

    uint8_t* p = s.ptr;
    uint8_t* q = s.ptr + s.len;

    for (; (p < q) && (*p == '_'); p++) {
    }
    if (p >= q) {
      goto fail;
    }

    // Parse sign.
    bool negative = false;
    do {
      if (*p == '+') {
        p++;
      } else if (*p == '-') {
        negative = true;
        p++;
      } else {
        break;
      }
      for (; (p < q) && (*p == '_'); p++) {
      }
    } while (0);
    if (p >= q) {
      goto fail;
    }

    bool nan = false;
    switch (p[0]) {
      case 'I':
      case 'i':
        if (((q - p) < 3) ||                     //
            ((p[1] != 'N') && (p[1] != 'n')) ||  //
            ((p[2] != 'F') && (p[2] != 'f'))) {
          goto fail;
        }
        p += 3;

        if ((p >= q) || (*p == '_')) {
          break;
        } else if (((q - p) < 5) ||                     //
                   ((p[0] != 'I') && (p[0] != 'i')) ||  //
                   ((p[1] != 'N') && (p[1] != 'n')) ||  //
                   ((p[2] != 'I') && (p[2] != 'i')) ||  //
                   ((p[3] != 'T') && (p[3] != 't')) ||  //
                   ((p[4] != 'Y') && (p[4] != 'y'))) {
          goto fail;
        }
        p += 5;

        if ((p >= q) || (*p == '_')) {
          break;
        }
        goto fail;

      case 'N':
      case 'n':
        if (((q - p) < 3) ||                     //
            ((p[1] != 'A') && (p[1] != 'a')) ||  //
            ((p[2] != 'N') && (p[2] != 'n'))) {
          goto fail;
        }
        p += 3;

        if ((p >= q) || (*p == '_')) {
          nan = true;
          break;
        }
        goto fail;

      default:
        goto fail;
    }

    // Finish.
    for (; (p < q) && (*p == '_'); p++) {
    }
    if (p != q) {
      goto fail;
    }
    wuffs_base__result_f64 ret;
    ret.status.repr = NULL;
    ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(
        (nan ? 0x7FFFFFFFFFFFFFFF : 0x7FF0000000000000) |
        (negative ? 0x8000000000000000 : 0));
    return ret;
  } while (0);

fail:
  do {
    wuffs_base__result_f64 ret;
    ret.status.repr = wuffs_base__error__bad_argument;
    ret.value = 0;
    return ret;
  } while (0);
}

WUFFS_BASE__MAYBE_STATIC wuffs_base__result_f64  //
wuffs_private_impl__high_prec_dec__to_f64(wuffs_private_impl__high_prec_dec* h,
                                          uint32_t options) {
  do {
    // powers converts decimal powers of 10 to binary powers of 2. For example,
    // (10000 >> 13) is 1. It stops before the elements exceed 60, also known
    // as WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL.
    //
    // This rounds down (1<<13 is a lower bound for 1e4). Adding 1 to the array
    // element value rounds up (1<<14 is an upper bound for 1e4) while staying
    // at or below WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL.
    //
    // When starting in the range [1e+1 .. 1e+2] (i.e. h->decimal_point == +2),
    // powers[2] == 6 and so:
    //  - Right shifting by 6+0 produces the range [10/64 .. 100/64] =
    //    [0.156250 .. 1.56250]. The resultant h->decimal_point is +0 or +1.
    //  - Right shifting by 6+1 produces the range [10/128 .. 100/128] =
    //    [0.078125 .. 0.78125]. The resultant h->decimal_point is -1 or -0.
    //
    // When starting in the range [1e-3 .. 1e-2] (i.e. h->decimal_point == -2),
    // powers[2] == 6 and so:
    //  - Left shifting by 6+0 produces the range [0.001*64 .. 0.01*64] =
    //    [0.064 .. 0.64]. The resultant h->decimal_point is -1 or -0.
    //  - Left shifting by 6+1 produces the range [0.001*128 .. 0.01*128] =
    //    [0.128 .. 1.28]. The resultant h->decimal_point is +0 or +1.
    //
    // Thus, when targeting h->decimal_point being +0 or +1, use (powers[n]+0)
    // when right shifting but (powers[n]+1) when left shifting.
    static const uint32_t num_powers = 19;
    static const uint8_t powers[19] = {
        0,  3,  6,  9,  13, 16, 19, 23, 26, 29,  //
        33, 36, 39, 43, 46, 49, 53, 56, 59,      //
    };

    // Handle zero and obvious extremes. The largest and smallest positive
    // finite f64 values are approximately 1.8e+308 and 4.9e-324.
    if ((h->num_digits == 0) || (h->decimal_point < -326)) {
      goto zero;
    } else if (h->decimal_point > 310) {
      goto infinity;
    }

    // Try the fast Eisel-Lemire algorithm again. Calculating the (man, exp10)
    // pair from the high_prec_dec h is more correct but slower than the
    // approach taken in wuffs_base__parse_number_f64. The latter is optimized
    // for the common cases (e.g. assuming no underscores or a leading '+'
    // sign) rather than the full set of cases allowed by the Wuffs API.
    //
    // When we have 19 or fewer mantissa digits, run Eisel-Lemire once (trying
    // for an exact result). When we have more than 19 mantissa digits, run it
    // twice to get a lower and upper bound. We still have an exact result
    // (within f64's rounding margin) if both bounds are equal (and valid).
    uint32_t i_max = h->num_digits;
    if (i_max > 19) {
      i_max = 19;
    }
    int32_t exp10 = h->decimal_point - ((int32_t)i_max);
    if ((-307 <= exp10) && (exp10 <= 288)) {
      uint64_t man = 0;
      uint32_t i;
      for (i = 0; i < i_max; i++) {
        man = (10 * man) + h->digits[i];
      }
      while (man != 0) {  // The 'while' is just an 'if' that we can 'break'.
        int64_t r0 =
            wuffs_private_impl__parse_number_f64_eisel_lemire(man + 0, exp10);
        if (r0 < 0) {
          break;
        } else if (h->num_digits > 19) {
          int64_t r1 =
              wuffs_private_impl__parse_number_f64_eisel_lemire(man + 1, exp10);
          if (r1 != r0) {
            break;
          }
        }
        wuffs_base__result_f64 ret;
        ret.status.repr = NULL;
        ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(
            ((uint64_t)r0) | (((uint64_t)(h->negative)) << 63));
        return ret;
      }
    }

    // When Eisel-Lemire fails, fall back to Simple Decimal Conversion. See
    // https://nigeltao.github.io/blog/2020/parse-number-f64-simple.html
    //
    // Scale by powers of 2 until we're in the range [0.1 .. 10]. Equivalently,
    // that h->decimal_point is +0 or +1.
    //
    // First we shift right while at or above 10...
    const int32_t f64_bias = -1023;
    int32_t exp2 = 0;
    while (h->decimal_point > 1) {
      uint32_t n = (uint32_t)(+h->decimal_point);
      uint32_t shift = (n < num_powers)
                           ? powers[n]
                           : WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL;

      wuffs_private_impl__high_prec_dec__small_rshift(h, shift);
      if (h->decimal_point < -WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE) {
        goto zero;
      }
      exp2 += (int32_t)shift;
    }
    // ...then we shift left while below 0.1.
    while (h->decimal_point < 0) {
      uint32_t shift;
      uint32_t n = (uint32_t)(-h->decimal_point);
      shift = (n < num_powers)
                  // The +1 is per "when targeting h->decimal_point being +0 or
                  // +1... when left shifting" in the powers comment above.
                  ? (powers[n] + 1u)
                  : WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL;

      wuffs_private_impl__high_prec_dec__small_lshift(h, shift);
      if (h->decimal_point > +WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE) {
        goto infinity;
      }
      exp2 -= (int32_t)shift;
    }

    // To get from "in the range [0.1 .. 10]" to "in the range [1 .. 2]" (which
    // will give us our exponent in base-2), the mantissa's first 3 digits will
    // determine the final left shift, equal to 52 (the number of explicit f64
    // bits) plus an additional adjustment.
    int man3 = (100 * h->digits[0]) +
               ((h->num_digits > 1) ? (10 * h->digits[1]) : 0) +
               ((h->num_digits > 2) ? h->digits[2] : 0);
    int32_t additional_lshift = 0;
    if (h->decimal_point == 0) {  // The value is in [0.1 .. 1].
      if (man3 < 125) {
        additional_lshift = +4;
      } else if (man3 < 250) {
        additional_lshift = +3;
      } else if (man3 < 500) {
        additional_lshift = +2;
      } else {
        additional_lshift = +1;
      }
    } else {  // The value is in [1 .. 10].
      if (man3 < 200) {
        additional_lshift = -0;
      } else if (man3 < 400) {
        additional_lshift = -1;
      } else if (man3 < 800) {
        additional_lshift = -2;
      } else {
        additional_lshift = -3;
      }
    }
    exp2 -= additional_lshift;
    uint32_t final_lshift = (uint32_t)(52 + additional_lshift);

    // The minimum normal exponent is (f64_bias + 1).
    while ((f64_bias + 1) > exp2) {
      uint32_t n = (uint32_t)((f64_bias + 1) - exp2);
      if (n > WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL) {
        n = WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL;
      }
      wuffs_private_impl__high_prec_dec__small_rshift(h, n);
      exp2 += (int32_t)n;
    }

    // Check for overflow.
    if ((exp2 - f64_bias) >= 0x07FF) {  // (1 << 11) - 1.
      goto infinity;
    }

    // Extract 53 bits for the mantissa (in base-2).
    wuffs_private_impl__high_prec_dec__small_lshift(h, final_lshift);
    uint64_t man2 = wuffs_private_impl__high_prec_dec__rounded_integer(h);

    // Rounding might have added one bit. If so, shift and re-check overflow.
    if ((man2 >> 53) != 0) {
      man2 >>= 1;
      exp2++;
      if ((exp2 - f64_bias) >= 0x07FF) {  // (1 << 11) - 1.
        goto infinity;
      }
    }

    // Handle subnormal numbers.
    if ((man2 >> 52) == 0) {
      exp2 = f64_bias;
    }

    // Pack the bits and return.
    uint64_t exp2_bits =
        (uint64_t)((exp2 - f64_bias) & 0x07FF);              // (1 << 11) - 1.
    uint64_t bits = (man2 & 0x000FFFFFFFFFFFFF) |            // (1 << 52) - 1.
                    (exp2_bits << 52) |                      //
                    (h->negative ? 0x8000000000000000 : 0);  // (1 << 63).

    wuffs_base__result_f64 ret;
    ret.status.repr = NULL;
    ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(bits);
    return ret;
  } while (0);

zero:
  do {
    uint64_t bits = h->negative ? 0x8000000000000000 : 0;

    wuffs_base__result_f64 ret;
    ret.status.repr = NULL;
    ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(bits);
    return ret;
  } while (0);

infinity:
  do {
    if (options & WUFFS_BASE__PARSE_NUMBER_FXX__REJECT_INF_AND_NAN) {
      wuffs_base__result_f64 ret;
      ret.status.repr = wuffs_base__error__bad_argument;
      ret.value = 0;
      return ret;
    }

    uint64_t bits = h->negative ? 0xFFF0000000000000 : 0x7FF0000000000000;

    wuffs_base__result_f64 ret;
    ret.status.repr = NULL;
    ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(bits);
    return ret;
  } while (0);
}

static inline bool  //
wuffs_private_impl__is_decimal_digit(uint8_t c) {
  return ('0' <= c) && (c <= '9');
}

WUFFS_BASE__MAYBE_STATIC wuffs_base__result_f64  //
wuffs_base__parse_number_f64(wuffs_base__slice_u8 s, uint32_t options) {
  // In practice, almost all "dd.ddddE±xxx" numbers can be represented
  // losslessly by a uint64_t mantissa "dddddd" and an int32_t base-10
  // exponent, adjusting "xxx" for the position (if present) of the decimal
  // separator '.' or ','.
  //
  // This (u64 man, i32 exp10) data structure is superficially similar to the
  // "Do It Yourself Floating Point" type from Loitsch (†), but the exponent
  // here is base-10, not base-2.
  //
  // If s's number fits in a (man, exp10), parse that pair with the
  // Eisel-Lemire algorithm. If not, or if Eisel-Lemire fails, parsing s with
  // the fallback algorithm is slower but comprehensive.
  //
  // † "Printing Floating-Point Numbers Quickly and Accurately with Integers"
  // (https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf).
  // Florian Loitsch is also the primary contributor to
  // https://github.com/google/double-conversion
  do {
    // Calculating that (man, exp10) pair needs to stay within s's bounds.
    // Provided that s isn't extremely long, work on a NUL-terminated copy of
    // s's contents. The NUL byte isn't a valid part of "±dd.ddddE±xxx".
    //
    // As the pointer p walks the contents, it's faster to repeatedly check "is
    // *p a valid digit" than "is p within bounds and *p a valid digit".
    if (s.len >= 256) {
      goto fallback;
    }
    uint8_t z[256];
    memcpy(&z[0], s.ptr, s.len);
    z[s.len] = 0;
    const uint8_t* p = &z[0];

    // Look for a leading minus sign. Technically, we could also look for an
    // optional plus sign, but the "script/process-json-numbers.c with -p"
    // benchmark is noticably slower if we do. It's optional and, in practice,
    // usually absent. Let the fallback catch it.
    bool negative = (*p == '-');
    if (negative) {
      p++;
    }

    // After walking "dd.dddd", comparing p later with p now will produce the
    // number of "d"s and "."s.
    const uint8_t* const start_of_digits_ptr = p;

    // Walk the "d"s before a '.', 'E', NUL byte, etc. If it starts with '0',
    // it must be a single '0'. If it starts with a non-zero decimal digit, it
    // can be a sequence of decimal digits.
    //
    // Update the man variable during the walk. It's OK if man overflows now.
    // We'll detect that later.
    uint64_t man;
    if (*p == '0') {
      man = 0;
      p++;
      if (wuffs_private_impl__is_decimal_digit(*p)) {
        goto fallback;
      }
    } else if (wuffs_private_impl__is_decimal_digit(*p)) {
      man = ((uint8_t)(*p - '0'));
      p++;
      for (; wuffs_private_impl__is_decimal_digit(*p); p++) {
        man = (10 * man) + ((uint8_t)(*p - '0'));
      }
    } else {
      goto fallback;
    }

    // Walk the "d"s after the optional decimal separator ('.' or ','),
    // updating the man and exp10 variables.
    int32_t exp10 = 0;
    if (*p ==
        ((options & WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
             ? ','
             : '.')) {
      p++;
      const uint8_t* first_after_separator_ptr = p;
      if (!wuffs_private_impl__is_decimal_digit(*p)) {
        goto fallback;
      }
      man = (10 * man) + ((uint8_t)(*p - '0'));
      p++;
      for (; wuffs_private_impl__is_decimal_digit(*p); p++) {
        man = (10 * man) + ((uint8_t)(*p - '0'));
      }
      exp10 = ((int32_t)(first_after_separator_ptr - p));
    }

    // Count the number of digits:
    //  - for an input of "314159",  digit_count is 6.
    //  - for an input of "3.14159", digit_count is 7.
    //
    // This is off-by-one if there is a decimal separator. That's OK for now.
    // We'll correct for that later. The "script/process-json-numbers.c with
    // -p" benchmark is noticably slower if we try to correct for that now.
    uint32_t digit_count = (uint32_t)(p - start_of_digits_ptr);

    // Update exp10 for the optional exponent, starting with 'E' or 'e'.
    if ((*p | 0x20) == 'e') {
      p++;
      int32_t exp_sign = +1;
      if (*p == '-') {
        p++;
        exp_sign = -1;
      } else if (*p == '+') {
        p++;
      }
      if (!wuffs_private_impl__is_decimal_digit(*p)) {
        goto fallback;
      }
      int32_t exp_num = ((uint8_t)(*p - '0'));
      p++;
      // The rest of the exp_num walking has a peculiar control flow but, once
      // again, the "script/process-json-numbers.c with -p" benchmark is
      // sensitive to alternative formulations.
      if (wuffs_private_impl__is_decimal_digit(*p)) {
        exp_num = (10 * exp_num) + ((uint8_t)(*p - '0'));
        p++;
      }
      if (wuffs_private_impl__is_decimal_digit(*p)) {
        exp_num = (10 * exp_num) + ((uint8_t)(*p - '0'));
        p++;
      }
      while (wuffs_private_impl__is_decimal_digit(*p)) {
        if (exp_num > 0x1000000) {
          goto fallback;
        }
        exp_num = (10 * exp_num) + ((uint8_t)(*p - '0'));
        p++;
      }
      exp10 += exp_sign * exp_num;
    }

    // The Wuffs API is that the original slice has no trailing data. It also
    // allows underscores, which we don't catch here but the fallback should.
    if (p != &z[s.len]) {
      goto fallback;
    }

    // Check that the uint64_t typed man variable has not overflowed, based on
    // digit_count.
    //
    // For reference:
    //   - (1 << 63) is  9223372036854775808, which has 19 decimal digits.
    //   - (1 << 64) is 18446744073709551616, which has 20 decimal digits.
    //   - 19 nines,  9999999999999999999, is  0x8AC7230489E7FFFF, which has 64
    //     bits and 16 hexadecimal digits.
    //   - 20 nines, 99999999999999999999, is 0x56BC75E2D630FFFFF, which has 67
    //     bits and 17 hexadecimal digits.
    if (digit_count > 19) {
      // Even if we have more than 19 pseudo-digits, it's not yet definitely an
      // overflow. Recall that digit_count might be off-by-one (too large) if
      // there's a decimal separator. It will also over-report the number of
      // meaningful digits if the input looks something like "0.000dddExxx".
      //
      // We adjust by the number of leading '0's and '.'s and re-compare to 19.
      // Once again, technically, we could skip ','s too, but that perturbs the
      // "script/process-json-numbers.c with -p" benchmark.
      const uint8_t* q = start_of_digits_ptr;
      for (; (*q == '0') || (*q == '.'); q++) {
      }
      digit_count -= (uint32_t)(q - start_of_digits_ptr);
      if (digit_count > 19) {
        goto fallback;
      }
    }

    // The wuffs_private_impl__parse_number_f64_eisel_lemire preconditions
    // include that exp10 is in the range [-307 ..= 288].
    if ((exp10 < -307) || (288 < exp10)) {
      goto fallback;
    }

#if PK_FLOATCONV_HAS_EXACT_DOUBLE_ARITHMETIC  // [pocketpy] Deviation 4.
    // If both man and (10 ** exp10) are exactly representable by a double, we
    // don't need to run the Eisel-Lemire algorithm.
    if ((-22 <= exp10) && (exp10 <= 22) && ((man >> 53) == 0)) {
      double d = (double)man;
      if (exp10 >= 0) {
        d *= wuffs_private_impl__f64_powers_of_10[+exp10];
      } else {
        d /= wuffs_private_impl__f64_powers_of_10[-exp10];
      }
      wuffs_base__result_f64 ret;
      ret.status.repr = NULL;
      ret.value = negative ? -d : +d;
      return ret;
    }
#endif

    // The wuffs_private_impl__parse_number_f64_eisel_lemire preconditions
    // include that man is non-zero. Parsing "0" should be caught by the "If
    // both man and (10 ** exp10)" above, but "0e99" might not.
    if (man == 0) {
      goto fallback;
    }

    // Our man and exp10 are in range. Run the Eisel-Lemire algorithm.
    int64_t r = wuffs_private_impl__parse_number_f64_eisel_lemire(man, exp10);
    if (r < 0) {
      goto fallback;
    }
    wuffs_base__result_f64 ret;
    ret.status.repr = NULL;
    ret.value = wuffs_base__ieee_754_bit_representation__from_u64_to_f64(
        ((uint64_t)r) | (((uint64_t)negative) << 63));
    return ret;
  } while (0);

fallback:
  do {
    wuffs_private_impl__high_prec_dec h;
    wuffs_base__status status =
        wuffs_private_impl__high_prec_dec__parse(&h, s, options);
    if (status.repr) {
      return wuffs_private_impl__parse_number_f64_special(s, options);
    }
    return wuffs_private_impl__high_prec_dec__to_f64(&h, options);
  } while (0);
}

// --------

static inline size_t  //
wuffs_private_impl__render_inf(wuffs_base__slice_u8 dst,
                               bool neg,
                               uint32_t options) {
  if (neg) {
    if (dst.len < 4) {
      return 0;
    }
    wuffs_base__poke_u32le__no_bounds_check(dst.ptr, 0x666E492D);  // '-Inf'le.
    return 4;
  }

  if (options & WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN) {
    if (dst.len < 4) {
      return 0;
    }
    wuffs_base__poke_u32le__no_bounds_check(dst.ptr, 0x666E492B);  // '+Inf'le.
    return 4;
  }

  if (dst.len < 3) {
    return 0;
  }
  wuffs_base__poke_u24le__no_bounds_check(dst.ptr, 0x666E49);  // 'Inf'le.
  return 3;
}

static inline size_t  //
wuffs_private_impl__render_nan(wuffs_base__slice_u8 dst) {
  if (dst.len < 3) {
    return 0;
  }
  wuffs_base__poke_u24le__no_bounds_check(dst.ptr, 0x4E614E);  // 'NaN'le.
  return 3;
}

static size_t  //
wuffs_private_impl__high_prec_dec__render_exponent_absent(
    wuffs_base__slice_u8 dst,
    wuffs_private_impl__high_prec_dec* h,
    uint32_t precision,
    uint32_t options) {
  size_t n = (h->negative ||
              (options & WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN))
                 ? 1
                 : 0;
  if (h->decimal_point <= 0) {
    n += 1;
  } else {
    n += (size_t)(h->decimal_point);
  }
  if (precision > 0) {
    n += precision + 1;  // +1 for the '.'.
  }

  // Don't modify dst if the formatted number won't fit.
  if (n > dst.len) {
    return 0;
  }

  // Align-left or align-right.
  uint8_t* ptr = (options & WUFFS_BASE__RENDER_NUMBER_XXX__ALIGN_RIGHT)
                     ? &dst.ptr[dst.len - n]
                     : &dst.ptr[0];

  // Leading "±".
  if (h->negative) {
    *ptr++ = '-';
  } else if (options & WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN) {
    *ptr++ = '+';
  }

  // Integral digits.
  if (h->decimal_point <= 0) {
    *ptr++ = '0';
  } else {
    uint32_t m =
        wuffs_base__u32__min(h->num_digits, (uint32_t)(h->decimal_point));
    uint32_t i = 0;
    for (; i < m; i++) {
      *ptr++ = (uint8_t)('0' | h->digits[i]);
    }
    for (; i < (uint32_t)(h->decimal_point); i++) {
      *ptr++ = '0';
    }
  }

  // Separator and then fractional digits.
  if (precision > 0) {
    *ptr++ =
        (options & WUFFS_BASE__RENDER_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
            ? ','
            : '.';
    uint32_t i = 0;
    for (; i < precision; i++) {
      uint32_t j = ((uint32_t)(h->decimal_point)) + i;
      *ptr++ = (uint8_t)('0' | ((j < h->num_digits) ? h->digits[j] : 0));
    }
  }

  return n;
}

static size_t  //
wuffs_private_impl__high_prec_dec__render_exponent_present(
    wuffs_base__slice_u8 dst,
    wuffs_private_impl__high_prec_dec* h,
    uint32_t precision,
    uint32_t options) {
  int32_t exp = 0;
  if (h->num_digits > 0) {
    exp = h->decimal_point - 1;
  }
  bool negative_exp = exp < 0;
  if (negative_exp) {
    exp = -exp;
  }

  size_t n = (h->negative ||
              (options & WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN))
                 ? 4
                 : 3;  // Mininum 3 bytes: first digit and then "e±".
  if (precision > 0) {
    n += precision + 1;  // +1 for the '.'.
  }
  n += (exp < 100) ? 2 : 3;

  // Don't modify dst if the formatted number won't fit.
  if (n > dst.len) {
    return 0;
  }

  // Align-left or align-right.
  uint8_t* ptr = (options & WUFFS_BASE__RENDER_NUMBER_XXX__ALIGN_RIGHT)
                     ? &dst.ptr[dst.len - n]
                     : &dst.ptr[0];

  // Leading "±".
  if (h->negative) {
    *ptr++ = '-';
  } else if (options & WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN) {
    *ptr++ = '+';
  }

  // Integral digit.
  if (h->num_digits > 0) {
    *ptr++ = (uint8_t)('0' | h->digits[0]);
  } else {
    *ptr++ = '0';
  }

  // Separator and then fractional digits.
  if (precision > 0) {
    *ptr++ =
        (options & WUFFS_BASE__RENDER_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA)
            ? ','
            : '.';
    uint32_t i = 1;
    uint32_t j = wuffs_base__u32__min(h->num_digits, precision + 1);
    for (; i < j; i++) {
      *ptr++ = (uint8_t)('0' | h->digits[i]);
    }
    for (; i <= precision; i++) {
      *ptr++ = '0';
    }
  }

  // Exponent: "e±" and then 2 or 3 digits.
  *ptr++ = 'e';
  *ptr++ = negative_exp ? '-' : '+';
  if (exp < 10) {
    *ptr++ = '0';
    *ptr++ = (uint8_t)('0' | exp);
  } else if (exp < 100) {
    *ptr++ = (uint8_t)('0' | (exp / 10));
    *ptr++ = (uint8_t)('0' | (exp % 10));
  } else {
    int32_t e = exp / 100;
    exp -= e * 100;
    *ptr++ = (uint8_t)('0' | e);
    *ptr++ = (uint8_t)('0' | (exp / 10));
    *ptr++ = (uint8_t)('0' | (exp % 10));
  }

  return n;
}

WUFFS_BASE__MAYBE_STATIC size_t  //
wuffs_base__render_number_f64(wuffs_base__slice_u8 dst,
                              double x,
                              uint32_t precision,
                              uint32_t options) {
  // Decompose x (64 bits) into negativity (1 bit), base-2 exponent (11 bits
  // with a -1023 bias) and mantissa (52 bits).
  uint64_t bits = wuffs_base__ieee_754_bit_representation__from_f64_to_u64(x);
  bool neg = (bits >> 63) != 0;
  int32_t exp2 = ((int32_t)(bits >> 52)) & 0x7FF;
  uint64_t man = bits & 0x000FFFFFFFFFFFFFul;

  // Apply the exponent bias and set the implicit top bit of the mantissa,
  // unless x is subnormal. Also take care of Inf and NaN.
  if (exp2 == 0x7FF) {
    if (man != 0) {
      return wuffs_private_impl__render_nan(dst);
    }
    return wuffs_private_impl__render_inf(dst, neg, options);
  } else if (exp2 == 0) {
    exp2 = -1022;
  } else {
    exp2 -= 1023;
    man |= 0x0010000000000000ul;
  }

  // Ensure that precision isn't too large.
  if (precision > 4095) {
    precision = 4095;
  }

  // Convert from the (neg, exp2, man) tuple to an HPD.
  wuffs_private_impl__high_prec_dec h;
  wuffs_private_impl__high_prec_dec__assign(&h, man, neg);
  if (h.num_digits > 0) {
    wuffs_private_impl__high_prec_dec__lshift(&h,
                                              exp2 - 52);  // 52 mantissa bits.
  }

  // Handle the "%e" and "%f" formats.
  switch (options & (WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_ABSENT |
                     WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_PRESENT)) {
    case WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_ABSENT:  // The "%"f" format.
      if (options & WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION) {
        wuffs_private_impl__high_prec_dec__round_just_enough(&h, exp2, man);
        int32_t p = ((int32_t)(h.num_digits)) - h.decimal_point;
        precision = ((uint32_t)(wuffs_base__i32__max(0, p)));
      } else {
        wuffs_private_impl__high_prec_dec__round_nearest(
            &h, ((int32_t)precision) + h.decimal_point);
      }
      return wuffs_private_impl__high_prec_dec__render_exponent_absent(
          dst, &h, precision, options);

    case WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_PRESENT:  // The "%e" format.
      if (options & WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION) {
        wuffs_private_impl__high_prec_dec__round_just_enough(&h, exp2, man);
        precision = (h.num_digits > 0) ? (h.num_digits - 1) : 0;
      } else {
        wuffs_private_impl__high_prec_dec__round_nearest(
            &h, ((int32_t)precision) + 1);
      }
      return wuffs_private_impl__high_prec_dec__render_exponent_present(
          dst, &h, precision, options);
  }

  // We have the "%g" format and so precision means the number of significant
  // digits, not the number of digits after the decimal separator. Perform
  // rounding and determine whether to use "%e" or "%f".
  int32_t e_threshold = 0;
  if (options & WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION) {
    wuffs_private_impl__high_prec_dec__round_just_enough(&h, exp2, man);
    precision = h.num_digits;
    e_threshold = PK_FLOATCONV_REPR_E_THRESHOLD;  // [pocketpy] Deviation 3; was 6.
  } else {
    if (precision == 0) {
      precision = 1;
    }
    wuffs_private_impl__high_prec_dec__round_nearest(&h, ((int32_t)precision));
    e_threshold = ((int32_t)precision);
    int32_t nd = ((int32_t)(h.num_digits));
    if ((e_threshold > nd) && (nd >= h.decimal_point)) {
      e_threshold = nd;
    }
  }

  // Use the "%e" format if the exponent is large.
  int32_t e = h.decimal_point - 1;
  if ((e < -4) || (e_threshold <= e)) {
    uint32_t p = wuffs_base__u32__min(precision, h.num_digits);
    return wuffs_private_impl__high_prec_dec__render_exponent_present(
        dst, &h, (p > 0) ? (p - 1) : 0, options);
  }

  // Use the "%f" format otherwise.
  int32_t p = ((int32_t)precision);
  if (p > h.decimal_point) {
    p = ((int32_t)(h.num_digits));
  }
  precision = ((uint32_t)(wuffs_base__i32__max(0, p - h.decimal_point)));
  return wuffs_private_impl__high_prec_dec__render_exponent_absent(
      dst, &h, precision, options);
}

/* ---------------- [pocketpy] public API ----------------
 *
 * Parse options. Wuffs' defaults are stricter than C's `strtod`, so we opt
 * back in to redundant leading zeroes: `float("007")` and the literal `00.7`
 * both have to keep working.
 *
 * Underscores stay rejected. The lexer never puts one inside a number token,
 * `float("1_0")` was already an error under `strtod`, and Wuffs' rule is looser
 * than PEP 515's anyway (it would accept a leading `_`).
 *
 * Infinities and NaNs stay accepted, so `float("nan")` keeps working and
 * `1e999` keeps overflowing to `inf` the way CPython does, rather than raising.
 */
#define PK_FLOATCONV_PARSE_OPTIONS (WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_MULTIPLE_LEADING_ZEROES)

static bool pk_floatconv__is_digit(char c) { return ('0' <= c) && (c <= '9'); }

// Whether the NUL-terminated `p` starts with `word`, which must be lowercase
// ASCII. Case insensitive.
static bool pk_floatconv__starts_with(const char* p, const char* word) {
    for(; *word != '\0'; word++, p++) {
        if((*p | 0x20) != *word) return false;
    }
    return true;
}

bool c11__parse_f64(const char* data, int size, double* out) {
    if(size <= 0) return false;
    wuffs_base__slice_u8 s;
    s.ptr = (uint8_t*)data;  // The vendored parser only reads through this.
    s.len = (size_t)size;
    wuffs_base__result_f64 res = wuffs_base__parse_number_f64(s, PK_FLOATCONV_PARSE_OPTIONS);
    if(res.status.repr != NULL) return false;
    *out = res.value;
    return true;
}

double strtod1(const char* s, char** p_end) {
    const char* p = s;
    // strtod() skips leading whitespace. Spelled out rather than via isspace()
    // so that it cannot pick up a locale's extra space characters.
    while(*p == ' ' || (*p >= '\t' && *p <= '\r'))
        p++;

    const char* start = p;
    if(*p == '+' || *p == '-') p++;

    // Find the longest prefix that c11__parse_f64 will accept. It only takes
    // whole slices, so the scanning strtod() does implicitly happens here.
    const char* end;
    if((*p | 0x20) == 'i') {
        if(!pk_floatconv__starts_with(p, "inf")) goto fail;
        end = p + 3;
        if(pk_floatconv__starts_with(end, "inity")) end += 5;
    } else if((*p | 0x20) == 'n') {
        if(!pk_floatconv__starts_with(p, "nan")) goto fail;
        end = p + 3;
    } else {
        int digits = 0;
        for(; pk_floatconv__is_digit(*p); p++)
            digits++;
        if(*p == '.') {
            p++;
            for(; pk_floatconv__is_digit(*p); p++)
                digits++;
        }
        if(digits == 0) goto fail;
        end = p;
        // Like strtod(), only consume the exponent if it is well formed. In
        // "1e+" the 'e' belongs to whatever comes after the number.
        if((*p | 0x20) == 'e') {
            const char* q = p + 1;
            if(*q == '+' || *q == '-') q++;
            if(pk_floatconv__is_digit(*q)) {
                for(; pk_floatconv__is_digit(*q); q++) {}
                end = q;
            }
        }
    }

    double out;
    if(!c11__parse_f64(start, (int)(end - start), &out)) goto fail;
    if(p_end != NULL) *p_end = (char*)end;
    return out;

fail:
    if(p_end != NULL) *p_end = (char*)s;
    return 0.0;
}

int c11__f64_to_shortest(char* dst, int dst_size, double x) {
    if(dst_size <= 0) return 0;
    wuffs_base__slice_u8 s;
    s.ptr = (uint8_t*)dst;
    s.len = (size_t)dst_size;
    // Neither EXPONENT_ABSENT nor EXPONENT_PRESENT means "%g", which with
    // PK_FLOATCONV_REPR_E_THRESHOLD is CPython's repr() notation.
    return (int)wuffs_base__render_number_f64(s, x, 0,
                                              WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION);
}

int c11__f64_to_fixed(char* dst, int dst_size, double x, int precision) {
    if(dst_size <= 0) return 0;
    if(precision < 0) precision = 0;
    if(precision > C11_F64_MAX_PRECISION) precision = C11_F64_MAX_PRECISION;
    wuffs_base__slice_u8 s;
    s.ptr = (uint8_t*)dst;
    s.len = (size_t)dst_size;
    return (int)wuffs_base__render_number_f64(s, x, (uint32_t)precision,
                                              WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_ABSENT);
}



#undef WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_MULTIPLE_LEADING_ZEROES
#undef WUFFS_BASE__PARSE_NUMBER_XXX__ALLOW_UNDERSCORES
#undef WUFFS_PRIVATE_IMPL__HPD__DECIMAL_POINT__RANGE
#undef WUFFS_BASE__PARSE_NUMBER_XXX__DEFAULT_OPTIONS
#undef WUFFS_BASE__RENDER_NUMBER_XXX__DEFAULT_OPTIONS
#undef WUFFS_BASE__PARSE_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA
#undef WUFFS_PRIVATE_IMPL__HPD__DIGITS_PRECISION
#undef WUFFS_PRIVATE_IMPL__HPD__SHIFT__MAX_INCL
#undef WUFFS_BASE__RENDER_NUMBER_FXX__DECIMAL_SEPARATOR_IS_A_COMMA
#undef WUFFS_BASE__PARSE_NUMBER_FXX__REJECT_INF_AND_NAN
#undef WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_ABSENT
#undef PK_FLOATCONV_PARSE_OPTIONS
#undef WUFFS_BASE__RENDER_NUMBER_FXX__EXPONENT_PRESENT
#undef WUFFS_BASE__RENDER_NUMBER_XXX__ALIGN_RIGHT
#undef PK_FLOATCONV_REPR_E_THRESHOLD
#undef PK_FLOATCONV_HAS_EXACT_DOUBLE_ARITHMETIC
#undef WUFFS_BASE__MAYBE_STATIC
#undef WUFFS_BASE__RENDER_NUMBER_FXX__JUST_ENOUGH_PRECISION
#undef WUFFS_BASE__RENDER_NUMBER_XXX__LEADING_PLUS_SIGN
