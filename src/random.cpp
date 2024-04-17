#include "pocketpy/random.h"

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

struct mt19937{
    static const int N = 624;
    static const int M = 397;
    const uint32_t MATRIX_A = 0x9908b0dfUL;   /* constant vector a */
    const uint32_t UPPER_MASK = 0x80000000UL; /* most significant w-r bits */
    const uint32_t LOWER_MASK = 0x7fffffffUL; /* least significant r bits */

    uint32_t mt[N]; /* the array for the state vector  */
    int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

    /* initializes mt[N] with a seed */
    void seed(uint32_t s)
    {
        mt[0]= s & 0xffffffffUL;
        for (mti=1; mti<N; mti++) {
            mt[mti] = 
            (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
            /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
            /* In the previous versions, MSBs of the seed affect   */
            /* only MSBs of the array mt[].                        */
            /* 2002/01/09 modified by Makoto Matsumoto             */
            mt[mti] &= 0xffffffffUL;
            /* for >32 bit machines */
        }
    }

    /* generates a random number on [0,0xffffffff]-interval */
    uint32_t next_uint32(void)
    {
        uint32_t y;
        static uint32_t mag01[2]={0x0UL, MATRIX_A};
        /* mag01[x] = x * MATRIX_A  for x=0,1 */

        if (mti >= N) { /* generate N words at one time */
            int kk;

            if (mti == N+1)   /* if init_genrand() has not been called, */
                seed(5489UL); /* a default initial seed is used */

            for (kk=0;kk<N-M;kk++) {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
            }
            for (;kk<N-1;kk++) {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
            }
            y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
            mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

            mti = 0;
        }
    
        y = mt[mti++];

        /* Tempering */
        y ^= (y >> 11);
        y ^= (y << 7) & 0x9d2c5680UL;
        y ^= (y << 15) & 0xefc60000UL;
        y ^= (y >> 18);

        return y;
    }

    uint64_t next_uint64(void){
        return (uint64_t(next_uint32()) << 32) | next_uint32();
    }

    /* generates a random number on [0,1)-real-interval */
    float random(void)
    {
        return next_uint32()*(1.0/4294967296.0); /* divided by 2^32 */
    }

    /* generates a random number on [a, b]-interval */
    int64_t randint(int64_t a, int64_t b){
        uint64_t delta = b - a + 1;
        if(delta < 0x80000000UL){
            return a + next_uint32() % (uint32_t)delta;
        }else{
            return a + next_uint64() % delta;
        }
    }

    float uniform(float a, float b){
        return a + random() * (b - a);
    }
};


namespace pkpy{

struct Random{
    PY_CLASS(Random, random, Random)
    mt19937 gen;

    Random(){
        auto count = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        gen.seed((uint32_t)count);
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_func<1>(type, __new__, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<Random>(cls);
        });

        vm->bind_method<1>(type, "seed", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            self.gen.seed(CAST(i64, args[1]));
            return vm->None;
        });

        vm->bind_method<2>(type, "randint", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            i64 a = CAST(i64, args[1]);
            i64 b = CAST(i64, args[2]);
            if (a > b) vm->ValueError("randint(a, b): a must be less than or equal to b");
            return VAR(self.gen.randint(a, b));
        });

        vm->bind_method<0>(type, "random", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            return VAR(self.gen.random());
        });

        vm->bind_method<2>(type, "uniform", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            f64 a = CAST(f64, args[1]);
            f64 b = CAST(f64, args[2]);
            if (a > b) std::swap(a, b);
            return VAR(self.gen.uniform(a, b));
        });

        vm->bind_method<1>(type, "shuffle", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            List& L = CAST(List&, args[1]);
            for(int i = L.size() - 1; i > 0; i--){
                int j = self.gen.randint(0, i);
                std::swap(L[i], L[j]);
            }
            return vm->None;
        });

        vm->bind_method<1>(type, "choice", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            auto [data, size] = vm->_cast_array(args[1]);
            if(size == 0) vm->IndexError("cannot choose from an empty sequence");
            int index = self.gen.randint(0, size-1);
            return data[index];
        });

        vm->bind(type, "choices(self, population, weights=None, k=1)", [](VM* vm, ArgsView args) {
            Random& self = PK_OBJ_GET(Random, args[0]);
            auto [data, size] = vm->_cast_array(args[1]);
            if(size == 0) vm->IndexError("cannot choose from an empty sequence");
            pod_vector<f64> cum_weights(size);
            if(args[2] == vm->None){
                for(int i = 0; i < size; i++) cum_weights[i] = i + 1;
            }else{
                auto [weights, weights_size] = vm->_cast_array(args[2]);
                if(weights_size != size) vm->ValueError(_S("len(weights) != ", size));
                cum_weights[0] = CAST(f64, weights[0]);
                for(int i = 1; i < size; i++){
                    cum_weights[i] = cum_weights[i - 1] + CAST(f64, weights[i]);
                }
            }
            if(cum_weights[size - 1] <= 0) vm->ValueError("total of weights must be greater than zero");
            int k = CAST(i64, args[3]);
            List result(k);
            for(int i = 0; i < k; i++){
                f64 r = self.gen.uniform(0.0, cum_weights[size - 1]);
                int idx = std::lower_bound(cum_weights.begin(), cum_weights.end(), r) - cum_weights.begin();
                result[i] = data[idx];
            }
            return VAR(std::move(result));
        });
    }
};

void add_module_random(VM* vm){
    PyObject* mod = vm->new_module("random");
    Random::register_class(vm, mod);
    PyObject* instance = vm->heap.gcnew<Random>(Random::_type(vm));
    mod->attr().set("seed", vm->getattr(instance, "seed"));
    mod->attr().set("random", vm->getattr(instance, "random"));
    mod->attr().set("uniform", vm->getattr(instance, "uniform"));
    mod->attr().set("randint", vm->getattr(instance, "randint"));
    mod->attr().set("shuffle", vm->getattr(instance, "shuffle"));
    mod->attr().set("choice", vm->getattr(instance, "choice"));
    mod->attr().set("choices", vm->getattr(instance, "choices"));
}

}   // namespace pkpy