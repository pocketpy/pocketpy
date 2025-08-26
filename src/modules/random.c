#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"
#include <time.h>

int64_t time_ns();  // from random.c

/* https://github.com/clibs/mt19937ar

Copyright (c) 2011 Mutsuo Saito, Makoto Matsumoto, Hiroshima
University and The University of Tokyo. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of the Hiroshima University nor the names of
      its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

typedef struct mt19937 {
    uint32_t mt[N]; /* the array for the state vector  */
    int mti;        /* mti==N+1 means mt[N] is not initialized */
} mt19937;

/* initializes mt[N] with a seed */
static void mt19937__seed(mt19937* self, uint32_t s) {
    self->mt[0] = s & 0xffffffffUL;
    for(self->mti = 1; self->mti < N; self->mti++) {
        self->mt[self->mti] =
            (1812433253UL * (self->mt[self->mti - 1] ^ (self->mt[self->mti - 1] >> 30)) +
             self->mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        self->mt[self->mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

static void mt19937__ctor(mt19937* self) { self->mti = N + 1; }

/* generates a random number on [0,0xffffffff]-interval */
static uint32_t mt19937__next_uint32(mt19937* self) {
    uint32_t* mt = self->mt;
    uint32_t y;
    static uint32_t mag01[2] = {0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if(self->mti >= N) { /* generate N words at one time */
        int kk;

        if(self->mti == N + 1) { /* if init_genrand() has not been called, */
            int64_t seed = time_ns();
            mt19937__seed(self, (uint32_t)seed);
            // seed(5489UL); /* a default initial seed is used */
        }

        for(kk = 0; kk < N - M; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for(; kk < N - 1; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        self->mti = 0;
    }

    y = mt[self->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

static uint64_t mt19937__next_uint64(mt19937* self) {
    return (uint64_t)mt19937__next_uint32(self) << 32 | mt19937__next_uint32(self);
}

static double mt19937__random(mt19937* self) {
    // from cpython
    uint32_t a = mt19937__next_uint32(self) >> 5;
    uint32_t b = mt19937__next_uint32(self) >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
}

static double mt19937__uniform(mt19937* self, double a, double b) {
    if(a > b) { return b + mt19937__random(self) * (a - b); }
    return a + mt19937__random(self) * (b - a);
}

/* generates a random number on [a, b]-interval */
int64_t mt19937__randint(mt19937* self, int64_t a, int64_t b) {
    uint64_t delta = b - a + 1;
    if(delta < 0x80000000UL) {
        return a + mt19937__next_uint32(self) % delta;
    } else {
        return a + mt19937__next_uint64(self) % delta;
    }
}

static bool Random__new__(int argc, py_Ref argv) {
    mt19937* ud = py_newobject(py_retval(), py_totype(argv), 0, sizeof(mt19937));
    mt19937__ctor(ud);
    return true;
}

static bool Random__init__(int argc, py_Ref argv) {
    if(argc == 1) {
        // do nothing
    } else if(argc == 2) {
        mt19937* ud = py_touserdata(py_arg(0));
        if(!py_isnone(&argv[1])) {
            PY_CHECK_ARG_TYPE(1, tp_int);
            py_i64 seed = py_toint(py_arg(1));
            mt19937__seed(ud, (uint32_t)seed);
        }
    } else {
        return TypeError("Random(): expected 1 or 2 arguments, got %d");
    }
    py_newnone(py_retval());
    return true;
}

static bool Random_seed(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    mt19937* ud = py_touserdata(py_arg(0));
    py_i64 seed;
    if(py_isnone(&argv[1])) {
        seed = time_ns();
    } else {
        PY_CHECK_ARG_TYPE(1, tp_int);
        seed = py_toint(py_arg(1));
    }
    mt19937__seed(ud, (uint32_t)seed);
    py_newnone(py_retval());
    return true;
}

static bool Random_random(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    mt19937* ud = py_touserdata(py_arg(0));
    py_f64 res = mt19937__random(ud);
    py_newfloat(py_retval(), res);
    return true;
}

static bool Random_uniform(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    mt19937* ud = py_touserdata(py_arg(0));
    py_f64 a, b;
    if(!py_castfloat(py_arg(1), &a)) return false;
    if(!py_castfloat(py_arg(2), &b)) return false;
    py_f64 res = mt19937__uniform(ud, a, b);
    py_newfloat(py_retval(), res);
    return true;
}

static bool Random_shuffle(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_list);
    mt19937* ud = py_touserdata(py_arg(0));
    py_Ref L = py_arg(1);
    int length = py_list_len(L);
    for(int i = length - 1; i > 0; i--) {
        int j = mt19937__randint(ud, 0, i);
        py_list_swap(L, i, j);
    }
    py_newnone(py_retval());
    return true;
}

static bool Random_randint(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    mt19937* ud = py_touserdata(py_arg(0));
    py_i64 a = py_toint(py_arg(1));
    py_i64 b = py_toint(py_arg(2));
    if(a > b) return ValueError("randint(a, b): a must be less than or equal to b");
    py_i64 res = mt19937__randint(ud, a, b);
    py_newint(py_retval(), res);
    return true;
}

static bool Random_choice(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    mt19937* ud = py_touserdata(py_arg(0));
    py_TValue* p;
    int length = pk_arrayview(py_arg(1), &p);
    if(length == -1) return TypeError("choice(): argument must be a list or tuple");
    if(length == 0) return IndexError("cannot choose from an empty sequence");
    int index = mt19937__randint(ud, 0, length - 1);
    py_assign(py_retval(), p + index);
    return true;
}

static bool Random_choices(int argc, py_Ref argv) {
    mt19937* ud = py_touserdata(py_arg(0));
    py_TValue* p;
    int length = pk_arrayview(py_arg(1), &p);
    if(length == -1) return TypeError("choices(): argument must be a list or tuple");
    if(length == 0) return IndexError("cannot choose from an empty sequence");
    py_Ref weights = py_arg(2);
    if(!py_checktype(py_arg(3), tp_int)) return false;
    py_i64 k = py_toint(py_arg(3));

    py_f64* cum_weights = PK_MALLOC(sizeof(py_f64) * length);
    if(py_isnone(weights)) {
        for(int i = 0; i < length; i++)
            cum_weights[i] = i + 1;
    } else {
        py_TValue* w;
        int wlen = pk_arrayview(weights, &w);
        if(wlen == -1) {
            PK_FREE(cum_weights);
            return TypeError("choices(): weights must be a list or tuple");
        }
        if(wlen != length) {
            PK_FREE(cum_weights);
            return ValueError("len(weights) != len(population)");
        }
        if(!py_castfloat(&w[0], &cum_weights[0])) {
            PK_FREE(cum_weights);
            return false;
        }
        for(int i = 1; i < length; i++) {
            py_f64 tmp;
            if(!py_castfloat(&w[i], &tmp)) {
                PK_FREE(cum_weights);
                return false;
            }
            cum_weights[i] = cum_weights[i - 1] + tmp;
        }
    }

    py_f64 total = cum_weights[length - 1];
    if(total <= 1e-6) {
        PK_FREE(cum_weights);
        return ValueError("total of weights must be greater than 1e-6");
    }

    py_newlistn(py_retval(), k);
    for(int i = 0; i < k; i++) {
        py_f64 key = mt19937__random(ud) * total;
        int index;
        c11__lower_bound(py_f64, cum_weights, length, key, c11__less, &index);
        assert(index != length);
        py_list_setitem(py_retval(), i, p + index);
    }

    PK_FREE(cum_weights);
    return true;
}

void pk__add_module_random() {
    py_Ref mod = py_newmodule("random");
    py_Type type = py_newtype("Random", tp_object, mod, NULL);

    py_bindmagic(type, __new__, Random__new__);
    py_bindmagic(type, __init__, Random__init__);
    py_bindmethod(type, "seed", Random_seed);
    py_bindmethod(type, "random", Random_random);
    py_bindmethod(type, "uniform", Random_uniform);
    py_bindmethod(type, "randint", Random_randint);
    py_bindmethod(type, "shuffle", Random_shuffle);
    py_bindmethod(type, "choice", Random_choice);
    py_bind(py_tpobject(type), "choices(self, population, weights=None, k=1)", Random_choices);

    py_Ref inst = py_pushtmp();
    if(!py_tpcall(type, 0, NULL)) goto __ERROR;
    py_assign(inst, py_retval());

#define ADD_INST_BOUNDMETHOD(name)                                                                 \
    if(!py_getattr(inst, py_name(name))) goto __ERROR;                                             \
    py_setdict(mod, py_name(name), py_retval());

    ADD_INST_BOUNDMETHOD("seed");
    ADD_INST_BOUNDMETHOD("random");
    ADD_INST_BOUNDMETHOD("uniform");
    ADD_INST_BOUNDMETHOD("randint");
    ADD_INST_BOUNDMETHOD("shuffle");
    ADD_INST_BOUNDMETHOD("choice");
    ADD_INST_BOUNDMETHOD("choices");

#undef ADD_INST_BOUNDMETHOD

    py_pop();  // pop inst
    return;

__ERROR:
    py_printexc();
    c11__abort("failed to add module random");
}

#undef N
#undef M
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK
#undef ADD_INST_BOUNDMETHOD

void py_newRandom(py_OutRef out) {
    py_Type type = py_gettype("random", py_name("Random"));
    assert(type != 0);
    mt19937* ud = py_newobject(out, type, 0, sizeof(mt19937));
    mt19937__ctor(ud);
}

void py_Random_seed(py_Ref self, py_i64 seed) {
    mt19937* ud = py_touserdata(self);
    mt19937__seed(ud, (uint32_t)seed);
}

py_f64 py_Random_random(py_Ref self) {
    mt19937* ud = py_touserdata(self);
    return mt19937__random(ud);
}

py_f64 py_Random_uniform(py_Ref self, py_f64 a, py_f64 b) {
    mt19937* ud = py_touserdata(self);
    return mt19937__uniform(ud, a, b);
}

py_i64 py_Random_randint(py_Ref self, py_i64 a, py_i64 b) {
    mt19937* ud = py_touserdata(self);
    if(a > b) { c11__abort("randint(a, b): a must be less than or equal to b"); }
    return mt19937__randint(ud, a, b);
}