#include "pocketpy/line_profiler.h"

namespace pkpy{

struct LineProfiler{
    PY_CLASS(LineProfiler, line_profiler, LineProfiler)

    std::set<void*> _functions;

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<LineProfiler>(type);

        vm->bind(type, "add_function(self, func)", [](VM* vm, ArgsView args){
            // ...
            return vm->None;
        });

        vm->bind(type, "runcall(self, func, *args, **kw)", [](VM* vm, ArgsView args){
            // ...
            return vm->None;
        });

        vm->bind(type, "print_stats(self)", [](VM* vm, ArgsView args){
            // ...
            return vm->None;
        });
    }
};

void add_module_line_profiler(VM *vm){
    PyObject* mod = vm->new_module("line_profiler");
    LineProfiler::register_class(vm, mod);
}

}   // namespace pkpy