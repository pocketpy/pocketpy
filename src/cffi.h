#pragma once

#include "vm.h"

namespace pkpy {

struct CType{
    PY_CLASS(CType, c, type_)

    const char* name;       // must be a literal
    const int size;
    const int index;
    constexpr CType(const char name[], int size, int index) : name(name), size(size), index(index) {}

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            CType& self = CAST(CType, args[0]);
            StrStream ss;
            ss << "<c._type '" << self.name << "' (" << self.size*8 << " bits)>";
            return VAR(ss.str());
        });
    }
};

constexpr CType kCTypes[] = {
    CType("char_", sizeof(char), 0), CType("int_", sizeof(int), 1),
    CType("float_", sizeof(float), 2), CType("double_", sizeof(double), 3),
    CType("bool_", sizeof(bool), 4), CType("void_", 1, 5),
    CType("int8_", sizeof(int8_t), 6), CType("int16_", sizeof(int16_t), 7),
    CType("int32_", sizeof(int32_t), 8), CType("int64_", sizeof(int64_t), 9), 
    CType("uint8_", sizeof(uint8_t), 10), CType("uint16_", sizeof(uint16_t), 11),
    CType("uint32_", sizeof(uint32_t), 12), CType("uint64_", sizeof(uint64_t), 13),
    CType("void_p_", sizeof(intptr_t), 14),
    // use macro here to do extension
};

const int kCTypeCount = sizeof(kCTypes) / sizeof(CType);

constexpr int C_TYPE(const char name[]){
    for(int k=0; k<kCTypeCount; k++){
        const char* i = kCTypes[k].name;
        const char* j = name;
        while(*i && *j && *i == *j) { i++; j++;}
        if(*i == *j) return k;
    }
    UNREACHABLE();
}

#define C_TYPE_T(x) (kCTypes[C_TYPE(x)])

struct Pointer{
    PY_CLASS(Pointer, c, ptr_)

    void* ptr;
    CType ctype;       // base type

    Pointer(void* ptr, CType ctype) : ptr(ptr), ctype(ctype) {}

    Pointer operator+(i64 offset) const {
        return Pointer((int8_t*)ptr + offset * ctype.size, ctype);
    }

    Pointer operator-(i64 offset) const {
        return Pointer((int8_t*)ptr - offset * ctype.size, ctype);
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            StrStream ss;
            ss << "<" << self.ctype.name << "* at " << (i64)self.ptr << ">";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "__add__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            return VAR_T(Pointer, self + CAST(i64, args[1]));
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            return VAR_T(Pointer, self - CAST_V(i64, args[1]));
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            Pointer& other = CAST(Pointer, args[1]);
            return VAR(self.ptr == other.ptr);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            Pointer& other = CAST(Pointer, args[1]);
            return VAR(self.ptr != other.ptr);
        });

        // https://docs.python.org/zh-cn/3/library/ctypes.html
        vm->bind_method<1>(type, "__getitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            i64 index = CAST_V(i64, args[1]);
            return (self+index).get(vm);
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            i64 index = CAST_V(i64, args[1]);
            (self+index).set(vm, args[2]);
            return vm->None;
        });

        vm->bind_method<1>(type, "cast", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            CType& ctype = CAST(CType, args[1]);
            return VAR_T(Pointer, self.ptr, ctype);
        });

        vm->bind_method<0>(type, "get", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            return self.get(vm);
        });

        vm->bind_method<1>(type, "set", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer, args[0]);
            self.set(vm, args[1]);
            return vm->None;
        });
    }

    template<typename T>
    inline T& ref() noexcept { return *reinterpret_cast<T*>(ptr); }

    template<typename TP>
    inline TP cast() noexcept {
        static_assert(std::is_pointer_v<TP>);
        return reinterpret_cast<TP>(ptr);
    }

    PyVar get(VM* vm){
        switch(ctype.index){
            case C_TYPE("char_"): return VAR(ref<char>());
            case C_TYPE("int_"): return VAR(ref<int>());
            case C_TYPE("float_"): return VAR(ref<float>());
            case C_TYPE("double_"): return VAR(ref<double>());
            case C_TYPE("bool_"): return VAR(ref<bool>());
            case C_TYPE("void_"): vm->ValueError("cannot get void*"); break;
            case C_TYPE("int8_"): return VAR(ref<int8_t>());
            case C_TYPE("int16_"): return VAR(ref<int16_t>());
            case C_TYPE("int32_"): return VAR(ref<int32_t>());
            case C_TYPE("int64_"): return VAR(ref<int64_t>());
            case C_TYPE("uint8_"): return VAR(ref<uint8_t>());
            case C_TYPE("uint16_"): return VAR(ref<uint16_t>());
            case C_TYPE("uint32_"): return VAR(ref<uint32_t>());
            case C_TYPE("uint64_"): return VAR(ref<uint64_t>());
            case C_TYPE("void_p_"): return VAR_T(Pointer, ref<void*>(), C_TYPE_T("void_"));
            // use macro here to do extension
            default: UNREACHABLE();
        }
        return vm->None;
    }

    void set(VM* vm, const PyVar& val){
        switch(ctype.index){
            case C_TYPE("char_"): ref<char>() = CAST_V(i64, val); break;
            case C_TYPE("int_"): ref<int>() = CAST_V(i64, val); break;
            case C_TYPE("float_"): ref<float>() = CAST_V(f64, val); break;
            case C_TYPE("double_"): ref<double>() = CAST_V(f64, val); break;
            case C_TYPE("bool_"): ref<bool>() = CAST_V(bool, val); break;
            case C_TYPE("void_"): vm->ValueError("cannot set void*"); break;
            case C_TYPE("int8_"): ref<int8_t>() = CAST_V(i64, val); break;
            case C_TYPE("int16_"): ref<int16_t>() = CAST_V(i64, val); break;
            case C_TYPE("int32_"): ref<int32_t>() = CAST_V(i64, val); break;
            case C_TYPE("int64_"): ref<int64_t>() = CAST_V(i64, val); break;
            case C_TYPE("uint8_"): ref<uint8_t>() = CAST_V(i64, val); break;
            case C_TYPE("uint16_"): ref<uint16_t>() = CAST_V(i64, val); break;
            case C_TYPE("uint32_"): ref<uint32_t>() = CAST_V(i64, val); break;
            case C_TYPE("uint64_"): ref<uint64_t>() = CAST_V(i64, val); break;
            case C_TYPE("void_p_"): ref<void*>() = CAST(Pointer, val).ptr; break;
            // use macro here to do extension
            default: UNREACHABLE();
        }
    }
};

struct StructMemberInfo {
    int offset;
    CType type;
};

struct StructMetaInfo {
    Str name;
    std::map<StrName, StructMemberInfo> members;
};

struct Point2{
    int x;
    int y;
};

static const StructMetaInfo _Point2_info = {
    "Point2",
    {
        {StrName("x"), {offsetof(Point2, x), C_TYPE_T("int_")}},
        {StrName("y"), {offsetof(Point2, y), C_TYPE_T("int_")}},
    }
};

struct Struct {
    PY_CLASS(Struct, c, struct_)

    const StructMetaInfo* info;
    int8_t* _data;      // store any `struct`

    Struct(const StructMetaInfo* info, int8_t* data) : info(info), _data(data) {}
    Struct(){
        info = &_Point2_info;
        _data = new int8_t[sizeof(Point2)];
    }
    ~Struct(){ delete[] _data; }

    Pointer address(VM* vm, StrName name){
        auto it = info->members.find(name);
        if(it == info->members.end()) vm->AttributeError("struct " + info->name + " has no member " + name.str());
        const StructMemberInfo& info = it->second;
        return {_data+info.offset, info.type};
    }

    PyVarOrNull __getattr__(VM* vm, StrName name){
        return address(vm, name).get(vm);
    }

    void __setattr__(VM* vm, StrName name, const PyVar& val){
        address(vm, name).set(vm, val);
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", [](VM* vm, Args& args) {
            return VAR_T(Struct);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Struct& self = CAST(Struct, args[0]);
            StrStream ss;
            ss << self.info->name << "(" << ")";
            return VAR(ss.str());
        });
    }
};

void add_module_c(VM* vm){
    PyVar mod = vm->new_module("c");
    PyVar ptr_t = Pointer::register_class(vm, mod);
    CType::register_class(vm, mod);
    Struct::register_class(vm, mod);

    for(int i=0; i<kCTypeCount; i++){
        vm->setattr(mod, kCTypes[i].name, VAR_T(CType, kCTypes[i]));
    }
    vm->setattr(mod, "nullptr", VAR_T(Pointer, nullptr, C_TYPE_T("void_")));

    vm->bind_func<1>(mod, "malloc", [](VM* vm, Args& args) {
        i64 size = CAST_V(i64, args[0]);
        return VAR_T(Pointer, malloc(size), C_TYPE_T("void_"));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, Args& args) {
        Pointer& self = CAST(Pointer, args[0]);
        free(self.ptr);
        return vm->None;
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, Args& args) {
        CType& ctype = CAST(CType, args[0]);
        return VAR(ctype.size);
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer, args[0]);
        Pointer& src = CAST(Pointer, args[1]);
        i64 size = CAST_V(i64, args[2]);
        memcpy(dst.ptr, src.ptr, size);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer, args[0]);
        i64 val = CAST_V(i64, args[1]);
        i64 size = CAST_V(i64, args[2]);
        memset(dst.ptr, (int)val, size);
        return vm->None;
    });

    vm->bind_func<1>(mod, "strdup", [ptr_t](VM* vm, Args& args) {
        if(is_type(args[0], vm->tp_str)){
            const Str& s = CAST(Str, args[0]);
            return VAR_T(Pointer, strdup(s.c_str()), C_TYPE_T("char_"));
        }else if(is_type(args[0], OBJ_GET(Type, ptr_t))){
            Pointer& p = CAST(Pointer, args[0]);
            return VAR_T(Pointer, strdup(p.cast<char*>()), C_TYPE_T("char_"));
        }else{
            vm->TypeError("strdup() argument must be 'str' or 'char*'");
            return vm->None;
        }
    });

    vm->bind_func<2>(mod, "strcmp", [](VM* vm, Args& args) {
        Pointer& p1 = CAST(Pointer, args[0]);
        Pointer& p2 = CAST(Pointer, args[1]);
        return VAR(strcmp(p1.cast<char*>(), p2.cast<char*>()));
    });

    vm->bind_func<1>(mod, "strlen", [](VM* vm, Args& args) {
        Pointer& p = CAST(Pointer, args[0]);
        return VAR(strlen(p.cast<char*>()));
    });
}

}   // namespace pkpy