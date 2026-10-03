/* Per-function binary64 tests; see tests/dmath/README.md. No host-libm oracle. */
#include "pocketpy/common/dmath.h"
#include <fenv.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    T_isfinite,
    T_isinf,
    T_isnan,
    T_isnormal,
    T_fabs,
    T_copysign,
    T_fmin,
    T_fmax,
    T_ceil,
    T_floor,
    T_trunc,
    T_modf,
    T_fmod,
    T_sqrt,
    T_cbrt,
    T_exp,
    T_exp2,
    T_exp10,
    T_pow,
    T_log,
    T_log2,
    T_log10,
    T_log_base,
    T_sin,
    T_cos,
    T_tan,
    T_sincos,
    T_asin,
    T_acos,
    T_atan,
    T_atan2
} Operation;

typedef struct {
    const char* category;
    const char* name;
    Operation op;
    int outputs;
    int rounding_independent;
} Function;

const static Function functions[] = {
    {"classification",         "isfinite", T_isfinite, 1, 1},
    {"classification",         "isinf",    T_isinf,    1, 1},
    {"classification",         "isnan",    T_isnan,    1, 1},
    {"classification",         "isnormal", T_isnormal, 1, 1},
    {"sign_and_order",         "fabs",     T_fabs,     1, 1},
    {"sign_and_order",         "copysign", T_copysign, 1, 1},
    {"sign_and_order",         "fmin",     T_fmin,     1, 1},
    {"sign_and_order",         "fmax",     T_fmax,     1, 1},
    {"rounding_and_remainder", "ceil",     T_ceil,     1, 1},
    {"rounding_and_remainder", "floor",    T_floor,    1, 1},
    {"rounding_and_remainder", "trunc",    T_trunc,    1, 1},
    {"rounding_and_remainder", "modf",     T_modf,     2, 1},
    {"rounding_and_remainder", "fmod",     T_fmod,     1, 1},
    {"roots",                  "sqrt",     T_sqrt,     1, 0},
    {"roots",                  "cbrt",     T_cbrt,     1, 0},
    {"exponentials",           "exp",      T_exp,      1, 0},
    {"exponentials",           "exp2",     T_exp2,     1, 0},
    {"exponentials",           "exp10",    T_exp10,    1, 0},
    {"exponentials",           "pow",      T_pow,      1, 0},
    {"logarithms",             "log",      T_log,      1, 0},
    {"logarithms",             "log2",     T_log2,     1, 0},
    {"logarithms",             "log10",    T_log10,    1, 0},
    {"logarithms",             "log_base", T_log_base, 1, 0},
    {"trigonometry",           "sin",      T_sin,      1, 0},
    {"trigonometry",           "cos",      T_cos,      1, 0},
    {"trigonometry",           "tan",      T_tan,      1, 0},
    {"trigonometry",           "sincos",   T_sincos,   2, 0},
    {"inverse_trigonometry",   "asin",     T_asin,     1, 0},
    {"inverse_trigonometry",   "acos",     T_acos,     1, 0},
    {"inverse_trigonometry",   "atan",     T_atan,     1, 0},
    {"inverse_trigonometry",   "atan2",    T_atan2,    1, 0},
};

const static uint64_t magnitude_mask = UINT64_C(0x7fffffffffffffff);
const static uint64_t infinity_bits = UINT64_C(0x7ff0000000000000);
const static uint64_t canonical_nan = UINT64_C(0x7ff8000000000000);
const static uint64_t sign_bit = UINT64_C(0x8000000000000000);
static int failures;

static void evaluate(const Function* f, uint64_t a, uint64_t b, uint64_t out[2]) {
    double x = pk_dmath_from_bits(a), y = pk_dmath_from_bits(b), z = 0, aux = 0;
    out[1] = 0;
    switch(f->op) {
        case T_isfinite: out[0] = dmath_isfinite(x); return;
        case T_isinf: out[0] = dmath_isinf(x); return;
        case T_isnan: out[0] = dmath_isnan(x); return;
        case T_isnormal: out[0] = dmath_isnormal(x); return;
        case T_fabs: z = dmath_fabs(x); break;
        case T_copysign: z = dmath_copysign(x, y); break;
        case T_fmin: z = dmath_fmin(x, y); break;
        case T_fmax: z = dmath_fmax(x, y); break;
        case T_ceil: z = dmath_ceil(x); break;
        case T_floor: z = dmath_floor(x); break;
        case T_trunc: z = dmath_trunc(x); break;
        case T_modf: z = dmath_modf(x, &aux); break;
        case T_fmod: z = dmath_fmod(x, y); break;
        case T_sqrt: z = dmath_sqrt(x); break;
        case T_cbrt: z = dmath_cbrt(x); break;
        case T_exp: z = dmath_exp(x); break;
        case T_exp2: z = dmath_exp2(x); break;
        case T_exp10: z = dmath_exp10(x); break;
        case T_pow: z = dmath_pow(x, y); break;
        case T_log: z = dmath_log(x); break;
        case T_log2: z = dmath_log2(x); break;
        case T_log10: z = dmath_log10(x); break;
        case T_log_base: z = dmath_log_base(x, y); break;
        case T_sin: z = dmath_sin(x); break;
        case T_cos: z = dmath_cos(x); break;
        case T_tan: z = dmath_tan(x); break;
        case T_sincos: dmath_sincos(x, &z, &aux); break;
        case T_asin: z = dmath_asin(x); break;
        case T_acos: z = dmath_acos(x); break;
        case T_atan: z = dmath_atan(x); break;
        case T_atan2: z = dmath_atan2(x, y); break;
    }
    out[0] = pk_dmath_bits(z);
    out[1] = pk_dmath_bits(aux);
}

static void mismatch(const Function* f,
                     const char* label,
                     const char* check,
                     uint64_t a,
                     uint64_t b,
                     uint64_t actual,
                     uint64_t expected) {
    if(failures++ < 30)
        fprintf(stderr,
                "FAIL dmath_%s/%s [%s] x=%016" PRIx64 " y=%016" PRIx64 " got=%016" PRIx64
                " expected=%016" PRIx64 "\n",
                f->name,
                label,
                check,
                a,
                b,
                actual,
                expected);
}

static int accurate(uint64_t actual, uint64_t reference, unsigned ulps) {
    if(actual == reference) return 1;
    uint64_t a = actual & magnitude_mask, r = reference & magnitude_mask;
    // Nonfinite outputs have semantic checks, never NaN payload/sign checks.
    if(r > infinity_bits) return a > infinity_bits;
    if(r == infinity_bits) return a == infinity_bits && !((actual ^ reference) & sign_bit);
    if(r == 0 || a >= infinity_bits || ((actual ^ reference) & sign_bit)) return 0;
    return (a > r ? a - r : r - a) <= ulps;
}

static uint64_t signature_word(uint64_t u) {
    // A fingerprint records NaN classification, not its representation.
    return (u & magnitude_mask) > infinity_bits ? canonical_nan : u;
}

static uint64_t parse_word(const char* text) {
    char* end;
    uint64_t u = strtoull(text, &end, 16);
    if(strlen(text) != 16 || *end) {
        fprintf(stderr, "Invalid binary64 word: %s\n", text);
        exit(1);
    }
    return u;
}

static uint64_t parse_result(const char* text) {
    if(strcmp(text, "nan") == 0) return canonical_nan;
    if(strcmp(text, "+inf") == 0) return infinity_bits;
    if(strcmp(text, "-inf") == 0) return infinity_bits | sign_bit;
    return parse_word(text);
}

static unsigned
    named_cases(const Function* f, const char* directory, int probe, uint64_t* expected_sweep) {
    char path[1024], line[1024];
    snprintf(path, sizeof(path), "%s/%s.txt", directory, f->name);
    FILE* file = fopen(path, "r");
    if(!file) {
        perror(path);
        exit(1);
    }
    unsigned count = 0;
    int found_sweep = 0;
    while(fgets(line, sizeof(line), file)) {
        char fingerprint[32];
        if(sscanf(line, "# sweep %31s", fingerprint) == 1) {
            found_sweep = 1;
            if(strcmp(fingerprint, "PENDING") == 0) {
                if(!probe) {
                    fprintf(stderr, "%s: unreviewed sweep\n", path);
                    failures++;
                }
            } else
                *expected_sweep = parse_word(fingerprint);
            continue;
        }
        if(line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        char label[128], words[6][32];
        unsigned ulps;
        if(sscanf(line,
                  "%127s %31s %31s %31s %31s %31s %31s %u",
                  label,
                  words[0],
                  words[1],
                  words[2],
                  words[3],
                  words[4],
                  words[5],
                  &ulps) != 8) {
            fprintf(stderr, "Malformed case in %s\n", path);
            exit(1);
        }
        uint64_t a = parse_word(words[0]), b = parse_word(words[1]), out[2];
        evaluate(f, a, b, out);
        for(int i = 0; i < f->outputs; i++) {
            uint64_t ref = parse_result(words[4 + i]);
            if(!accurate(out[i], ref, ulps))
                mismatch(f, label, i ? "accuracy/output1" : "accuracy/output0", a, b, out[i], ref);
            if(!probe) {
                if(strcmp(words[2 + i], "PENDING") == 0) {
                    fprintf(stderr, "%s/%s: unreviewed result\n", f->name, label);
                    failures++;
                } else {
                    uint64_t want = parse_result(words[2 + i]);
                    if(!accurate(out[i], want, 0))
                        mismatch(f, label, i ? "bits/output1" : "bits/output0", a, b, out[i], want);
                }
            }
        }
        if(probe == 1)
            printf("CASE %s %s %016" PRIx64 " %016" PRIx64 "\n",
                   f->name,
                   label,
                   signature_word(out[0]),
                   signature_word(out[1]));
        count++;
    }
    if(ferror(file) || !count || !found_sweep) {
        fprintf(stderr, "Incomplete corpus: %s\n", path);
        exit(1);
    }
    fclose(file);
    return count;
}

static uint64_t hash_word(uint64_t hash, uint64_t word) {
    for(int byte = 0; byte < 8; byte++, word >>= 8)
        hash = (hash ^ (word & 255)) * UINT64_C(1099511628211);
    return hash;
}

static uint64_t random_word(uint64_t* state) {
    // SplitMix64, with unsigned wrapping arithmetic and a new seed per function.
    uint64_t z = (*state += UINT64_C(0x9e3779b97f4a7c15));
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

static void invariants(const Function* f, uint64_t a, uint64_t b, const uint64_t out[2]) {
    uint64_t ax = a & magnitude_mask, ay = b & magnitude_mask, expected;
    int has_exact = 1;
    switch(f->op) {
        case T_isfinite: expected = ax < infinity_bits; break;
        case T_isinf: expected = ax == infinity_bits; break;
        case T_isnan: expected = ax > infinity_bits; break;
        case T_isnormal: expected = ax >= UINT64_C(0x0010000000000000) && ax < infinity_bits; break;
        case T_fabs: expected = ax; break;
        case T_copysign: expected = ax | (b & sign_bit); break;
        default:
            has_exact = 0;
            expected = 0;
            break;
    }
    if(has_exact) {
        if(!accurate(out[0], expected, 0))
            mismatch(f, "sweep", "finite bits/nonfinite class", a, b, out[0], expected);
        return;
    }
    if(f->op == T_fmin || f->op == T_fmax) {
        uint64_t reverse[2];
        evaluate(f, b, a, reverse);
        if(!accurate(out[0], reverse[0], 0))
            mismatch(f, "sweep", "commutativity", a, b, out[0], reverse[0]);
    }
    if(f->op == T_sincos) {
        expected = pk_dmath_bits(dmath_sin(pk_dmath_from_bits(a)));
        if(!accurate(out[0], expected, 0))
            mismatch(f, "sweep", "separate sine", a, b, out[0], expected);
        expected = pk_dmath_bits(dmath_cos(pk_dmath_from_bits(a)));
        if(!accurate(out[1], expected, 0))
            mismatch(f, "sweep", "separate cosine", a, b, out[1], expected);
    }
    if(f->op == T_fmod && ax < infinity_bits && ay != 0 && ay <= infinity_bits) {
        if((out[0] & magnitude_mask) >= ay || ((a ^ out[0]) & sign_bit))
            mismatch(f, "sweep", "remainder range/sign", a, b, out[0], a & sign_bit);
    }
    if(f->op == T_modf && ax < infinity_bits) {
        double fraction = pk_dmath_from_bits(out[0]), integral = pk_dmath_from_bits(out[1]);
        expected = pk_dmath_bits(fraction + integral);
        if(expected != a) mismatch(f, "sweep", "recomposition", a, b, expected, a);
        if((out[0] & magnitude_mask) >= UINT64_C(0x3ff0000000000000) || ((out[0] ^ a) & sign_bit) ||
           ((out[1] ^ a) & sign_bit))
            mismatch(f, "sweep", "fraction range/sign", a, b, out[0], a & sign_bit);
    }
}

static uint64_t sweep(const Function* f) {
    uint64_t state = UINT64_C(0x243f6a8885a308d3);
    for(const char* p = f->name; *p; p++)
        state = hash_word(state, (unsigned char)*p);
    uint64_t hash = UINT64_C(14695981039346656037);
    // All exponent fields, with fresh significands of both signs, followed by
    // 16,384 full-range words and an additional stream in useful finite domains.
    for(unsigned i = 0; i < 4096 + 16384; i++) {
        uint64_t a = random_word(&state), b = random_word(&state), out[2];
        if(i < 4096)
            a = ((uint64_t)(i & 1) << 63) | ((uint64_t)(i / 2) << 52) |
                (a & UINT64_C(0x000fffffffffffff));
        evaluate(f, a, b, out);
        invariants(f, a, b, out);
        for(int j = 0; j < f->outputs; j++)
            hash = hash_word(hash, signature_word(out[j]));
        if(f->op >= T_exp && f->op <= T_pow) {
            double bounded = (double)(a >> 32) * 0x1p-21 - 1024.0;
            if(f->op == T_exp10) bounded *= 0.25;
            if(f->op == T_pow) {
                a = UINT64_C(0x3ff0000000000000) + (a & 4095);
                b = pk_dmath_bits(bounded * 0x1p40);
            } else
                a = pk_dmath_bits(bounded);
        } else if(f->op == T_asin || f->op == T_acos) {
            a = pk_dmath_bits((double)(a >> 11) * 0x1p-52 - 1.0);
        } else if(f->op >= T_log && f->op <= T_log_base) {
            a &= magnitude_mask;
            b &= magnitude_mask;
        } else
            continue;
        evaluate(f, a, b, out);
        invariants(f, a, b, out);
        for(int j = 0; j < f->outputs; j++)
            hash = hash_word(hash, signature_word(out[j]));
    }
    return hash;
}

static void run(const Function* f, const char* directory, int probe) {
    int before = failures;
    uint64_t frozen = 0;
    unsigned count = named_cases(f, directory, probe, &frozen);
    uint64_t actual = sweep(f);
    if(probe)
        printf("SWEEP %s %016" PRIx64 "\n", f->name, actual);
    else if(actual != frozen)
        mismatch(f, "sweep", "fingerprint", 0, 0, actual, frozen);
    if(f->rounding_independent) {
        const int modes[] = {FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
        for(unsigned i = 0; i < sizeof(modes) / sizeof(*modes); i++) {
            if(fesetround(modes[i]) != 0) {
                fputs("fesetround failed\n", stderr);
                exit(1);
            }
            named_cases(f, directory, probe ? 2 : 0, &frozen);
            uint64_t directed = sweep(f);
            if(directed != actual)
                mismatch(f, "sweep", "rounding mode dependence", modes[i], 0, directed, actual);
        }
        if(fesetround(FE_TONEAREST) != 0) {
            fputs("cannot restore rounding\n", stderr);
            exit(1);
        }
    }
    printf("%s %s/%s: %u named cases, sweep=%016" PRIx64 ", %d rounding mode(s)\n",
           failures == before ? "PASS" : "FAIL",
           f->category,
           f->name,
           count,
           actual,
           f->rounding_independent ? 4 : 1);
}

int main(int argc, char** argv) {
    const char* directory = "tests/dmath/cases";
    const char* only = NULL;
    int probe = 0, list = 0, ran = 0;
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--cases") == 0 && i + 1 < argc)
            directory = argv[++i];
        else if(strcmp(argv[i], "--probe") == 0)
            probe = 1;
        else if(strcmp(argv[i], "--list") == 0)
            list = 1;
        else if(argv[i][0] != '-' && !only)
            only = argv[i];
        else {
            fputs("Usage: test_dmath [--cases DIR] [--list] [--probe] [FUNCTION]\n", stderr);
            return 1;
        }
    }
    if(fegetround() != FE_TONEAREST) {
        fputs("dmath requires nearest-even\n", stderr);
        return 1;
    }
    volatile double tiny = pk_dmath_from_bits(3);
    volatile double normal = pk_dmath_from_bits(UINT64_C(0x0010000000000000));
    if(pk_dmath_bits(tiny + tiny) != 6 ||
       pk_dmath_bits(normal * 0.25) != UINT64_C(0x0004000000000000)) {
        fputs("dmath requires gradual underflow (FTZ/DAZ disabled)\n", stderr);
        return 1;
    }
    for(unsigned i = 0; i < sizeof(functions) / sizeof(*functions); i++) {
        const Function* f = &functions[i];
        if(only && strcmp(f->name, only) != 0) continue;
        if(list)
            printf("%s/%s\n", f->category, f->name);
        else
            run(f, directory, probe);
        ran++;
    }
    if(!ran) {
        fprintf(stderr, "Unknown dmath function: %s\n", only);
        return 1;
    }
    if(failures) fprintf(stderr, "%d dmath check(s) failed\n", failures);
    return failures ? 1 : 0;
}
