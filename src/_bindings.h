extern "C" {
typedef i64 (*__f_int__int_int_int)(VM*, i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_int_int(VM* vm, const char* mod, const char* name, __f_int__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_int_float)(VM*, i64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_int_float(VM* vm, const char* mod, const char* name, __f_int__int_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_int_str)(VM*, i64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_int_str(VM* vm, const char* mod, const char* name, __f_int__int_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_int_bool)(VM*, i64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_int_bool(VM* vm, const char* mod, const char* name, __f_int__int_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float_int)(VM*, i64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_float_int(VM* vm, const char* mod, const char* name, __f_int__int_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float_float)(VM*, i64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_float_float(VM* vm, const char* mod, const char* name, __f_int__int_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float_str)(VM*, i64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_float_str(VM* vm, const char* mod, const char* name, __f_int__int_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float_bool)(VM*, i64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_float_bool(VM* vm, const char* mod, const char* name, __f_int__int_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str_int)(VM*, i64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_str_int(VM* vm, const char* mod, const char* name, __f_int__int_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str_float)(VM*, i64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_str_float(VM* vm, const char* mod, const char* name, __f_int__int_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str_str)(VM*, i64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_str_str(VM* vm, const char* mod, const char* name, __f_int__int_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str_bool)(VM*, i64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_str_bool(VM* vm, const char* mod, const char* name, __f_int__int_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool_int)(VM*, i64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_bool_int(VM* vm, const char* mod, const char* name, __f_int__int_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool_float)(VM*, i64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_bool_float(VM* vm, const char* mod, const char* name, __f_int__int_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool_str)(VM*, i64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_bool_str(VM* vm, const char* mod, const char* name, __f_int__int_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool_bool)(VM*, i64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_bool_bool(VM* vm, const char* mod, const char* name, __f_int__int_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int_int)(VM*, f64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_int_int(VM* vm, const char* mod, const char* name, __f_int__float_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int_float)(VM*, f64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_int_float(VM* vm, const char* mod, const char* name, __f_int__float_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int_str)(VM*, f64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_int_str(VM* vm, const char* mod, const char* name, __f_int__float_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int_bool)(VM*, f64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_int_bool(VM* vm, const char* mod, const char* name, __f_int__float_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float_int)(VM*, f64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_float_int(VM* vm, const char* mod, const char* name, __f_int__float_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float_float)(VM*, f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_float_float(VM* vm, const char* mod, const char* name, __f_int__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float_str)(VM*, f64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_float_str(VM* vm, const char* mod, const char* name, __f_int__float_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float_bool)(VM*, f64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_float_bool(VM* vm, const char* mod, const char* name, __f_int__float_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str_int)(VM*, f64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_str_int(VM* vm, const char* mod, const char* name, __f_int__float_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str_float)(VM*, f64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_str_float(VM* vm, const char* mod, const char* name, __f_int__float_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str_str)(VM*, f64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_str_str(VM* vm, const char* mod, const char* name, __f_int__float_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str_bool)(VM*, f64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_str_bool(VM* vm, const char* mod, const char* name, __f_int__float_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool_int)(VM*, f64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_bool_int(VM* vm, const char* mod, const char* name, __f_int__float_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool_float)(VM*, f64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_bool_float(VM* vm, const char* mod, const char* name, __f_int__float_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool_str)(VM*, f64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_bool_str(VM* vm, const char* mod, const char* name, __f_int__float_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool_bool)(VM*, f64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_bool_bool(VM* vm, const char* mod, const char* name, __f_int__float_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int_int)(VM*, const char*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_int_int(VM* vm, const char* mod, const char* name, __f_int__str_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int_float)(VM*, const char*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_int_float(VM* vm, const char* mod, const char* name, __f_int__str_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int_str)(VM*, const char*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_int_str(VM* vm, const char* mod, const char* name, __f_int__str_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int_bool)(VM*, const char*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_int_bool(VM* vm, const char* mod, const char* name, __f_int__str_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float_int)(VM*, const char*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_float_int(VM* vm, const char* mod, const char* name, __f_int__str_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float_float)(VM*, const char*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_float_float(VM* vm, const char* mod, const char* name, __f_int__str_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float_str)(VM*, const char*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_float_str(VM* vm, const char* mod, const char* name, __f_int__str_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float_bool)(VM*, const char*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_float_bool(VM* vm, const char* mod, const char* name, __f_int__str_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str_int)(VM*, const char*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_str_int(VM* vm, const char* mod, const char* name, __f_int__str_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str_float)(VM*, const char*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_str_float(VM* vm, const char* mod, const char* name, __f_int__str_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str_str)(VM*, const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_str_str(VM* vm, const char* mod, const char* name, __f_int__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str_bool)(VM*, const char*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_str_bool(VM* vm, const char* mod, const char* name, __f_int__str_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool_int)(VM*, const char*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_bool_int(VM* vm, const char* mod, const char* name, __f_int__str_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool_float)(VM*, const char*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_bool_float(VM* vm, const char* mod, const char* name, __f_int__str_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool_str)(VM*, const char*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_bool_str(VM* vm, const char* mod, const char* name, __f_int__str_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool_bool)(VM*, const char*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_bool_bool(VM* vm, const char* mod, const char* name, __f_int__str_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int_int)(VM*, bool, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_int_int(VM* vm, const char* mod, const char* name, __f_int__bool_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int_float)(VM*, bool, i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_int_float(VM* vm, const char* mod, const char* name, __f_int__bool_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int_str)(VM*, bool, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_int_str(VM* vm, const char* mod, const char* name, __f_int__bool_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int_bool)(VM*, bool, i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_int_bool(VM* vm, const char* mod, const char* name, __f_int__bool_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float_int)(VM*, bool, f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_float_int(VM* vm, const char* mod, const char* name, __f_int__bool_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float_float)(VM*, bool, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_float_float(VM* vm, const char* mod, const char* name, __f_int__bool_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float_str)(VM*, bool, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_float_str(VM* vm, const char* mod, const char* name, __f_int__bool_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float_bool)(VM*, bool, f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_float_bool(VM* vm, const char* mod, const char* name, __f_int__bool_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str_int)(VM*, bool, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_str_int(VM* vm, const char* mod, const char* name, __f_int__bool_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str_float)(VM*, bool, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_str_float(VM* vm, const char* mod, const char* name, __f_int__bool_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str_str)(VM*, bool, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_str_str(VM* vm, const char* mod, const char* name, __f_int__bool_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str_bool)(VM*, bool, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_str_bool(VM* vm, const char* mod, const char* name, __f_int__bool_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool_int)(VM*, bool, bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool_int(VM* vm, const char* mod, const char* name, __f_int__bool_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool_float)(VM*, bool, bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool_float(VM* vm, const char* mod, const char* name, __f_int__bool_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool_str)(VM*, bool, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool_str(VM* vm, const char* mod, const char* name, __f_int__bool_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool_bool)(VM*, bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_int__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(vm, _0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int_int_int)(VM*, i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_int_int(VM* vm, const char* mod, const char* name, __f_float__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_int_float)(VM*, i64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_int_float(VM* vm, const char* mod, const char* name, __f_float__int_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_int_str)(VM*, i64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_int_str(VM* vm, const char* mod, const char* name, __f_float__int_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_int_bool)(VM*, i64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_int_bool(VM* vm, const char* mod, const char* name, __f_float__int_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float_int)(VM*, i64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_float_int(VM* vm, const char* mod, const char* name, __f_float__int_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float_float)(VM*, i64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_float_float(VM* vm, const char* mod, const char* name, __f_float__int_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float_str)(VM*, i64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_float_str(VM* vm, const char* mod, const char* name, __f_float__int_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float_bool)(VM*, i64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_float_bool(VM* vm, const char* mod, const char* name, __f_float__int_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str_int)(VM*, i64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_str_int(VM* vm, const char* mod, const char* name, __f_float__int_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str_float)(VM*, i64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_str_float(VM* vm, const char* mod, const char* name, __f_float__int_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str_str)(VM*, i64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_str_str(VM* vm, const char* mod, const char* name, __f_float__int_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str_bool)(VM*, i64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_str_bool(VM* vm, const char* mod, const char* name, __f_float__int_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool_int)(VM*, i64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_bool_int(VM* vm, const char* mod, const char* name, __f_float__int_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool_float)(VM*, i64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_bool_float(VM* vm, const char* mod, const char* name, __f_float__int_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool_str)(VM*, i64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_bool_str(VM* vm, const char* mod, const char* name, __f_float__int_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool_bool)(VM*, i64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_bool_bool(VM* vm, const char* mod, const char* name, __f_float__int_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int_int)(VM*, f64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_int_int(VM* vm, const char* mod, const char* name, __f_float__float_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int_float)(VM*, f64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_int_float(VM* vm, const char* mod, const char* name, __f_float__float_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int_str)(VM*, f64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_int_str(VM* vm, const char* mod, const char* name, __f_float__float_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int_bool)(VM*, f64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_int_bool(VM* vm, const char* mod, const char* name, __f_float__float_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float_int)(VM*, f64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_float_int(VM* vm, const char* mod, const char* name, __f_float__float_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float_float)(VM*, f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_float_float(VM* vm, const char* mod, const char* name, __f_float__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float_str)(VM*, f64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_float_str(VM* vm, const char* mod, const char* name, __f_float__float_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float_bool)(VM*, f64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_float_bool(VM* vm, const char* mod, const char* name, __f_float__float_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str_int)(VM*, f64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_str_int(VM* vm, const char* mod, const char* name, __f_float__float_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str_float)(VM*, f64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_str_float(VM* vm, const char* mod, const char* name, __f_float__float_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str_str)(VM*, f64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_str_str(VM* vm, const char* mod, const char* name, __f_float__float_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str_bool)(VM*, f64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_str_bool(VM* vm, const char* mod, const char* name, __f_float__float_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool_int)(VM*, f64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_bool_int(VM* vm, const char* mod, const char* name, __f_float__float_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool_float)(VM*, f64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_bool_float(VM* vm, const char* mod, const char* name, __f_float__float_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool_str)(VM*, f64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_bool_str(VM* vm, const char* mod, const char* name, __f_float__float_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool_bool)(VM*, f64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_bool_bool(VM* vm, const char* mod, const char* name, __f_float__float_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int_int)(VM*, const char*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_int_int(VM* vm, const char* mod, const char* name, __f_float__str_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int_float)(VM*, const char*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_int_float(VM* vm, const char* mod, const char* name, __f_float__str_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int_str)(VM*, const char*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_int_str(VM* vm, const char* mod, const char* name, __f_float__str_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int_bool)(VM*, const char*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_int_bool(VM* vm, const char* mod, const char* name, __f_float__str_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float_int)(VM*, const char*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_float_int(VM* vm, const char* mod, const char* name, __f_float__str_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float_float)(VM*, const char*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_float_float(VM* vm, const char* mod, const char* name, __f_float__str_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float_str)(VM*, const char*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_float_str(VM* vm, const char* mod, const char* name, __f_float__str_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float_bool)(VM*, const char*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_float_bool(VM* vm, const char* mod, const char* name, __f_float__str_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str_int)(VM*, const char*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_str_int(VM* vm, const char* mod, const char* name, __f_float__str_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str_float)(VM*, const char*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_str_float(VM* vm, const char* mod, const char* name, __f_float__str_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str_str)(VM*, const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_str_str(VM* vm, const char* mod, const char* name, __f_float__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str_bool)(VM*, const char*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_str_bool(VM* vm, const char* mod, const char* name, __f_float__str_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool_int)(VM*, const char*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_bool_int(VM* vm, const char* mod, const char* name, __f_float__str_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool_float)(VM*, const char*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_bool_float(VM* vm, const char* mod, const char* name, __f_float__str_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool_str)(VM*, const char*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_bool_str(VM* vm, const char* mod, const char* name, __f_float__str_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool_bool)(VM*, const char*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_bool_bool(VM* vm, const char* mod, const char* name, __f_float__str_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int_int)(VM*, bool, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_int_int(VM* vm, const char* mod, const char* name, __f_float__bool_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int_float)(VM*, bool, i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_int_float(VM* vm, const char* mod, const char* name, __f_float__bool_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int_str)(VM*, bool, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_int_str(VM* vm, const char* mod, const char* name, __f_float__bool_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int_bool)(VM*, bool, i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_int_bool(VM* vm, const char* mod, const char* name, __f_float__bool_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float_int)(VM*, bool, f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_float_int(VM* vm, const char* mod, const char* name, __f_float__bool_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float_float)(VM*, bool, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_float_float(VM* vm, const char* mod, const char* name, __f_float__bool_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float_str)(VM*, bool, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_float_str(VM* vm, const char* mod, const char* name, __f_float__bool_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float_bool)(VM*, bool, f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_float_bool(VM* vm, const char* mod, const char* name, __f_float__bool_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str_int)(VM*, bool, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_str_int(VM* vm, const char* mod, const char* name, __f_float__bool_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str_float)(VM*, bool, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_str_float(VM* vm, const char* mod, const char* name, __f_float__bool_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str_str)(VM*, bool, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_str_str(VM* vm, const char* mod, const char* name, __f_float__bool_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str_bool)(VM*, bool, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_str_bool(VM* vm, const char* mod, const char* name, __f_float__bool_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool_int)(VM*, bool, bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool_int(VM* vm, const char* mod, const char* name, __f_float__bool_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool_float)(VM*, bool, bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool_float(VM* vm, const char* mod, const char* name, __f_float__bool_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool_str)(VM*, bool, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool_str(VM* vm, const char* mod, const char* name, __f_float__bool_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool_bool)(VM*, bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_float__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(vm, _0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int_int_int)(VM*, i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_int_int(VM* vm, const char* mod, const char* name, __f_str__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_int_float)(VM*, i64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_int_float(VM* vm, const char* mod, const char* name, __f_str__int_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_int_str)(VM*, i64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_int_str(VM* vm, const char* mod, const char* name, __f_str__int_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_int_bool)(VM*, i64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_int_bool(VM* vm, const char* mod, const char* name, __f_str__int_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float_int)(VM*, i64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_float_int(VM* vm, const char* mod, const char* name, __f_str__int_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float_float)(VM*, i64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_float_float(VM* vm, const char* mod, const char* name, __f_str__int_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float_str)(VM*, i64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_float_str(VM* vm, const char* mod, const char* name, __f_str__int_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float_bool)(VM*, i64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_float_bool(VM* vm, const char* mod, const char* name, __f_str__int_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str_int)(VM*, i64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_str_int(VM* vm, const char* mod, const char* name, __f_str__int_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str_float)(VM*, i64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_str_float(VM* vm, const char* mod, const char* name, __f_str__int_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str_str)(VM*, i64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_str_str(VM* vm, const char* mod, const char* name, __f_str__int_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str_bool)(VM*, i64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_str_bool(VM* vm, const char* mod, const char* name, __f_str__int_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool_int)(VM*, i64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_bool_int(VM* vm, const char* mod, const char* name, __f_str__int_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool_float)(VM*, i64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_bool_float(VM* vm, const char* mod, const char* name, __f_str__int_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool_str)(VM*, i64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_bool_str(VM* vm, const char* mod, const char* name, __f_str__int_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool_bool)(VM*, i64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_bool_bool(VM* vm, const char* mod, const char* name, __f_str__int_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int_int)(VM*, f64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_int_int(VM* vm, const char* mod, const char* name, __f_str__float_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int_float)(VM*, f64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_int_float(VM* vm, const char* mod, const char* name, __f_str__float_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int_str)(VM*, f64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_int_str(VM* vm, const char* mod, const char* name, __f_str__float_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int_bool)(VM*, f64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_int_bool(VM* vm, const char* mod, const char* name, __f_str__float_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float_int)(VM*, f64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_float_int(VM* vm, const char* mod, const char* name, __f_str__float_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float_float)(VM*, f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_float_float(VM* vm, const char* mod, const char* name, __f_str__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float_str)(VM*, f64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_float_str(VM* vm, const char* mod, const char* name, __f_str__float_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float_bool)(VM*, f64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_float_bool(VM* vm, const char* mod, const char* name, __f_str__float_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str_int)(VM*, f64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_str_int(VM* vm, const char* mod, const char* name, __f_str__float_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str_float)(VM*, f64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_str_float(VM* vm, const char* mod, const char* name, __f_str__float_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str_str)(VM*, f64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_str_str(VM* vm, const char* mod, const char* name, __f_str__float_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str_bool)(VM*, f64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_str_bool(VM* vm, const char* mod, const char* name, __f_str__float_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool_int)(VM*, f64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_bool_int(VM* vm, const char* mod, const char* name, __f_str__float_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool_float)(VM*, f64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_bool_float(VM* vm, const char* mod, const char* name, __f_str__float_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool_str)(VM*, f64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_bool_str(VM* vm, const char* mod, const char* name, __f_str__float_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool_bool)(VM*, f64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_bool_bool(VM* vm, const char* mod, const char* name, __f_str__float_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int_int)(VM*, const char*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_int_int(VM* vm, const char* mod, const char* name, __f_str__str_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int_float)(VM*, const char*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_int_float(VM* vm, const char* mod, const char* name, __f_str__str_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int_str)(VM*, const char*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_int_str(VM* vm, const char* mod, const char* name, __f_str__str_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int_bool)(VM*, const char*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_int_bool(VM* vm, const char* mod, const char* name, __f_str__str_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float_int)(VM*, const char*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_float_int(VM* vm, const char* mod, const char* name, __f_str__str_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float_float)(VM*, const char*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_float_float(VM* vm, const char* mod, const char* name, __f_str__str_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float_str)(VM*, const char*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_float_str(VM* vm, const char* mod, const char* name, __f_str__str_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float_bool)(VM*, const char*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_float_bool(VM* vm, const char* mod, const char* name, __f_str__str_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str_int)(VM*, const char*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_str_int(VM* vm, const char* mod, const char* name, __f_str__str_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str_float)(VM*, const char*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_str_float(VM* vm, const char* mod, const char* name, __f_str__str_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str_str)(VM*, const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_str_str(VM* vm, const char* mod, const char* name, __f_str__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str_bool)(VM*, const char*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_str_bool(VM* vm, const char* mod, const char* name, __f_str__str_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool_int)(VM*, const char*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_bool_int(VM* vm, const char* mod, const char* name, __f_str__str_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool_float)(VM*, const char*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_bool_float(VM* vm, const char* mod, const char* name, __f_str__str_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool_str)(VM*, const char*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_bool_str(VM* vm, const char* mod, const char* name, __f_str__str_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool_bool)(VM*, const char*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_bool_bool(VM* vm, const char* mod, const char* name, __f_str__str_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int_int)(VM*, bool, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_int_int(VM* vm, const char* mod, const char* name, __f_str__bool_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int_float)(VM*, bool, i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_int_float(VM* vm, const char* mod, const char* name, __f_str__bool_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int_str)(VM*, bool, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_int_str(VM* vm, const char* mod, const char* name, __f_str__bool_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int_bool)(VM*, bool, i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_int_bool(VM* vm, const char* mod, const char* name, __f_str__bool_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float_int)(VM*, bool, f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_float_int(VM* vm, const char* mod, const char* name, __f_str__bool_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float_float)(VM*, bool, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_float_float(VM* vm, const char* mod, const char* name, __f_str__bool_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float_str)(VM*, bool, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_float_str(VM* vm, const char* mod, const char* name, __f_str__bool_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float_bool)(VM*, bool, f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_float_bool(VM* vm, const char* mod, const char* name, __f_str__bool_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str_int)(VM*, bool, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_str_int(VM* vm, const char* mod, const char* name, __f_str__bool_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str_float)(VM*, bool, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_str_float(VM* vm, const char* mod, const char* name, __f_str__bool_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str_str)(VM*, bool, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_str_str(VM* vm, const char* mod, const char* name, __f_str__bool_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str_bool)(VM*, bool, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_str_bool(VM* vm, const char* mod, const char* name, __f_str__bool_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool_int)(VM*, bool, bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool_int(VM* vm, const char* mod, const char* name, __f_str__bool_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool_float)(VM*, bool, bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool_float(VM* vm, const char* mod, const char* name, __f_str__bool_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool_str)(VM*, bool, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool_str(VM* vm, const char* mod, const char* name, __f_str__bool_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool_bool)(VM*, bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_str__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(vm, _0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int_int_int)(VM*, i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_int_int(VM* vm, const char* mod, const char* name, __f_bool__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_int_float)(VM*, i64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_int_float(VM* vm, const char* mod, const char* name, __f_bool__int_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_int_str)(VM*, i64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_int_str(VM* vm, const char* mod, const char* name, __f_bool__int_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_int_bool)(VM*, i64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_int_bool(VM* vm, const char* mod, const char* name, __f_bool__int_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float_int)(VM*, i64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_float_int(VM* vm, const char* mod, const char* name, __f_bool__int_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float_float)(VM*, i64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_float_float(VM* vm, const char* mod, const char* name, __f_bool__int_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float_str)(VM*, i64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_float_str(VM* vm, const char* mod, const char* name, __f_bool__int_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float_bool)(VM*, i64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_float_bool(VM* vm, const char* mod, const char* name, __f_bool__int_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str_int)(VM*, i64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_str_int(VM* vm, const char* mod, const char* name, __f_bool__int_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str_float)(VM*, i64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_str_float(VM* vm, const char* mod, const char* name, __f_bool__int_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str_str)(VM*, i64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_str_str(VM* vm, const char* mod, const char* name, __f_bool__int_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str_bool)(VM*, i64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_str_bool(VM* vm, const char* mod, const char* name, __f_bool__int_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool_int)(VM*, i64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool_int(VM* vm, const char* mod, const char* name, __f_bool__int_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool_float)(VM*, i64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool_float(VM* vm, const char* mod, const char* name, __f_bool__int_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool_str)(VM*, i64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool_str(VM* vm, const char* mod, const char* name, __f_bool__int_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool_bool)(VM*, i64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool_bool(VM* vm, const char* mod, const char* name, __f_bool__int_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int_int)(VM*, f64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_int_int(VM* vm, const char* mod, const char* name, __f_bool__float_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int_float)(VM*, f64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_int_float(VM* vm, const char* mod, const char* name, __f_bool__float_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int_str)(VM*, f64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_int_str(VM* vm, const char* mod, const char* name, __f_bool__float_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int_bool)(VM*, f64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_int_bool(VM* vm, const char* mod, const char* name, __f_bool__float_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float_int)(VM*, f64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_float_int(VM* vm, const char* mod, const char* name, __f_bool__float_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float_float)(VM*, f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_float_float(VM* vm, const char* mod, const char* name, __f_bool__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float_str)(VM*, f64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_float_str(VM* vm, const char* mod, const char* name, __f_bool__float_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float_bool)(VM*, f64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_float_bool(VM* vm, const char* mod, const char* name, __f_bool__float_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str_int)(VM*, f64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_str_int(VM* vm, const char* mod, const char* name, __f_bool__float_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str_float)(VM*, f64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_str_float(VM* vm, const char* mod, const char* name, __f_bool__float_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str_str)(VM*, f64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_str_str(VM* vm, const char* mod, const char* name, __f_bool__float_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str_bool)(VM*, f64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_str_bool(VM* vm, const char* mod, const char* name, __f_bool__float_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool_int)(VM*, f64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool_int(VM* vm, const char* mod, const char* name, __f_bool__float_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool_float)(VM*, f64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool_float(VM* vm, const char* mod, const char* name, __f_bool__float_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool_str)(VM*, f64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool_str(VM* vm, const char* mod, const char* name, __f_bool__float_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool_bool)(VM*, f64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool_bool(VM* vm, const char* mod, const char* name, __f_bool__float_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int_int)(VM*, const char*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_int_int(VM* vm, const char* mod, const char* name, __f_bool__str_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int_float)(VM*, const char*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_int_float(VM* vm, const char* mod, const char* name, __f_bool__str_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int_str)(VM*, const char*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_int_str(VM* vm, const char* mod, const char* name, __f_bool__str_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int_bool)(VM*, const char*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_int_bool(VM* vm, const char* mod, const char* name, __f_bool__str_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float_int)(VM*, const char*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_float_int(VM* vm, const char* mod, const char* name, __f_bool__str_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float_float)(VM*, const char*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_float_float(VM* vm, const char* mod, const char* name, __f_bool__str_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float_str)(VM*, const char*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_float_str(VM* vm, const char* mod, const char* name, __f_bool__str_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float_bool)(VM*, const char*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_float_bool(VM* vm, const char* mod, const char* name, __f_bool__str_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str_int)(VM*, const char*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_str_int(VM* vm, const char* mod, const char* name, __f_bool__str_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str_float)(VM*, const char*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_str_float(VM* vm, const char* mod, const char* name, __f_bool__str_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str_str)(VM*, const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_str_str(VM* vm, const char* mod, const char* name, __f_bool__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str_bool)(VM*, const char*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_str_bool(VM* vm, const char* mod, const char* name, __f_bool__str_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool_int)(VM*, const char*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool_int(VM* vm, const char* mod, const char* name, __f_bool__str_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool_float)(VM*, const char*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool_float(VM* vm, const char* mod, const char* name, __f_bool__str_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool_str)(VM*, const char*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool_str(VM* vm, const char* mod, const char* name, __f_bool__str_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool_bool)(VM*, const char*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool_bool(VM* vm, const char* mod, const char* name, __f_bool__str_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int_int)(VM*, bool, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int_int(VM* vm, const char* mod, const char* name, __f_bool__bool_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int_float)(VM*, bool, i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int_float(VM* vm, const char* mod, const char* name, __f_bool__bool_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int_str)(VM*, bool, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int_str(VM* vm, const char* mod, const char* name, __f_bool__bool_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int_bool)(VM*, bool, i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float_int)(VM*, bool, f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float_int(VM* vm, const char* mod, const char* name, __f_bool__bool_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float_float)(VM*, bool, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float_float(VM* vm, const char* mod, const char* name, __f_bool__bool_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float_str)(VM*, bool, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float_str(VM* vm, const char* mod, const char* name, __f_bool__bool_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float_bool)(VM*, bool, f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str_int)(VM*, bool, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str_int(VM* vm, const char* mod, const char* name, __f_bool__bool_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str_float)(VM*, bool, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str_float(VM* vm, const char* mod, const char* name, __f_bool__bool_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str_str)(VM*, bool, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str_str(VM* vm, const char* mod, const char* name, __f_bool__bool_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str_bool)(VM*, bool, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool_int)(VM*, bool, bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool_int(VM* vm, const char* mod, const char* name, __f_bool__bool_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool_float)(VM*, bool, bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool_float(VM* vm, const char* mod, const char* name, __f_bool__bool_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool_str)(VM*, bool, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool_str(VM* vm, const char* mod, const char* name, __f_bool__bool_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool_bool)(VM*, bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(vm, _0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int_int_int)(VM*, i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_int_int(VM* vm, const char* mod, const char* name, __f_None__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_int_float)(VM*, i64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_int_float(VM* vm, const char* mod, const char* name, __f_None__int_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_int_str)(VM*, i64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_int_str(VM* vm, const char* mod, const char* name, __f_None__int_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_int_bool)(VM*, i64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_int_bool(VM* vm, const char* mod, const char* name, __f_None__int_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_float_int)(VM*, i64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_float_int(VM* vm, const char* mod, const char* name, __f_None__int_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_float_float)(VM*, i64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_float_float(VM* vm, const char* mod, const char* name, __f_None__int_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_float_str)(VM*, i64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_float_str(VM* vm, const char* mod, const char* name, __f_None__int_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_float_bool)(VM*, i64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_float_bool(VM* vm, const char* mod, const char* name, __f_None__int_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_str_int)(VM*, i64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_str_int(VM* vm, const char* mod, const char* name, __f_None__int_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_str_float)(VM*, i64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_str_float(VM* vm, const char* mod, const char* name, __f_None__int_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_str_str)(VM*, i64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_str_str(VM* vm, const char* mod, const char* name, __f_None__int_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_str_bool)(VM*, i64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_str_bool(VM* vm, const char* mod, const char* name, __f_None__int_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool_int)(VM*, i64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_bool_int(VM* vm, const char* mod, const char* name, __f_None__int_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool_float)(VM*, i64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_bool_float(VM* vm, const char* mod, const char* name, __f_None__int_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool_str)(VM*, i64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_bool_str(VM* vm, const char* mod, const char* name, __f_None__int_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool_bool)(VM*, i64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_bool_bool(VM* vm, const char* mod, const char* name, __f_None__int_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_int_int)(VM*, f64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_int_int(VM* vm, const char* mod, const char* name, __f_None__float_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_int_float)(VM*, f64, i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_int_float(VM* vm, const char* mod, const char* name, __f_None__float_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_int_str)(VM*, f64, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_int_str(VM* vm, const char* mod, const char* name, __f_None__float_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_int_bool)(VM*, f64, i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_int_bool(VM* vm, const char* mod, const char* name, __f_None__float_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_float_int)(VM*, f64, f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_float_int(VM* vm, const char* mod, const char* name, __f_None__float_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_float_float)(VM*, f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_float_float(VM* vm, const char* mod, const char* name, __f_None__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_float_str)(VM*, f64, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_float_str(VM* vm, const char* mod, const char* name, __f_None__float_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_float_bool)(VM*, f64, f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_float_bool(VM* vm, const char* mod, const char* name, __f_None__float_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_str_int)(VM*, f64, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_str_int(VM* vm, const char* mod, const char* name, __f_None__float_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_str_float)(VM*, f64, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_str_float(VM* vm, const char* mod, const char* name, __f_None__float_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_str_str)(VM*, f64, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_str_str(VM* vm, const char* mod, const char* name, __f_None__float_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_str_bool)(VM*, f64, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_str_bool(VM* vm, const char* mod, const char* name, __f_None__float_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool_int)(VM*, f64, bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_bool_int(VM* vm, const char* mod, const char* name, __f_None__float_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool_float)(VM*, f64, bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_bool_float(VM* vm, const char* mod, const char* name, __f_None__float_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool_str)(VM*, f64, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_bool_str(VM* vm, const char* mod, const char* name, __f_None__float_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool_bool)(VM*, f64, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_bool_bool(VM* vm, const char* mod, const char* name, __f_None__float_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_int_int)(VM*, const char*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_int_int(VM* vm, const char* mod, const char* name, __f_None__str_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_int_float)(VM*, const char*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_int_float(VM* vm, const char* mod, const char* name, __f_None__str_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_int_str)(VM*, const char*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_int_str(VM* vm, const char* mod, const char* name, __f_None__str_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_int_bool)(VM*, const char*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_int_bool(VM* vm, const char* mod, const char* name, __f_None__str_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_float_int)(VM*, const char*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_float_int(VM* vm, const char* mod, const char* name, __f_None__str_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_float_float)(VM*, const char*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_float_float(VM* vm, const char* mod, const char* name, __f_None__str_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_float_str)(VM*, const char*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_float_str(VM* vm, const char* mod, const char* name, __f_None__str_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_float_bool)(VM*, const char*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_float_bool(VM* vm, const char* mod, const char* name, __f_None__str_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_str_int)(VM*, const char*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_str_int(VM* vm, const char* mod, const char* name, __f_None__str_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_str_float)(VM*, const char*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_str_float(VM* vm, const char* mod, const char* name, __f_None__str_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_str_str)(VM*, const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_str_str(VM* vm, const char* mod, const char* name, __f_None__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_str_bool)(VM*, const char*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_str_bool(VM* vm, const char* mod, const char* name, __f_None__str_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool_int)(VM*, const char*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_bool_int(VM* vm, const char* mod, const char* name, __f_None__str_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool_float)(VM*, const char*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_bool_float(VM* vm, const char* mod, const char* name, __f_None__str_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool_str)(VM*, const char*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_bool_str(VM* vm, const char* mod, const char* name, __f_None__str_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool_bool)(VM*, const char*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_bool_bool(VM* vm, const char* mod, const char* name, __f_None__str_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int_int)(VM*, bool, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_int_int(VM* vm, const char* mod, const char* name, __f_None__bool_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int_float)(VM*, bool, i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_int_float(VM* vm, const char* mod, const char* name, __f_None__bool_int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int_str)(VM*, bool, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_int_str(VM* vm, const char* mod, const char* name, __f_None__bool_int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int_bool)(VM*, bool, i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_int_bool(VM* vm, const char* mod, const char* name, __f_None__bool_int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float_int)(VM*, bool, f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_float_int(VM* vm, const char* mod, const char* name, __f_None__bool_float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float_float)(VM*, bool, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_float_float(VM* vm, const char* mod, const char* name, __f_None__bool_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float_str)(VM*, bool, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_float_str(VM* vm, const char* mod, const char* name, __f_None__bool_float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float_bool)(VM*, bool, f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_float_bool(VM* vm, const char* mod, const char* name, __f_None__bool_float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str_int)(VM*, bool, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_str_int(VM* vm, const char* mod, const char* name, __f_None__bool_str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str_float)(VM*, bool, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_str_float(VM* vm, const char* mod, const char* name, __f_None__bool_str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str_str)(VM*, bool, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_str_str(VM* vm, const char* mod, const char* name, __f_None__bool_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str_bool)(VM*, bool, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_str_bool(VM* vm, const char* mod, const char* name, __f_None__bool_str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool_int)(VM*, bool, bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool_int(VM* vm, const char* mod, const char* name, __f_None__bool_bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool_float)(VM*, bool, bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool_float(VM* vm, const char* mod, const char* name, __f_None__bool_bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool_str)(VM*, bool, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool_str(VM* vm, const char* mod, const char* name, __f_None__bool_bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool_bool)(VM*, bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_None__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(vm, _0, _1, _2);
        return vm->None;
    });
}

typedef i64 (*__f_int__int_int)(VM*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_int(VM* vm, const char* mod, const char* name, __f_int__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float)(VM*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_float(VM* vm, const char* mod, const char* name, __f_int__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str)(VM*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_str(VM* vm, const char* mod, const char* name, __f_int__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool)(VM*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_bool(VM* vm, const char* mod, const char* name, __f_int__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int)(VM*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_int(VM* vm, const char* mod, const char* name, __f_int__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float)(VM*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_float(VM* vm, const char* mod, const char* name, __f_int__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str)(VM*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_str(VM* vm, const char* mod, const char* name, __f_int__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool)(VM*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_bool(VM* vm, const char* mod, const char* name, __f_int__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int)(VM*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_int(VM* vm, const char* mod, const char* name, __f_int__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float)(VM*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_float(VM* vm, const char* mod, const char* name, __f_int__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str)(VM*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_str(VM* vm, const char* mod, const char* name, __f_int__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool)(VM*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_bool(VM* vm, const char* mod, const char* name, __f_int__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int)(VM*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_int(VM* vm, const char* mod, const char* name, __f_int__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float)(VM*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_float(VM* vm, const char* mod, const char* name, __f_int__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str)(VM*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_str(VM* vm, const char* mod, const char* name, __f_int__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool)(VM*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool(VM* vm, const char* mod, const char* name, __f_int__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(vm, _0, _1);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int_int)(VM*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_int(VM* vm, const char* mod, const char* name, __f_float__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float)(VM*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_float(VM* vm, const char* mod, const char* name, __f_float__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str)(VM*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_str(VM* vm, const char* mod, const char* name, __f_float__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool)(VM*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_bool(VM* vm, const char* mod, const char* name, __f_float__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int)(VM*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_int(VM* vm, const char* mod, const char* name, __f_float__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float)(VM*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_float(VM* vm, const char* mod, const char* name, __f_float__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str)(VM*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_str(VM* vm, const char* mod, const char* name, __f_float__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool)(VM*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_bool(VM* vm, const char* mod, const char* name, __f_float__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int)(VM*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_int(VM* vm, const char* mod, const char* name, __f_float__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float)(VM*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_float(VM* vm, const char* mod, const char* name, __f_float__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str)(VM*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_str(VM* vm, const char* mod, const char* name, __f_float__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool)(VM*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_bool(VM* vm, const char* mod, const char* name, __f_float__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int)(VM*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_int(VM* vm, const char* mod, const char* name, __f_float__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float)(VM*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_float(VM* vm, const char* mod, const char* name, __f_float__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str)(VM*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_str(VM* vm, const char* mod, const char* name, __f_float__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool)(VM*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool(VM* vm, const char* mod, const char* name, __f_float__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(vm, _0, _1);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int_int)(VM*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_int(VM* vm, const char* mod, const char* name, __f_str__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float)(VM*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_float(VM* vm, const char* mod, const char* name, __f_str__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str)(VM*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_str(VM* vm, const char* mod, const char* name, __f_str__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool)(VM*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_bool(VM* vm, const char* mod, const char* name, __f_str__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int)(VM*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_int(VM* vm, const char* mod, const char* name, __f_str__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float)(VM*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_float(VM* vm, const char* mod, const char* name, __f_str__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str)(VM*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_str(VM* vm, const char* mod, const char* name, __f_str__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool)(VM*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_bool(VM* vm, const char* mod, const char* name, __f_str__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int)(VM*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_int(VM* vm, const char* mod, const char* name, __f_str__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float)(VM*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_float(VM* vm, const char* mod, const char* name, __f_str__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str)(VM*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_str(VM* vm, const char* mod, const char* name, __f_str__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool)(VM*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_bool(VM* vm, const char* mod, const char* name, __f_str__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int)(VM*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_int(VM* vm, const char* mod, const char* name, __f_str__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float)(VM*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_float(VM* vm, const char* mod, const char* name, __f_str__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str)(VM*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_str(VM* vm, const char* mod, const char* name, __f_str__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool)(VM*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool(VM* vm, const char* mod, const char* name, __f_str__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(vm, _0, _1);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int_int)(VM*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_int(VM* vm, const char* mod, const char* name, __f_bool__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float)(VM*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_float(VM* vm, const char* mod, const char* name, __f_bool__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str)(VM*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_str(VM* vm, const char* mod, const char* name, __f_bool__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool)(VM*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool(VM* vm, const char* mod, const char* name, __f_bool__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int)(VM*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_int(VM* vm, const char* mod, const char* name, __f_bool__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float)(VM*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_float(VM* vm, const char* mod, const char* name, __f_bool__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str)(VM*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_str(VM* vm, const char* mod, const char* name, __f_bool__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool)(VM*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool(VM* vm, const char* mod, const char* name, __f_bool__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int)(VM*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_int(VM* vm, const char* mod, const char* name, __f_bool__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float)(VM*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_float(VM* vm, const char* mod, const char* name, __f_bool__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str)(VM*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_str(VM* vm, const char* mod, const char* name, __f_bool__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool)(VM*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool(VM* vm, const char* mod, const char* name, __f_bool__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int)(VM*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int(VM* vm, const char* mod, const char* name, __f_bool__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float)(VM*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float(VM* vm, const char* mod, const char* name, __f_bool__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str)(VM*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str(VM* vm, const char* mod, const char* name, __f_bool__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool)(VM*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(vm, _0, _1);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int_int)(VM*, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_int(VM* vm, const char* mod, const char* name, __f_None__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_float)(VM*, i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_float(VM* vm, const char* mod, const char* name, __f_None__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_str)(VM*, i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_str(VM* vm, const char* mod, const char* name, __f_None__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool)(VM*, i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_bool(VM* vm, const char* mod, const char* name, __f_None__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_int)(VM*, f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_int(VM* vm, const char* mod, const char* name, __f_None__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_float)(VM*, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_float(VM* vm, const char* mod, const char* name, __f_None__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_str)(VM*, f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_str(VM* vm, const char* mod, const char* name, __f_None__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool)(VM*, f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_bool(VM* vm, const char* mod, const char* name, __f_None__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_int)(VM*, const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_int(VM* vm, const char* mod, const char* name, __f_None__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_float)(VM*, const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_float(VM* vm, const char* mod, const char* name, __f_None__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_str)(VM*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_str(VM* vm, const char* mod, const char* name, __f_None__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool)(VM*, const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_bool(VM* vm, const char* mod, const char* name, __f_None__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int)(VM*, bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_int(VM* vm, const char* mod, const char* name, __f_None__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float)(VM*, bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_float(VM* vm, const char* mod, const char* name, __f_None__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str)(VM*, bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_str(VM* vm, const char* mod, const char* name, __f_None__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool)(VM*, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool(VM* vm, const char* mod, const char* name, __f_None__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(vm, _0, _1);
        return vm->None;
    });
}

typedef i64 (*__f_int__int)(VM*, i64);
__EXPORT
void pkpy_vm_bind__f_int__int(VM* vm, const char* mod, const char* name, __f_int__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 ret = f(vm, _0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float)(VM*, f64);
__EXPORT
void pkpy_vm_bind__f_int__float(VM* vm, const char* mod, const char* name, __f_int__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 ret = f(vm, _0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str)(VM*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str(VM* vm, const char* mod, const char* name, __f_int__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 ret = f(vm, _0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool)(VM*, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool(VM* vm, const char* mod, const char* name, __f_int__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 ret = f(vm, _0);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int)(VM*, i64);
__EXPORT
void pkpy_vm_bind__f_float__int(VM* vm, const char* mod, const char* name, __f_float__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 ret = f(vm, _0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float)(VM*, f64);
__EXPORT
void pkpy_vm_bind__f_float__float(VM* vm, const char* mod, const char* name, __f_float__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 ret = f(vm, _0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str)(VM*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str(VM* vm, const char* mod, const char* name, __f_float__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 ret = f(vm, _0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool)(VM*, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool(VM* vm, const char* mod, const char* name, __f_float__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 ret = f(vm, _0);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int)(VM*, i64);
__EXPORT
void pkpy_vm_bind__f_str__int(VM* vm, const char* mod, const char* name, __f_str__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* ret = f(vm, _0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float)(VM*, f64);
__EXPORT
void pkpy_vm_bind__f_str__float(VM* vm, const char* mod, const char* name, __f_str__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* ret = f(vm, _0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str)(VM*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str(VM* vm, const char* mod, const char* name, __f_str__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* ret = f(vm, _0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool)(VM*, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool(VM* vm, const char* mod, const char* name, __f_str__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* ret = f(vm, _0);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int)(VM*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int(VM* vm, const char* mod, const char* name, __f_bool__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool ret = f(vm, _0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float)(VM*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float(VM* vm, const char* mod, const char* name, __f_bool__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool ret = f(vm, _0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str)(VM*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str(VM* vm, const char* mod, const char* name, __f_bool__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool ret = f(vm, _0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool)(VM*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool(VM* vm, const char* mod, const char* name, __f_bool__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool ret = f(vm, _0);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int)(VM*, i64);
__EXPORT
void pkpy_vm_bind__f_None__int(VM* vm, const char* mod, const char* name, __f_None__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f(vm, _0);
        return vm->None;
    });
}

typedef void (*__f_None__float)(VM*, f64);
__EXPORT
void pkpy_vm_bind__f_None__float(VM* vm, const char* mod, const char* name, __f_None__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f(vm, _0);
        return vm->None;
    });
}

typedef void (*__f_None__str)(VM*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str(VM* vm, const char* mod, const char* name, __f_None__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f(vm, _0);
        return vm->None;
    });
}

typedef void (*__f_None__bool)(VM*, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool(VM* vm, const char* mod, const char* name, __f_None__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f(vm, _0);
        return vm->None;
    });
}

typedef i64 (*__f_int__)(VM*);
__EXPORT
void pkpy_vm_bind__f_int__(VM* vm, const char* mod, const char* name, __f_int__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 ret = f(vm);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__)(VM*);
__EXPORT
void pkpy_vm_bind__f_float__(VM* vm, const char* mod, const char* name, __f_float__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 ret = f(vm);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__)(VM*);
__EXPORT
void pkpy_vm_bind__f_str__(VM* vm, const char* mod, const char* name, __f_str__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* ret = f(vm);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__)(VM*);
__EXPORT
void pkpy_vm_bind__f_bool__(VM* vm, const char* mod, const char* name, __f_bool__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool ret = f(vm);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__)(VM*);
__EXPORT
void pkpy_vm_bind__f_None__(VM* vm, const char* mod, const char* name, __f_None__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f(vm);
        return vm->None;
    });
}
}