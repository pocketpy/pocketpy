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
#include "_generated.h"
#include "vm.h"
#include "re.h"
#include "random.h"
#include "bindings.h"

namespace pkpy {

inline CodeObject_ VM::compile(Str source, Str filename, CompileMode mode, bool unknown_global_scope) {
    Compiler compiler(this, source, filename, mode, unknown_global_scope);
    try{
        return compiler.compile();
    }catch(Exception& e){
#if PK_DEBUG_FULL_EXCEPTION
        std::cerr << e.summary() << std::endl;
#endif
        _error(e);
        return nullptr;
    }
}


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
            static std::stringstream ss_out;
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

/*************************GLOBAL NAMESPACE*************************/
extern "C" {
    PK_INLINE_EXPORT
    void pkpy_free(void* p){
        free(p);
    }

    PK_INLINE_EXPORT
    void pkpy_vm_exec(pkpy::VM* vm, const char* source){
        vm->exec(source, "main.py", pkpy::EXEC_MODE);
    }

    PK_INLINE_EXPORT
    void pkpy_vm_exec_2(pkpy::VM* vm, const char* source, const char* filename, int mode, const char* module){
        pkpy::PyObject* mod;
        if(module == nullptr) mod = vm->_main;
        else{
            mod = vm->_modules.try_get(module);
            if(mod == nullptr) return;
        }
        vm->exec(source, filename, (pkpy::CompileMode)mode, mod);
    }

    PK_INLINE_EXPORT
    void pkpy_vm_compile(pkpy::VM* vm, const char* source, const char* filename, int mode, bool* ok, char** res){
        try{
            pkpy::CodeObject_ code = vm->compile(source, filename, (pkpy::CompileMode)mode);
            *res = code->serialize(vm).c_str_dup();
            *ok = true;
        }catch(pkpy::Exception& e){
            *ok = false;
            *res = e.summary().c_str_dup();
        }catch(std::exception& e){
            *ok = false;
            *res = strdup(e.what());
        }catch(...){
            *ok = false;
            *res = strdup("unknown error");
        }
    }

    PK_INLINE_EXPORT
    pkpy::REPL* pkpy_new_repl(pkpy::VM* vm){
        pkpy::REPL* p = new pkpy::REPL(vm);
        return p;
    }

    PK_INLINE_EXPORT
    bool pkpy_repl_input(pkpy::REPL* r, const char* line){
        return r->input(line);
    }

    PK_INLINE_EXPORT
    void pkpy_vm_add_module(pkpy::VM* vm, const char* name, const char* source){
        vm->_lazy_modules[name] = source;
    }

    PK_INLINE_EXPORT
    pkpy::VM* pkpy_new_vm(bool enable_os=true){
        pkpy::VM* p = new pkpy::VM(enable_os);
        return p;
    }

    PK_INLINE_EXPORT
    void pkpy_delete_vm(pkpy::VM* vm){
        delete vm;
    }

    PK_INLINE_EXPORT
    void pkpy_delete_repl(pkpy::REPL* repl){
        delete repl;
    }

    PK_INLINE_EXPORT
    void pkpy_vm_gc_on_delete(pkpy::VM* vm, void (*f)(pkpy::VM *, pkpy::PyObject *)){
        vm->heap._gc_on_delete = f;
    }
}
