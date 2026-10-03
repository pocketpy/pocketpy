# Deterministic math tests

The suite covers every public function declared in
`include/pocketpy/common/dmath.h`. It replaces the old mixed Python assertions,
exponential/trigonometric vector headers, generators, and aggregate fingerprints.
The general math compatibility tests in `tests/704_math.py` and
`tests/931_math.py` are separate from this determinism suite.

## Find a function's cases

Each function has one file in `tests/dmath/cases/`, for example `sqrt.txt`,
`atan2.txt`, or `modf.txt`. Each row has a descriptive case name, two exact input
words, frozen finite output words, independently calculated references, and an
accuracy allowance. Nonfinite expectations are the readable tokens `nan`,
`+inf`, and `-inf`; their bit patterns are not compared. Hexadecimal
floating-point inputs appear beside each row.
`modf` orders its outputs as (fraction, integral); `sincos` uses (sine, cosine).
Classification results are integer 0/1, not binary64 encodings of 0.0/1.0.

| Category | Function groups | Main coverage |
| --- | --- | --- |
| Classification | `isfinite`, `isinf`, `isnan`, `isnormal` | All exponent classes, signs, quiet/signaling NaNs |
| Sign and ordering | `fabs`, `copysign`, `fmin`, `fmax` | Finite sign changes, NaN classification, NaNs in either operand, zero ties |
| Rounding and remainder | `ceil`, `floor`, `trunc`, `modf`, `fmod` | Fraction boundaries, word/exponent transitions, huge quotients, both outputs |
| Roots | `sqrt`, `cbrt` | Subnormals, exact roots, exponent classes, software sqrt |
| Exponentials | `exp`, `exp2`, `exp10`, `pow` | Reduction/table boundaries, underflow ties, overflow, parity, near-one bases |
| Logarithms | `log`, `log2`, `log10`, `log_base` | Normalization, neighbors of one, poles, invalid bases |
| Trigonometry | `sin`, `cos`, `tan`, `sincos` | Tiny inputs, argument-reduction boundaries, large finite angles, tangent poles |
| Inverse trigonometry | `asin`, `acos`, `atan`, `atan2` | Domain endpoints, polynomial interval boundaries, quadrants, axes, extreme ratios |

The new design contains 1,250 named cases across 31 functions. Mandatory IEEE
endpoints such as signed zero and infinity naturally recur; arbitrary inputs,
case organization, random streams, and fingerprints were designed anew.

## Run

From the repository root:

```sh
cmake -S . -B build -DPK_BUILD_MATH_TESTS=ON -DPK_ENABLE_DETERMINISM=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# Select a function, or list every group (Windows executables may be in Release/).
build/test_dmath --list
build/test_dmath atan2
build/test_dmath --cases /absolute/path/to/tests/dmath/cases sqrt

# Python bindings, optionally selecting one function.
./main tests/930_dmath.py
./main tests/930_dmath.py modf
./main tests/932_dmath_consumers.py

# Verify the independent references and complete public-function coverage.
python scripts/dmath/generate.py --check
```

CTest reports each function separately, plus `dmath.sqrt_software`. The latter
forces the integer-arithmetic sqrt fallback and checks the same expected bits
as hardware sqrt. Normal test execution uses neither `assert` in C nor the
host libm as a numerical oracle; Release/NDEBUG builds keep all checks active.

Python tests read the same case files. Byte copies through `stdc` transport
binary64 values without the integer or decimal parsers. Some ABIs quiet
signaling NaNs in transit; only their classification is asserted.
Python rounding return types and exact integer
arguments are checked separately. The six internal-only functions (`exp2`,
`exp10`, `sincos`, `isnormal`, `fmin`, `fmax`) are tested through the C runner.
Power operators, divmod, complex exponentials, and vector rotations have fresh
integration cases in `932_dmath_consumers.py`.

## Three distinct checks

1. **Accuracy and special-value semantics.** `scripts/dmath/oracle.py` uses
   exact integers/Fractions for bit operations, rounding, remainders, and
   integer powers. Other references use Decimal roots/exp/log, Machin's formula
   for pi, angle reduction and convergent series. Rounded answers must agree
   at both 430 and 570 decimal digits. Exact rational evaluation resolves
   binary halfway cases, including `exp2(-1075)`. NaNs have classification
   checks; infinities have classification/sign checks. Signed zero and exact
   finite operations have bitwise checks. Approximation allowances are listed
   in `scripts/dmath/cases.py`; they are test budgets, not global error proofs.
2. **Frozen finite output bits.** Every named case also checks the reviewed
   finite result exactly, including both outputs of `modf`/`sincos`. The initial
   baseline was accepted only after GCC, Clang, and MSVC independently passed
   the accuracy checks and produced identical results. All observed nonzero
   errors in this named corpus were one ULP. Tolerances never weaken this
   exact-output check.
3. **Per-function sweeps.** Each function gets its own SplitMix64 stream:
   every exponent field with both signs and fresh significands, followed by
   16,384 full-range inputs. Exponential, logarithmic and inverse-trigonometric
   groups add inputs in useful finite domains. Each function has a separate
   endian-independent fingerprint. Classification and bit operations have
   exact runtime references; other invariants include min/max commutativity,
   remainder range/sign, modf recomposition, and sincos
   agreement with separate calls. These sweeps detect deterministic changes;
   they do not establish random-input accuracy against a high-precision oracle.
   NaN results are mapped to one test-only token before hashing, so differences
   in NaN sign, payload or quiet bit cannot fail a determinism check. Infinity
   tokens distinguish signs. Finite values, including signed zero, are unchanged.

Classification, sign/order, rounding, and remainder groups run under all four
rounding modes. Other computations require nearest-even. Entry checks detect
unsupported rounding and FTZ/DAZ environments. No test or kernel repairs the
embedding application's floating-point environment.

## Updating cases

Edit the named function in `scripts/dmath/cases.py`, then run the generator.
Existing frozen words are preserved only when the case name and both inputs
match. New cases are marked `PENDING`; normal tests fail until reviewed. The
generator never invokes dmath or automatically recalibrates expected bits.
`test_dmath --probe [function]` prints candidate words/fingerprints while still
checking independent references and invariants. It is a diagnostic, not a
passing determinism test; CI always runs without `--probe`.

Review numerical changes before updating frozen words. Compare independent
compiler results, run the accuracy checks and sanitizers, and inspect every
changed output. Never regenerate a baseline merely to turn a failure green.

## Implementation audit (2026-10-03)

| Area | Result |
| --- | --- |
| `isfinite`, `isinf`, `isnan`, `isnormal` | Bit classification; full exponent/sign coverage. No arithmetic on NaNs. |
| `fabs`, `copysign` | Finite sign-bit operations retained. MSVC x86 may quiet signaling NaNs through its return ABI; NaN encoding is explicitly outside the determinism contract. No ABI workaround or new normalization. |
| `fmin`, `fmax` | Fixed single-NaN handling and operand-order-dependent zero ties. Two NaNs produce NaN. Minimum prefers -0; maximum prefers +0. |
| `ceil`, `floor`, `trunc` | Unsigned masks, no out-of-range float-to-int casts. All finite magnitudes supported. |
| `modf`, `fmod` | Remainder algorithms retained. Both modf outputs and full finite exponent ranges are covered; NaN results are checked by classification. |
| `sqrt` | Hardware implementation retained. Software/hardware results use the same corpus and finite-result fingerprint. |
| `cbrt`, `asin`, `acos`, `atan`, `atan2` | Legacy algorithms retained. Boundary/accuracy tests cover these formerly omitted C groups. |
| OpenLibm exp/log/pow/trig kernels | Reviewed special branches, shift/cast bounds, subnormal paths and reduction dependencies. New cases and full UBSan pass; no approximation coefficients changed. |
| NaN/Inf constants | Replaced overflow-expression construction with bit construction: GCC directed-rounding tests found that the former NaN expression could evaluate to finite zero. This fixes classification, without requiring any particular NaN output bits. |
| Build contract | Format, fast-math and excess-precision checks apply to all kernels. Floating-point options belong to CMake; no compiler-specific FP pragmas or local control macros. |
| `math.modf` binding | Fixed direct float-storage access for integer/non-numeric arguments; use the existing checked numeric conversion. |

The contract assumes IEEE binary64, nearest-even, gradual underflow, disabled
FP traps, and the configured noncontracting arithmetic. `errno`, FP exception
flags, and NaN payload propagation by arithmetic functions are outside it.
There is no portable preprocessor test for `-ffp-contract`; the build must pass
the correct option. CPU FMA availability alone does not mean contraction is on.

The legacy Zig/musl source links do not identify the original upstream revision;
this remains a provenance limitation, not a build-time source dependency. The
OpenLibm source revision is pinned separately in `3rd/openlibm/README.md`.
The integer-prefix parser overflow discussed earlier is outside dmath and is
not changed here. The new corpus deliberately transports binary64 words so a
parser failure cannot hide or imitate a math-kernel failure.
