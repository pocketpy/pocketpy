#pragma once

#include "vm.h"

struct CType{
    PY_CLASS(c, type)

    const char* name;       // must be a literal
    int size;        
    constexpr CType(const char name[], int size) : name(name), size(size) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, pkpy::Args& args) {
            CType& self = vm->py_cast<CType>(args[0]);
            StrStream ss;
            ss << "<c.type '" << self.name << "'>";
            return vm->PyStr(ss.str());
        });
    }
};

constexpr CType kCTypes[] = {
    CType("int8", 1), CType("int16", 2), CType("int32", 4), CType("int64", 8),
    CType("uint8", 1), CType("uint16", 2), CType("uint32", 4), CType("uint64", 8),
    CType("float32", 4), CType("float64", 8), CType("bool8", 1), CType("void", 0),
};

const int kCTypeCount = sizeof(kCTypes) / sizeof(CType);

constexpr int ctype(const char name[]){
    for(int k=0; k<kCTypeCount; k++){
        const char* i = kCTypes[k].name;
        const char* j = name;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    UNREACHABLE();
}

constexpr CType ctype_t(const char name[]){
    return kCTypes[ctype(name)];
}

struct Pointer{
    PY_CLASS(c, ptr)

    void* ptr;
    CType _ctype;

    Pointer(void* ptr, CType ctype) : ptr(ptr), _ctype(ctype) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, pkpy::Args& args) {
            Pointer& self = vm->py_cast<Pointer>(args[0]);
            StrStream ss;
            ss << "<" << self._ctype.name << "* at " << std::hex << self.ptr << ">";
            return vm->PyStr(ss.str());
        });

        vm->bind_method<1>(type, "__getitem__", [](VM* vm, pkpy::Args& args) {
            Pointer& self = vm->py_cast<Pointer>(args[0]);
            i64 index = vm->PyInt_AS_C(args[1]);
            switch(ctype(self._ctype.name)){        // TODO: optimize
                case ctype("int8"): return vm->PyInt(((int8_t*)self.ptr)[index]);
                case ctype("int16"): return vm->PyInt(((int16_t*)self.ptr)[index]);
                case ctype("int32"): return vm->PyInt(((int32_t*)self.ptr)[index]);
                case ctype("int64"): return vm->PyInt(((int64_t*)self.ptr)[index]);
                case ctype("uint8"): return vm->PyInt(((uint8_t*)self.ptr)[index]);
                case ctype("uint16"): return vm->PyInt(((uint16_t*)self.ptr)[index]);
                case ctype("uint32"): return vm->PyInt(((uint32_t*)self.ptr)[index]);
                case ctype("uint64"): return vm->PyInt(((uint64_t*)self.ptr)[index]);
                case ctype("float32"): return vm->PyFloat(((float*)self.ptr)[index]);
                case ctype("float64"): return vm->PyFloat(((double*)self.ptr)[index]);
                case ctype("bool8"): return vm->PyBool(((bool*)self.ptr)[index]);
                case ctype("void"): vm->TypeError("cannot index void*");
                default: UNREACHABLE();
            }
            return vm->None;
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, pkpy::Args& args) {
            Pointer& self = vm->py_cast<Pointer>(args[0]);
            i64 index = vm->PyInt_AS_C(args[1]);
            switch(ctype(self._ctype.name)){        // TODO: optimize
                case ctype("int8"): ((int8_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("int16"): ((int16_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("int32"): ((int32_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("int64"): ((int64_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("uint8"): ((uint8_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("uint16"): ((uint16_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("uint32"): ((uint32_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("uint64"): ((uint64_t*)self.ptr)[index] = vm->PyInt_AS_C(args[2]); break;
                case ctype("float32"): ((float*)self.ptr)[index] = vm->PyFloat_AS_C(args[2]); break;
                case ctype("float64"): ((double*)self.ptr)[index] = vm->PyFloat_AS_C(args[2]); break;
                case ctype("bool8"): ((bool*)self.ptr)[index] = vm->PyBool_AS_C(args[2]); break;
                case ctype("void"): vm->TypeError("cannot index void*");
                default: UNREACHABLE();
            }
            return vm->None;
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
    vm->register_class<CType>(mod);

    for(int i=0; i<kCTypeCount; i++){
        vm->setattr(mod, kCTypes[i].name, vm->new_object<CType>(kCTypes[i]));
    }

    vm->bind_func<1>(mod, "malloc", [](VM* vm, pkpy::Args& args) {
        i64 size = vm->PyInt_AS_C(args[0]);
        return vm->new_object<Pointer>(malloc(size), ctype_t("void"));
    });

    vm->bind_func<2>(mod, "cast", [](VM* vm, pkpy::Args& args) {
        Pointer& self = vm->py_cast<Pointer>(args[0]);
        CType& ctype = vm->py_cast<CType>(args[1]);
        return vm->new_object<Pointer>(self.ptr, ctype);
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
            return vm->new_object<Pointer>(strdup(s.c_str()), ctype_t("char"));
        }else if(is_type(args[0], OBJ_GET(Type, ptr_t))){
            Pointer& p = vm->py_cast<Pointer>(args[0]);
            return vm->new_object<Pointer>(strdup(p.cast<char*>()), ctype_t("char"));
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