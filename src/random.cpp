#include "pocketpy/random.h"

namespace pkpy{

struct Random{
    PY_CLASS(Random, random, Random)
    std::mt19937 gen;

    Random(){
        gen.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<Random>(type);

        vm->bind_method<1>(type, "seed", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            self.gen.seed(CAST(i64, args[1]));
            return vm->None;
        });

        vm->bind_method<2>(type, "randint", [](VM* vm, ArgsView args) {
            Random& self = _CAST(Random&, args[0]);
            i64 a = CAST(i64, args[1]);
            i64 b = CAST(i64, args[2]);
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
            const List& L = CAST(List&, args[1]);
            std::uniform_int_distribution<i64> dis(0, L.size() - 1);
            return L[dis(self.gen)];
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
}

}   // namespace pkpy