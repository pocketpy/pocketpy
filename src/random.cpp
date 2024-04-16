#include "pocketpy/random.h"

namespace pkpy{

struct Random{
    PY_CLASS(Random, random, Random)
    std::mt19937 gen;

    Random(){
        gen.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_func<1>(type, __new__, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            return vm->heap.gcnew<Random>(cls);
        });

        vm->bind_method<1>(type, "seed", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            self.gen.seed(CAST(i64, args[1]));
            return vm->None;
        });

        vm->bind_method<2>(type, "randint", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            i64 a = CAST(i64, args[1]);
            i64 b = CAST(i64, args[2]);
            if (a > b) vm->ValueError("randint(a, b): a must be less than or equal to b");
            std::uniform_int_distribution<i64> dis(a, b);
            return VAR(dis(self.gen));
        });

        vm->bind_method<0>(type, "random", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            std::uniform_real_distribution<f64> dis(0.0, 1.0);
            return VAR(dis(self.gen));
        });

        vm->bind_method<2>(type, "uniform", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            f64 a = CAST(f64, args[1]);
            f64 b = CAST(f64, args[2]);
            std::uniform_real_distribution<f64> dis(a, b);
            return VAR(dis(self.gen));
        });

        vm->bind_method<1>(type, "shuffle", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            List& L = CAST(List&, args[1]);
            std::shuffle(L.begin(), L.end(), self.gen);
            return vm->None;
        });

        vm->bind_method<1>(type, "choice", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            auto [data, size] = vm->_cast_array(args[1]);
            if(size == 0) vm->IndexError("cannot choose from an empty sequence");
            std::uniform_int_distribution<i64> dis(0, size - 1);
            return data[dis(self.gen)];
        });

        vm->bind(type, "choices(self, population, weights=None, k=1)", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
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
                f64 r = std::uniform_real_distribution<f64>(0.0, cum_weights[size - 1])(self.gen);
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