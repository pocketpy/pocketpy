extern "C" {
typedef i64 (*__f_int__int_int_int)(i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_int_int(VM* vm, const char* mod, const char* name, __f_int__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        i64 ret = f(_0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float_float)(f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_float_float(VM* vm, const char* mod, const char* name, __f_int__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        i64 ret = f(_0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str_str)(const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_str_str(VM* vm, const char* mod, const char* name, __f_int__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        i64 ret = f(_0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool_bool)(bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_int__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        i64 ret = f(_0, _1, _2);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int_int_int)(i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_int_int(VM* vm, const char* mod, const char* name, __f_float__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f64 ret = f(_0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float_float)(f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_float_float(VM* vm, const char* mod, const char* name, __f_float__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f64 ret = f(_0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str_str)(const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_str_str(VM* vm, const char* mod, const char* name, __f_float__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f64 ret = f(_0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool_bool)(bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_float__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f64 ret = f(_0, _1, _2);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int_int_int)(i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_int_int(VM* vm, const char* mod, const char* name, __f_str__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        const char* ret = f(_0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float_float)(f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_float_float(VM* vm, const char* mod, const char* name, __f_str__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        const char* ret = f(_0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str_str)(const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_str_str(VM* vm, const char* mod, const char* name, __f_str__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        const char* ret = f(_0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool_bool)(bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_str__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        const char* ret = f(_0, _1, _2);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int_int_int)(i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_int_int(VM* vm, const char* mod, const char* name, __f_bool__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        bool ret = f(_0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float_float)(f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_float_float(VM* vm, const char* mod, const char* name, __f_bool__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        bool ret = f(_0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str_str)(const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_str_str(VM* vm, const char* mod, const char* name, __f_bool__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        bool ret = f(_0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool_bool)(bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        bool ret = f(_0, _1, _2);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int_int_int)(i64, i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_int_int(VM* vm, const char* mod, const char* name, __f_None__int_int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 _2 = vm->PyInt_AS_C(args[2]);
        f(_0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__float_float_float)(f64, f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_float_float(VM* vm, const char* mod, const char* name, __f_None__float_float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 _2 = vm->PyFloat_AS_C(args[2]);
        f(_0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__str_str_str)(const char*, const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_str_str(VM* vm, const char* mod, const char* name, __f_None__str_str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* _2 = vm->PyStr_AS_C(args[2]);
        f(_0, _1, _2);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool_bool)(bool, bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool_bool(VM* vm, const char* mod, const char* name, __f_None__bool_bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<3>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool _2 = vm->PyBool_AS_C(args[2]);
        f(_0, _1, _2);
        return vm->None;
    });
}

typedef i64 (*__f_int__int_int)(i64, i64);
__EXPORT
void pkpy_vm_bind__f_int__int_int(VM* vm, const char* mod, const char* name, __f_int__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_float)(i64, f64);
__EXPORT
void pkpy_vm_bind__f_int__int_float(VM* vm, const char* mod, const char* name, __f_int__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_str)(i64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__int_str(VM* vm, const char* mod, const char* name, __f_int__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__int_bool)(i64, bool);
__EXPORT
void pkpy_vm_bind__f_int__int_bool(VM* vm, const char* mod, const char* name, __f_int__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_int)(f64, i64);
__EXPORT
void pkpy_vm_bind__f_int__float_int(VM* vm, const char* mod, const char* name, __f_int__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_float)(f64, f64);
__EXPORT
void pkpy_vm_bind__f_int__float_float(VM* vm, const char* mod, const char* name, __f_int__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_str)(f64, const char*);
__EXPORT
void pkpy_vm_bind__f_int__float_str(VM* vm, const char* mod, const char* name, __f_int__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float_bool)(f64, bool);
__EXPORT
void pkpy_vm_bind__f_int__float_bool(VM* vm, const char* mod, const char* name, __f_int__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_int)(const char*, i64);
__EXPORT
void pkpy_vm_bind__f_int__str_int(VM* vm, const char* mod, const char* name, __f_int__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_float)(const char*, f64);
__EXPORT
void pkpy_vm_bind__f_int__str_float(VM* vm, const char* mod, const char* name, __f_int__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_str)(const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_int__str_str(VM* vm, const char* mod, const char* name, __f_int__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str_bool)(const char*, bool);
__EXPORT
void pkpy_vm_bind__f_int__str_bool(VM* vm, const char* mod, const char* name, __f_int__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_int)(bool, i64);
__EXPORT
void pkpy_vm_bind__f_int__bool_int(VM* vm, const char* mod, const char* name, __f_int__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_float)(bool, f64);
__EXPORT
void pkpy_vm_bind__f_int__bool_float(VM* vm, const char* mod, const char* name, __f_int__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_str)(bool, const char*);
__EXPORT
void pkpy_vm_bind__f_int__bool_str(VM* vm, const char* mod, const char* name, __f_int__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool_bool)(bool, bool);
__EXPORT
void pkpy_vm_bind__f_int__bool_bool(VM* vm, const char* mod, const char* name, __f_int__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        i64 ret = f(_0, _1);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int_int)(i64, i64);
__EXPORT
void pkpy_vm_bind__f_float__int_int(VM* vm, const char* mod, const char* name, __f_float__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_float)(i64, f64);
__EXPORT
void pkpy_vm_bind__f_float__int_float(VM* vm, const char* mod, const char* name, __f_float__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_str)(i64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__int_str(VM* vm, const char* mod, const char* name, __f_float__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__int_bool)(i64, bool);
__EXPORT
void pkpy_vm_bind__f_float__int_bool(VM* vm, const char* mod, const char* name, __f_float__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_int)(f64, i64);
__EXPORT
void pkpy_vm_bind__f_float__float_int(VM* vm, const char* mod, const char* name, __f_float__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_float)(f64, f64);
__EXPORT
void pkpy_vm_bind__f_float__float_float(VM* vm, const char* mod, const char* name, __f_float__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_str)(f64, const char*);
__EXPORT
void pkpy_vm_bind__f_float__float_str(VM* vm, const char* mod, const char* name, __f_float__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float_bool)(f64, bool);
__EXPORT
void pkpy_vm_bind__f_float__float_bool(VM* vm, const char* mod, const char* name, __f_float__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_int)(const char*, i64);
__EXPORT
void pkpy_vm_bind__f_float__str_int(VM* vm, const char* mod, const char* name, __f_float__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_float)(const char*, f64);
__EXPORT
void pkpy_vm_bind__f_float__str_float(VM* vm, const char* mod, const char* name, __f_float__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_str)(const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_float__str_str(VM* vm, const char* mod, const char* name, __f_float__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str_bool)(const char*, bool);
__EXPORT
void pkpy_vm_bind__f_float__str_bool(VM* vm, const char* mod, const char* name, __f_float__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_int)(bool, i64);
__EXPORT
void pkpy_vm_bind__f_float__bool_int(VM* vm, const char* mod, const char* name, __f_float__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_float)(bool, f64);
__EXPORT
void pkpy_vm_bind__f_float__bool_float(VM* vm, const char* mod, const char* name, __f_float__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_str)(bool, const char*);
__EXPORT
void pkpy_vm_bind__f_float__bool_str(VM* vm, const char* mod, const char* name, __f_float__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool_bool)(bool, bool);
__EXPORT
void pkpy_vm_bind__f_float__bool_bool(VM* vm, const char* mod, const char* name, __f_float__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f64 ret = f(_0, _1);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int_int)(i64, i64);
__EXPORT
void pkpy_vm_bind__f_str__int_int(VM* vm, const char* mod, const char* name, __f_str__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_float)(i64, f64);
__EXPORT
void pkpy_vm_bind__f_str__int_float(VM* vm, const char* mod, const char* name, __f_str__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_str)(i64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__int_str(VM* vm, const char* mod, const char* name, __f_str__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__int_bool)(i64, bool);
__EXPORT
void pkpy_vm_bind__f_str__int_bool(VM* vm, const char* mod, const char* name, __f_str__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_int)(f64, i64);
__EXPORT
void pkpy_vm_bind__f_str__float_int(VM* vm, const char* mod, const char* name, __f_str__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_float)(f64, f64);
__EXPORT
void pkpy_vm_bind__f_str__float_float(VM* vm, const char* mod, const char* name, __f_str__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_str)(f64, const char*);
__EXPORT
void pkpy_vm_bind__f_str__float_str(VM* vm, const char* mod, const char* name, __f_str__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float_bool)(f64, bool);
__EXPORT
void pkpy_vm_bind__f_str__float_bool(VM* vm, const char* mod, const char* name, __f_str__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_int)(const char*, i64);
__EXPORT
void pkpy_vm_bind__f_str__str_int(VM* vm, const char* mod, const char* name, __f_str__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_float)(const char*, f64);
__EXPORT
void pkpy_vm_bind__f_str__str_float(VM* vm, const char* mod, const char* name, __f_str__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_str)(const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_str__str_str(VM* vm, const char* mod, const char* name, __f_str__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str_bool)(const char*, bool);
__EXPORT
void pkpy_vm_bind__f_str__str_bool(VM* vm, const char* mod, const char* name, __f_str__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_int)(bool, i64);
__EXPORT
void pkpy_vm_bind__f_str__bool_int(VM* vm, const char* mod, const char* name, __f_str__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_float)(bool, f64);
__EXPORT
void pkpy_vm_bind__f_str__bool_float(VM* vm, const char* mod, const char* name, __f_str__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_str)(bool, const char*);
__EXPORT
void pkpy_vm_bind__f_str__bool_str(VM* vm, const char* mod, const char* name, __f_str__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool_bool)(bool, bool);
__EXPORT
void pkpy_vm_bind__f_str__bool_bool(VM* vm, const char* mod, const char* name, __f_str__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        const char* ret = f(_0, _1);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int_int)(i64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__int_int(VM* vm, const char* mod, const char* name, __f_bool__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_float)(i64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__int_float(VM* vm, const char* mod, const char* name, __f_bool__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_str)(i64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__int_str(VM* vm, const char* mod, const char* name, __f_bool__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__int_bool)(i64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__int_bool(VM* vm, const char* mod, const char* name, __f_bool__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_int)(f64, i64);
__EXPORT
void pkpy_vm_bind__f_bool__float_int(VM* vm, const char* mod, const char* name, __f_bool__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_float)(f64, f64);
__EXPORT
void pkpy_vm_bind__f_bool__float_float(VM* vm, const char* mod, const char* name, __f_bool__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_str)(f64, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__float_str(VM* vm, const char* mod, const char* name, __f_bool__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float_bool)(f64, bool);
__EXPORT
void pkpy_vm_bind__f_bool__float_bool(VM* vm, const char* mod, const char* name, __f_bool__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_int)(const char*, i64);
__EXPORT
void pkpy_vm_bind__f_bool__str_int(VM* vm, const char* mod, const char* name, __f_bool__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_float)(const char*, f64);
__EXPORT
void pkpy_vm_bind__f_bool__str_float(VM* vm, const char* mod, const char* name, __f_bool__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_str)(const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str_str(VM* vm, const char* mod, const char* name, __f_bool__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str_bool)(const char*, bool);
__EXPORT
void pkpy_vm_bind__f_bool__str_bool(VM* vm, const char* mod, const char* name, __f_bool__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_int)(bool, i64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_int(VM* vm, const char* mod, const char* name, __f_bool__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_float)(bool, f64);
__EXPORT
void pkpy_vm_bind__f_bool__bool_float(VM* vm, const char* mod, const char* name, __f_bool__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_str)(bool, const char*);
__EXPORT
void pkpy_vm_bind__f_bool__bool_str(VM* vm, const char* mod, const char* name, __f_bool__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool_bool)(bool, bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool_bool(VM* vm, const char* mod, const char* name, __f_bool__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        bool ret = f(_0, _1);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int_int)(i64, i64);
__EXPORT
void pkpy_vm_bind__f_None__int_int(VM* vm, const char* mod, const char* name, __f_None__int_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_float)(i64, f64);
__EXPORT
void pkpy_vm_bind__f_None__int_float(VM* vm, const char* mod, const char* name, __f_None__int_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_str)(i64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__int_str(VM* vm, const char* mod, const char* name, __f_None__int_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__int_bool)(i64, bool);
__EXPORT
void pkpy_vm_bind__f_None__int_bool(VM* vm, const char* mod, const char* name, __f_None__int_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_int)(f64, i64);
__EXPORT
void pkpy_vm_bind__f_None__float_int(VM* vm, const char* mod, const char* name, __f_None__float_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_float)(f64, f64);
__EXPORT
void pkpy_vm_bind__f_None__float_float(VM* vm, const char* mod, const char* name, __f_None__float_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_str)(f64, const char*);
__EXPORT
void pkpy_vm_bind__f_None__float_str(VM* vm, const char* mod, const char* name, __f_None__float_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__float_bool)(f64, bool);
__EXPORT
void pkpy_vm_bind__f_None__float_bool(VM* vm, const char* mod, const char* name, __f_None__float_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_int)(const char*, i64);
__EXPORT
void pkpy_vm_bind__f_None__str_int(VM* vm, const char* mod, const char* name, __f_None__str_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_float)(const char*, f64);
__EXPORT
void pkpy_vm_bind__f_None__str_float(VM* vm, const char* mod, const char* name, __f_None__str_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_str)(const char*, const char*);
__EXPORT
void pkpy_vm_bind__f_None__str_str(VM* vm, const char* mod, const char* name, __f_None__str_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__str_bool)(const char*, bool);
__EXPORT
void pkpy_vm_bind__f_None__str_bool(VM* vm, const char* mod, const char* name, __f_None__str_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_int)(bool, i64);
__EXPORT
void pkpy_vm_bind__f_None__bool_int(VM* vm, const char* mod, const char* name, __f_None__bool_int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 _1 = vm->PyInt_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_float)(bool, f64);
__EXPORT
void pkpy_vm_bind__f_None__bool_float(VM* vm, const char* mod, const char* name, __f_None__bool_float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 _1 = vm->PyFloat_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_str)(bool, const char*);
__EXPORT
void pkpy_vm_bind__f_None__bool_str(VM* vm, const char* mod, const char* name, __f_None__bool_str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* _1 = vm->PyStr_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef void (*__f_None__bool_bool)(bool, bool);
__EXPORT
void pkpy_vm_bind__f_None__bool_bool(VM* vm, const char* mod, const char* name, __f_None__bool_bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<2>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool _1 = vm->PyBool_AS_C(args[1]);
        f(_0, _1);
        return vm->None;
    });
}

typedef i64 (*__f_int__int)(i64);
__EXPORT
void pkpy_vm_bind__f_int__int(VM* vm, const char* mod, const char* name, __f_int__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        i64 ret = f(_0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__float)(f64);
__EXPORT
void pkpy_vm_bind__f_int__float(VM* vm, const char* mod, const char* name, __f_int__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        i64 ret = f(_0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__str)(const char*);
__EXPORT
void pkpy_vm_bind__f_int__str(VM* vm, const char* mod, const char* name, __f_int__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        i64 ret = f(_0);
        return vm->PyInt(ret);
    });
}

typedef i64 (*__f_int__bool)(bool);
__EXPORT
void pkpy_vm_bind__f_int__bool(VM* vm, const char* mod, const char* name, __f_int__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        i64 ret = f(_0);
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__int)(i64);
__EXPORT
void pkpy_vm_bind__f_float__int(VM* vm, const char* mod, const char* name, __f_float__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f64 ret = f(_0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__float)(f64);
__EXPORT
void pkpy_vm_bind__f_float__float(VM* vm, const char* mod, const char* name, __f_float__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f64 ret = f(_0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__str)(const char*);
__EXPORT
void pkpy_vm_bind__f_float__str(VM* vm, const char* mod, const char* name, __f_float__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f64 ret = f(_0);
        return vm->PyFloat(ret);
    });
}

typedef f64 (*__f_float__bool)(bool);
__EXPORT
void pkpy_vm_bind__f_float__bool(VM* vm, const char* mod, const char* name, __f_float__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f64 ret = f(_0);
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__int)(i64);
__EXPORT
void pkpy_vm_bind__f_str__int(VM* vm, const char* mod, const char* name, __f_str__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        const char* ret = f(_0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__float)(f64);
__EXPORT
void pkpy_vm_bind__f_str__float(VM* vm, const char* mod, const char* name, __f_str__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        const char* ret = f(_0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__str)(const char*);
__EXPORT
void pkpy_vm_bind__f_str__str(VM* vm, const char* mod, const char* name, __f_str__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        const char* ret = f(_0);
        return vm->PyStr(ret);
    });
}

typedef const char* (*__f_str__bool)(bool);
__EXPORT
void pkpy_vm_bind__f_str__bool(VM* vm, const char* mod, const char* name, __f_str__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        const char* ret = f(_0);
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__int)(i64);
__EXPORT
void pkpy_vm_bind__f_bool__int(VM* vm, const char* mod, const char* name, __f_bool__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        bool ret = f(_0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__float)(f64);
__EXPORT
void pkpy_vm_bind__f_bool__float(VM* vm, const char* mod, const char* name, __f_bool__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        bool ret = f(_0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__str)(const char*);
__EXPORT
void pkpy_vm_bind__f_bool__str(VM* vm, const char* mod, const char* name, __f_bool__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        bool ret = f(_0);
        return vm->PyBool(ret);
    });
}

typedef bool (*__f_bool__bool)(bool);
__EXPORT
void pkpy_vm_bind__f_bool__bool(VM* vm, const char* mod, const char* name, __f_bool__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        bool ret = f(_0);
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__int)(i64);
__EXPORT
void pkpy_vm_bind__f_None__int(VM* vm, const char* mod, const char* name, __f_None__int f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 _0 = vm->PyInt_AS_C(args[0]);
        f(_0);
        return vm->None;
    });
}

typedef void (*__f_None__float)(f64);
__EXPORT
void pkpy_vm_bind__f_None__float(VM* vm, const char* mod, const char* name, __f_None__float f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 _0 = vm->PyFloat_AS_C(args[0]);
        f(_0);
        return vm->None;
    });
}

typedef void (*__f_None__str)(const char*);
__EXPORT
void pkpy_vm_bind__f_None__str(VM* vm, const char* mod, const char* name, __f_None__str f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* _0 = vm->PyStr_AS_C(args[0]);
        f(_0);
        return vm->None;
    });
}

typedef void (*__f_None__bool)(bool);
__EXPORT
void pkpy_vm_bind__f_None__bool(VM* vm, const char* mod, const char* name, __f_None__bool f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<1>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool _0 = vm->PyBool_AS_C(args[0]);
        f(_0);
        return vm->None;
    });
}

typedef i64 (*__f_int__)();
__EXPORT
void pkpy_vm_bind__f_int__(VM* vm, const char* mod, const char* name, __f_int__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        i64 ret = f();
        return vm->PyInt(ret);
    });
}

typedef f64 (*__f_float__)();
__EXPORT
void pkpy_vm_bind__f_float__(VM* vm, const char* mod, const char* name, __f_float__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f64 ret = f();
        return vm->PyFloat(ret);
    });
}

typedef const char* (*__f_str__)();
__EXPORT
void pkpy_vm_bind__f_str__(VM* vm, const char* mod, const char* name, __f_str__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        const char* ret = f();
        return vm->PyStr(ret);
    });
}

typedef bool (*__f_bool__)();
__EXPORT
void pkpy_vm_bind__f_bool__(VM* vm, const char* mod, const char* name, __f_bool__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        bool ret = f();
        return vm->PyBool(ret);
    });
}

typedef void (*__f_None__)();
__EXPORT
void pkpy_vm_bind__f_None__(VM* vm, const char* mod, const char* name, __f_None__ f) {
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<0>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {
        f();
        return vm->None;
    });
}
}