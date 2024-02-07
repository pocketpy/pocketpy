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

        vm->bind(type, "runcall(self, func, *args)", [](VM* vm, ArgsView view){
            LineProfiler& self = PK_OBJ_GET(LineProfiler, view[0]);
            // enable_by_count
            PyObject* func = view[1];
            const Tuple& args = CAST(Tuple&, view[2]);
            for(PyObject* arg : args) vm->s_data.push(arg);
            vm->s_data.push(func);
            PyObject* ret = vm->vectorcall(args.size());
            // disable_by_count
            return ret;
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