# Fixed binary64 mathematical kernels

pocketpy vendors the C algorithms from [OpenLibm v0.8.8](https://github.com/JuliaMath/openlibm/tree/5fe399749f9276eaa0b8403e507470da05cbbb3f),
commit `5fe399749f9276eaa0b8403e507470da05cbbb3f`, in
`src/common/dmath_openlibm.c`. There is no build-time download or runtime libm
dependency for these functions.

The imported files are `src/e_exp.c`, `src/s_exp2.c`, `src/e_log.c`,
`src/e_log2.c`, `src/e_log10.c`, `src/e_pow.c`, `src/k_log.h`, and
`src/s_scalbn.c`, plus `src/s_sin.c`, `src/s_cos.c`, `src/s_tan.c`,
`src/s_sincos.c`, `src/k_sin.c`, `src/k_cos.c`, `src/k_tan.c`,
`src/e_rem_pio2.c`, `src/k_rem_pio2.c`, `src/s_floor.c`, `src/s_ceil.c`, and
`src/s_trunc.c`.
`SHA256SUMS` records the unmodified upstream files. Copyright
and permission notices remain in the C source and in `LICENSE.txt`; retain
the latter with binary distributions. No OpenLibm assembly, long-double code,
build system, or test suite is included.

Local adaptations:

- Prefix private functions and macros; move constants into function scope so
  ordinary, unity, and amalgamated builds use the same algorithms.
- Access binary64 words through `memcpy` and integer shifts, without depending
  on the machine's byte order or violating aliasing rules.
- Replace signed shifts and out-of-range integer conversions with defined
  arithmetic. Bound the private `scalbn` exponent before addition.
- Use pocketpy's correctly rounded `sqrt` and bitwise `fabs`/`copysign` helpers.
- Canonicalize every NaN result to `0x7ff8000000000000`. Preserve signed zeros.
- Rely on the build configuration to disable FP contraction; no local FP pragmas.
- Implement `dmath_exp10(x)` through the fixed `pow(10, x)` kernel and
  `dmath_log_base(x, base)` through the fixed natural logarithm kernels.
- Specialize large-angle reduction to binary64 (upstream `prec = 1`), retaining
  the first 66 words of the `2/pi` table and the adaptive recomputation steps.
  The full finite binary64 range, including `DBL_MAX`, is supported.
- Reuse the private `scalbn` and a bit-mask adaptation of `floor`; neither
  helper calls the host libm. Public `dmath_floor` shares the same kernel.
- Share the sine/cosine kernels and one reduction in `dmath_sincos`, using the
  same tiny-input cutoffs as the separate functions so results agree bitwise.
  Tangent uses its dedicated polynomial and compensated reciprocal kernel.
- Implement `ceil`/`floor`/`trunc` using the upstream exponent/mask algorithms
  with one unsigned 64-bit word in place of two 32-bit words. These kernels
  never cast the floating value to an integer and omit upstream operations
  used solely to raise the inexact flag. Finite results are exact across the
  complete binary64 range and independent of the host rounding mode.

The coefficients, table entries, reduction steps, and floating-point operation
order are pinned. Upstream updates must be reviewed as numerical behavior
changes; do not regenerate expected results just to make a failing test pass.

## Determinism contract

Given identical binary64 input bits, finite results (including signed zeros)
have identical bits on supported builds with IEEE 754 binary64 arithmetic, round-to-nearest-even,
and gradual underflow. The embedding application must leave FTZ/DAZ disabled
and must not change the thread's rounding mode while evaluating Python code.
The library does not change the host floating-point environment. Hardware
exception traps must be disabled. NaNs are compared by classification and
infinities by classification/sign; nonfinite bit patterns, `errno`, NaN payload
propagation, and FP exception flags are not part of the contract. The OpenLibm
wrappers currently canonicalize NaNs as an implementation detail.

Use GCC/Clang with `-fno-fast-math -ffp-contract=off`, or MSVC with `/fp:precise`.
The deterministic CMake configuration supplies these options. On 32-bit x86,
use SSE2 and `-mfpmath=sse`; x87 excess precision is rejected. The kernels also
reject detectable fast-math/finite-only builds and non-binary64 formats.
Custom/amalgamated builds must obey the same contract. Architecture-specific
libm calls, FMA variants, and CPU dispatch must not be added to this layer.
The build flag `-ffp-contract=off` is required for GCC/Clang. Source files do
not adapt compiler options locally. There is no portable preprocessor macro
that detects contraction mode; hardware FMA availability is not such a check.

Determinism takes priority over matching a particular CPython platform's libm.
OpenLibm's documented approximation error is below one ulp for `exp`/`log` and
below 0.503 ulp for normal `exp2` results. This is not a claim of correct rounding
for all inputs or a universal one-ulp bound on `pow`; its small-integer multiply
shortcuts can accumulate rounding error.
The trigonometric kernels are described upstream as nearly rounded; this does
not promise correct rounding for every input. The accuracy vectors test them
against independently computed references with a one-ulp allowance.

## Python behavior

`math.exp`, `math.log`, `math.log2`, `math.log10`,
`math.pow`, and floating `**` share this layer. Consumers such as `cmath`,
easing, and color conversion inherit the new kernels.
`math.sin`, `math.cos`, `math.tan`, and internal `dmath_sincos` also use this
layer. Vector/matrix rotations retain the combined sine/cosine entry point.
`math.ceil`, `math.floor`, and `math.trunc` use the fixed rounding kernels;
floating `//` and `divmod` also inherit `dmath_floor`.

The C functions return IEEE values, also exposed by the Python math wrappers:
overflow returns signed infinity, underflow returns a subnormal or signed zero,
and invalid real domains return canonical NaN. Logarithms of zero return
negative infinity; zero to a negative floating power returns signed infinity.
No arithmetic exception type or new Python math API is introduced. This is an
intentional difference from CPython's domain/overflow exception policy.

The C rounding functions preserve signed zeros and infinities and canonicalize
NaNs. Python `math.ceil`/`floor`/`trunc` return an `int` when the result fits the
signed 64-bit range. Results outside that range remain integral `float` values;
NaNs and infinities also remain floats instead of raising an exception or
undergoing an undefined integer conversion. Integer inputs are returned exactly,
without a round trip through double. Zero results at the Python level remain
integer zero. This differs from CPython's arbitrary-precision integer results
and its exceptions for nonfinite rounding inputs.

Integer `**` preserves its fixed-width result type and now uses defined
modulo-2**64 arithmetic instead of C signed-overflow UB. The existing
`ZeroDivisionError` for integer zero to a negative integer power is retained.
Existing pocketpy limitations remain: no arbitrary-precision integers, no
automatic complex promotion for negative fractional real powers (`**` returns
NaN), and the existing numeric conversion protocol. The audit and tests of the
remaining dmath functions are documented in `tests/dmath/README.md`.

## Validation

Configure with `-DPK_BUILD_MATH_TESTS=ON`, build, then run
`ctest --test-dir <build-directory> -C Release --output-on-failure`.

The new suite covers all 31 public dmath functions in separate groups, with
one readable case file per function under `tests/dmath/cases/`. It replaces
the previous vector headers and shared fingerprints. CTest also forces the
software sqrt path on hardware-sqrt hosts. `tests/930_dmath.py` checks Python
bindings using the same finite output bits and nonfinite classification/sign
expectations; `tests/932_dmath_consumers.py` checks operators and other consumers.

Independent Decimal/Fraction references, frozen output bits and per-function
random sweeps are distinct checks. See `tests/dmath/README.md` for the case
categories, numerical contract, audit findings, commands and baseline review
process. Check references with `python scripts/dmath/generate.py --check`;
this never updates frozen expectations from implementation outputs.
