#pragma once

#include "common.h"
#include "vm.h"
#include <type_traits>
#include <vector>

namespace pkpy {

template<typename Ret, typename... Params>
struct NativeProxyFunc {
    using T = Ret(*)(Params...);
    static constexpr int N = sizeof...(Params);
    T func;
    NativeProxyFunc(T func) : func(func) {}

    PyVar operator()(VM* vm, Args& args) {
        if (args.size() != N) vm->TypeError("invalid number of arguments");
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyVar> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        func(py_cast<Params>(vm, args[Is])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyVar> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        __Ret ret = func(py_cast<Params>(vm, args[Is])...);
        return VAR(std::move(ret));
    }
};

struct TypeInfo;

struct MemberInfo{
    TypeInfo* type;
    int offset;
};

struct TypeInfo{
    const char* name;
    int size;
    int index;      // for basic types only
    std::map<StrName, MemberInfo> members;

    TypeInfo(const char name[], int size, int index) : name(name), size(size), index(index) {}
    TypeInfo(const char name[], int size, std::map<StrName, MemberInfo> members)
        : name(name), size(size), index(-1), members(members) {}
    TypeInfo() : name(nullptr), size(0), index(-1) {}
};

template<typename T>
constexpr int type_index() { return -1; }
template<> constexpr int type_index<void>() { return 0; }
template<> constexpr int type_index<char>() { return 1; }
template<> constexpr int type_index<short>() { return 2; }
template<> constexpr int type_index<int>() { return 3; }
template<> constexpr int type_index<long>() { return 4; }
template<> constexpr int type_index<long long>() { return 5; }
template<> constexpr int type_index<unsigned char>() { return 6; }
template<> constexpr int type_index<unsigned short>() { return 7; }
template<> constexpr int type_index<unsigned int>() { return 8; }
template<> constexpr int type_index<unsigned long>() { return 9; }
template<> constexpr int type_index<unsigned long long>() { return 10; }
template<> constexpr int type_index<float>() { return 11; }
template<> constexpr int type_index<double>() { return 12; }
template<> constexpr int type_index<bool>() { return 13; }

struct Vec2 {
    float x, y;
};

static std::map<std::string_view, TypeInfo> _type_infos;

auto _ = [](){
    #define REGISTER_BASIC_TYPE(T) _type_infos[#T] = TypeInfo(#T, sizeof(T), type_index<T>())
    _type_infos["void"] = TypeInfo("void", 1, type_index<void>());
    REGISTER_BASIC_TYPE(char);
    REGISTER_BASIC_TYPE(short);
    REGISTER_BASIC_TYPE(int);
    REGISTER_BASIC_TYPE(long);
    REGISTER_BASIC_TYPE(long long);
    REGISTER_BASIC_TYPE(unsigned char);
    REGISTER_BASIC_TYPE(unsigned short);
    REGISTER_BASIC_TYPE(unsigned int);
    REGISTER_BASIC_TYPE(unsigned long);
    REGISTER_BASIC_TYPE(unsigned long long);
    REGISTER_BASIC_TYPE(float);
    REGISTER_BASIC_TYPE(double);
    REGISTER_BASIC_TYPE(bool);
    #undef REGISTER_BASIC_TYPE

    _type_infos["Vec2"] = TypeInfo("Vec2", sizeof(Vec2), {
        {"x", {&_type_infos["float"], offsetof(Vec2, x)}},
        {"y", {&_type_infos["float"], offsetof(Vec2, y)}},
    });
    return 0;
}();


struct Pointer{
    PY_CLASS(Pointer, c, ptr_)

    char* ptr;
    TypeInfo* ctype;
    int level;         // level of pointer

    i64 unit_size() const {
        return level == 1 ? ctype->size : sizeof(void*);
    }

    bool type_equal(const Pointer& other) const {
        return level == other.level && ctype == other.ctype;
    }

    Pointer() : ptr(nullptr), ctype(&_type_infos["void"]), level(1) {}
    Pointer(char* ptr, TypeInfo* ctype, int level=1) : ptr(ptr), ctype(ctype), level(level) {}

    Pointer operator+(i64 offset) const { 
        return Pointer(ptr + offset * unit_size(), ctype, level);
    }

    Pointer operator-(i64 offset) const { 
        return Pointer(ptr - offset * unit_size(), ctype, level);
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            StrStream ss;
            ss << "<" << self.ctype->name;
            for(int i=0; i<self.level; i++) ss << "*";
            ss << " at " << (i64)self.ptr << ">";
            return VAR(ss.str());
        });

        vm->bind_method<1>(type, "__add__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return VAR_T(Pointer, self + CAST(i64, args[1]));
        });

        vm->bind_method<1>(type, "__sub__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return VAR_T(Pointer, self - CAST(i64, args[1]));
        });

        vm->bind_method<1>(type, "__eq__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            Pointer& other = CAST(Pointer&, args[1]);
            return VAR(self.ptr == other.ptr);
        });

        vm->bind_method<1>(type, "__ne__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            Pointer& other = CAST(Pointer&, args[1]);
            return VAR(self.ptr != other.ptr);
        });

        vm->bind_method<1>(type, "__getitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            i64 index = CAST(i64, args[1]);
            return (self+index).get(vm);
        });

        vm->bind_method<2>(type, "__setitem__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            i64 index = CAST(i64, args[1]);
            (self+index).set(vm, args[2]);
            return vm->None;
        });

        vm->bind_method<0>(type, "get", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            return self.get(vm);
        });

        vm->bind_method<1>(type, "set", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            self.set(vm, args[1]);
            return vm->None;
        });
    }

    template<typename T>
    inline T& ref() noexcept { return *reinterpret_cast<T*>(ptr); }

    PyVar get(VM* vm){
        if(level > 1) return VAR_T(Pointer, ptr, ctype, level-1);
        switch(ctype->index){
#define CASE(T) case type_index<T>(): return VAR(ref<T>())
            case type_index<void>(): vm->ValueError("cannot get void*"); break;
            CASE(char);
            CASE(short);
            CASE(int);
            CASE(long);
            CASE(long long);
            CASE(unsigned char);
            CASE(unsigned short);
            CASE(unsigned int);
            CASE(unsigned long);
            CASE(unsigned long long);
            CASE(float);
            CASE(double);
            CASE(bool);
#undef CASE
        }
        return VAR_T(Pointer, *this);
    }

    void set(VM* vm, const PyVar& val){
        if(level > 1) {
            Pointer& p = CAST(Pointer&, val);
            ref<void*>() = p.ptr;   // We don't check the type, just copy the underlying address
            return;
        }
        switch(ctype->index){
#define CASE(T1, T2) case type_index<T1>(): ref<T1>() = CAST(T2, val); break
            case type_index<void>(): vm->ValueError("cannot set void*"); break;
            CASE(char, i64);
            CASE(short, i64);
            CASE(int, i64);
            CASE(long, i64);
            CASE(long long, i64);
            CASE(unsigned char, i64);
            CASE(unsigned short, i64);
            CASE(unsigned int, i64);
            CASE(unsigned long, i64);
            CASE(unsigned long long, i64);
            CASE(float, f64);
            CASE(double, f64);
            CASE(bool, bool);
#undef CASE
        }
        UNREACHABLE();
    }

    Pointer address(VM* vm, StrName name){
        auto it = ctype->members.find(name);
        if(it == ctype->members.end()){
            vm->AttributeError(Str("struct '") + ctype->name + "' has no member " + name.str().escape(true));
        }
        MemberInfo& info = it->second;
        return {ptr+info.offset, info.type, level};
    }
};


struct Struct {
    PY_CLASS(Struct, c, struct_)

    char* data;
    Pointer head;

    TypeInfo* ctype() const { return head.ctype; }

    Struct(const Pointer& head) {
        data = new char[head.ctype->size];
        memcpy(data, head.ptr, head.ctype->size);
        this->head = Pointer(data, head.ctype, head.level);
    }

    static void _register(VM* vm, PyVar mod, PyVar type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Struct& self = CAST(Struct&, args[0]);
            StrStream ss;
            ss << self.ctype()->name << "(" << ")";
            return VAR(ss.str());
        });
    }
};

void add_module_c(VM* vm){
    PyVar mod = vm->new_module("c");
    PyVar ptr_t = Pointer::register_class(vm, mod);
    Struct::register_class(vm, mod);

    vm->setattr(mod, "nullptr", VAR_T(Pointer, nullptr, &_type_infos["void"]));

    vm->bind_func<1>(mod, "malloc", [](VM* vm, Args& args) {
        i64 size = CAST(i64, args[0]);
        return VAR_T(Pointer, (char*)malloc(size), &_type_infos["void"]);
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, Args& args) {
        Pointer& self = CAST(Pointer&, args[0]);
        free(self.ptr);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer&, args[0]);
        Pointer& src = CAST(Pointer&, args[1]);
        i64 size = CAST(i64, args[2]);
        memcpy(dst.ptr, src.ptr, size);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer&, args[0]);
        i64 val = CAST(i64, args[1]);
        i64 size = CAST(i64, args[2]);
        memset(dst.ptr, (int)val, size);
        return vm->None;
    });
}

PyVar py_var(VM* vm, void* p){
    return VAR_T(Pointer, (char*)p, &_type_infos["void"]);
}

PyVar py_var(VM* vm, char* p){
    return VAR_T(Pointer, (char*)p, &_type_infos["char"]);
}

}   // namespace pkpy