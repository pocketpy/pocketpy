#pragma once

#include "vm.h"

struct CType{
    PY_CLASS(c, type)

    StrName name;
    int size;
    CType(const StrName& name, int size) : name(name), size(size) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());
    }
};

struct Pointer{
    PY_CLASS(c, ptr)

    void* ptr;
    CType ctype;

    Pointer(void* ptr, CType ctype) : ptr(ptr), ctype(ctype) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, pkpy::Args& args) {
            Pointer& self = vm->py_cast<Pointer>(args[0]);
            StrStream ss;
            ss << "<" << self.ctype.name.str() << "* at " << std::hex << self.ptr << ">";
            return vm->PyStr(ss.str());
        });
    }

    template<class T>
    inline T cast() noexcept {
        return reinterpret_cast<T>(ptr);
    }
};

void add_module_c(VM* vm){
    PyVar mod = vm->new_module("c");
    PyVar ptr_t = vm->register_class<Pointer>(mod);
    PyVar ctype_t = vm->register_class<CType>(mod);

    vm->setattr(mod, "char", vm->new_object<CType>("char", 1));
    vm->setattr(mod, "int8", vm->new_object<CType>("int8", 1));
    vm->setattr(mod, "int16", vm->new_object<CType>("int16", 2));
    vm->setattr(mod, "int32", vm->new_object<CType>("int32", 4));
    vm->setattr(mod, "int64", vm->new_object<CType>("int64", 8));
    vm->setattr(mod, "uint8", vm->new_object<CType>("uchar", 1));
    vm->setattr(mod, "uint16", vm->new_object<CType>("uint16", 2));
    vm->setattr(mod, "uint32", vm->new_object<CType>("uint32", 4));
    vm->setattr(mod, "uint64", vm->new_object<CType>("uint64", 8));
    vm->setattr(mod, "float", vm->new_object<CType>("float", 4));
    vm->setattr(mod, "double", vm->new_object<CType>("double", 8));
    vm->setattr(mod, "bool", vm->new_object<CType>("bool", 1));

    vm->bind_func<1>(mod, "malloc", [](VM* vm, pkpy::Args& args) {
        i64 size = vm->PyInt_AS_C(args[0]);
        return vm->new_object<Pointer>(malloc(size), CType("char", 1));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, pkpy::Args& args) {
        Pointer& self = vm->py_cast<Pointer>(args[0]);
        free(self.ptr);
        return vm->None;
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, pkpy::Args& args) {
        CType& ctype = vm->py_cast<CType>(args[0]);
        return vm->PyInt(ctype.size);
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, pkpy::Args& args) {
        Pointer& dst = vm->py_cast<Pointer>(args[0]);
        Pointer& src = vm->py_cast<Pointer>(args[1]);
        i64 size = vm->PyInt_AS_C(args[2]);
        memcpy(dst.ptr, src.ptr, size);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, pkpy::Args& args) {
        Pointer& dst = vm->py_cast<Pointer>(args[0]);
        i64 val = vm->PyInt_AS_C(args[1]);
        i64 size = vm->PyInt_AS_C(args[2]);
        memset(dst.ptr, (int)val, size);
        return vm->None;
    });

    vm->bind_func<1>(mod, "strdup", [ptr_t](VM* vm, pkpy::Args& args) {
        if(is_type(args[0], vm->tp_str)){
            const Str& s = vm->PyStr_AS_C(args[0]);
            return vm->new_object<Pointer>(strdup(s.c_str()), CType("char", 1));
        }else if(is_type(args[0], OBJ_GET(Type, ptr_t))){
            Pointer& p = vm->py_cast<Pointer>(args[0]);
            return vm->new_object<Pointer>(strdup(p.cast<char*>()), CType("char", 1));
        }else{
            vm->TypeError("strdup() argument must be 'str' or 'c.ptr'");
            return vm->None;
        }
    });

    vm->bind_func<2>(mod, "strcmp", [](VM* vm, pkpy::Args& args) {
        Pointer& p1 = vm->py_cast<Pointer>(args[0]);
        Pointer& p2 = vm->py_cast<Pointer>(args[1]);
        return vm->PyInt(strcmp(p1.cast<char*>(), p2.cast<char*>()));
    });
}