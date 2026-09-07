#pragma once

#include <stdbool.h>

/* Deterministic conversions between `double` and its decimal text form.
 *
 * The C library's `strtod`, `snprintf("%g")` and friends are not usable when
 * bit-identical results across platforms are required: they are affected by
 * the current locale, and several libc implementations are not correctly
 * rounded. The routines below are ported from Wuffs and only use integer
 * arithmetic, so they produce the same bits on every IEEE-754 platform.
 *
 * See `src/common/floatconv.c` for provenance and license.
 */

/* Buffer size that `c11__f64_to_shortest` never exceeds. */
#define C11_F64_SHORTEST_BUF_SIZE 32

/* The largest precision `c11__f64_to_fixed` honours. Anything larger is
 * clamped, matching the upstream Wuffs limit. */
#define C11_F64_MAX_PRECISION 4095

/* Buffer size that `c11__f64_to_fixed` never exceeds for a given precision:
 * a sign, up to 309 integral digits, a '.', `precision` fractional digits and
 * a byte of slack. */
#define C11_F64_FIXED_BUF_SIZE(precision) (312 + (precision))

/* A drop-in replacement for `strtod`, minus hexadecimal floats and minus any
 * locale sensitivity. Skips leading whitespace, parses the longest prefix of
 * `s` that forms a decimal float (including `inf`, `infinity` and `nan`, case
 * insensitive) and, if `p_end` is non-NULL, stores the first unconsumed
 * character there. Returns 0.0 and sets `*p_end` to `s` if nothing parses. */
double strtod1(const char* s, char** p_end);

/* Parses the whole of `[data, data + size)` as a decimal float. Returns false
 * without touching `*out` unless every byte is consumed. */
bool c11__parse_f64(const char* data, int size, double* out);

/* Writes `x` using the fewest digits that still round-trip back to `x`, in the
 * notation CPython's `repr()` picks. `x` must be finite. Returns the number of
 * bytes written, or 0 if `dst_size` is too small. */
int c11__f64_to_shortest(char* dst, int dst_size, double x);

/* Writes `x` with exactly `precision` digits after the decimal point, i.e.
 * `"%.*f"`. `x` must be finite. Returns the number of bytes written, or 0 if
 * `dst_size` is too small. */
int c11__f64_to_fixed(char* dst, int dst_size, double x, int precision);
