#pragma once

#include "compiler.h"
#include "obj.h"
#include "repl.h"
#include "iter.h"
#include "base64.h"
#include "cffi.h"
#include "linalg.h"
#include "easing.h"
#include "io.h"
#include "vm.h"
#include "re.h"
#include "random.h"
#include "bindings.h"

namespace pkpy {

void init_builtins(VM* _vm);

struct PyREPL{
    PY_CLASS(PyREPL, sys, _repl)

    REPL* repl;

    PyREPL(VM* vm){ repl = new REPL(vm); }
    ~PyREPL(){ delete repl; }

    PyREPL(const PyREPL&) = delete;
    PyREPL& operator=(const PyREPL&) = delete;

    PyREPL(PyREPL&& other) noexcept{
        repl = other.repl;
        other.repl = nullptr;
    }

    struct TempOut{
        PrintFunc backup;
        VM* vm;
        TempOut(VM* vm, PrintFunc f){
            this->vm = vm;
            this->backup = vm->_stdout;
            vm->_stdout = f;
        }
        ~TempOut(){
            vm->_stdout = backup;
        }
        TempOut(const TempOut&) = delete;
        TempOut& operator=(const TempOut&) = delete;
        TempOut(TempOut&&) = delete;
        TempOut& operator=(TempOut&&) = delete;
    };

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<1>(type, [](VM* vm, ArgsView args){
            return VAR_T(PyREPL, vm);
        });

        vm->bind_method<1>(type, "input", [](VM* vm, ArgsView args){
            PyREPL& self = _CAST(PyREPL&, args[0]);
            const Str& s = CAST(Str&, args[1]);
            PK_LOCAL_STATIC std::stringstream ss_out;
            ss_out.str("");
            TempOut _(vm, [](VM* vm, const Str& s){ ss_out << s; });
            bool ok = self.repl->input(s.str());
            return VAR(Tuple({VAR(ok), VAR(ss_out.str())}));
        });
    }
};


void add_module_timeit(VM* vm);
void add_module_time(VM* vm);
void add_module_sys(VM* vm);
void add_module_json(VM* vm);

void add_module_math(VM* vm);
void add_module_dis(VM* vm);
void add_module_traceback(VM* vm);
void add_module_gc(VM* vm);

}   // namespace pkpy