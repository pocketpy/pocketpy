#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy {

template<typename Ret, typename... Params>
struct NativeProxyFunc {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(*)(Params...);
    _Fp func;
    NativeProxyFunc(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, Args& args) {
        if (args.size() != N) {
            vm->TypeError("expected " + std::to_string(N) + " arguments, but got " + std::to_string(args.size()));
        }
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyObject*> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        func(py_cast<Params>(vm, args[Is])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyObject*> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        __Ret ret = func(py_cast<Params>(vm, args[Is])...);
        return VAR(std::move(ret));
    }
};

template<typename Ret, typename T, typename... Params>
struct NativeProxyMethod {
    static constexpr int N = sizeof...(Params);
    using _Fp = Ret(T::*)(Params...);
    _Fp func;
    NativeProxyMethod(_Fp func) : func(func) {}

    PyObject* operator()(VM* vm, Args& args) {
        int actual_size = args.size() - 1;
        if (actual_size != N) {
            vm->TypeError("expected " + std::to_string(N) + " arguments, but got " + std::to_string(actual_size));
        }
        return call<Ret>(vm, args, std::make_index_sequence<N>());
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<std::is_void_v<__Ret>, PyObject*> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        T& self = py_cast<T&>(vm, args[0]);
        (self.*func)(py_cast<Params>(vm, args[Is+1])...);
        return vm->None;
    }

    template<typename __Ret, size_t... Is>
    std::enable_if_t<!std::is_void_v<__Ret>, PyObject*> call(VM* vm, Args& args, std::index_sequence<Is...>) {
        T& self = py_cast<T&>(vm, args[0]);
        __Ret ret = (self.*func)(py_cast<Params>(vm, args[Is+1])...);
        return VAR(std::move(ret));
    }
};

template<typename Ret, typename... Params>
auto native_proxy_callable(Ret(*func)(Params...)) {
    return NativeProxyFunc<Ret, Params...>(func);
}

template<typename Ret, typename T, typename... Params>
auto native_proxy_callable(Ret(T::*func)(Params...)) {
    return NativeProxyMethod<Ret, T, Params...>(func);
}


template<typename T>
constexpr int type_index() { return 0; }
template<> constexpr int type_index<void>() { return 1; }
template<> constexpr int type_index<char>() { return 2; }
template<> constexpr int type_index<short>() { return 3; }
template<> constexpr int type_index<int>() { return 4; }
template<> constexpr int type_index<long>() { return 5; }
template<> constexpr int type_index<long long>() { return 6; }
template<> constexpr int type_index<unsigned char>() { return 7; }
template<> constexpr int type_index<unsigned short>() { return 8; }
template<> constexpr int type_index<unsigned int>() { return 9; }
template<> constexpr int type_index<unsigned long>() { return 10; }
template<> constexpr int type_index<unsigned long long>() { return 11; }
template<> constexpr int type_index<float>() { return 12; }
template<> constexpr int type_index<double>() { return 13; }
template<> constexpr int type_index<bool>() { return 14; }

template<typename T>
struct TypeId{ inline static int id; };

struct TypeInfo;

struct MemberInfo{
    const TypeInfo* type;
    int offset;
};

struct TypeInfo{
    const char* name;
    int size;
    int index;
    std::map<StrName, MemberInfo> members;
};

struct Vec2 {
    float x, y;
};

struct TypeDB{
    std::vector<TypeInfo> _by_index;
    std::map<std::string_view, int> _by_name;

    template<typename T>
    void register_type(const char name[], std::map<StrName, MemberInfo>&& members){
        TypeInfo ti;
        ti.name = name;
        if constexpr(std::is_same_v<T, void>) ti.size = 1;
        else ti.size = sizeof(T);
        ti.members = std::move(members);
        TypeId<T>::id = ti.index = _by_index.size()+1;    // 0 is reserved for NULL
        _by_name[name] = ti.index;
        _by_index.push_back(ti);
    }

    const TypeInfo* get(int index) const {
        return index == 0 ? nullptr : &_by_index[index-1];
    }

    const TypeInfo* get(std::string_view name) const {
        auto it = _by_name.find(name);
        if(it == _by_name.end()) return nullptr;
        return get(it->second);
    }

    const TypeInfo* get(const Str& s) const {
        return get(s.sv());
    }

    template<typename T>
    const TypeInfo* get() const {
        return get(TypeId<std::decay_t<T>>::id);
    }
};

static TypeDB _type_db;


inline static auto ___x = [](){
    #define REGISTER_BASIC_TYPE(T) _type_db.register_type<T>(#T, {});
    _type_db.register_type<void>("void", {});
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

    _type_db.register_type<Vec2>("Vec2", {
        {"x", { _type_db.get<float>(), offsetof(Vec2, x) }},
        {"y", { _type_db.get<float>(), offsetof(Vec2, y) }},
    });
    return 0;
}();

struct Pointer{
    PY_CLASS(Pointer, c, _ptr)

    const TypeInfo* ctype;      // this is immutable
    int level;                  // level of pointer
    char* ptr;

    i64 unit_size() const {
        return level == 1 ? ctype->size : sizeof(void*);
    }

    Pointer() : ctype(_type_db.get<void>()), level(1), ptr(nullptr) {}
    Pointer(const TypeInfo* ctype, int level, char* ptr): ctype(ctype), level(level), ptr(ptr) {}
    Pointer(const TypeInfo* ctype, char* ptr): ctype(ctype), level(1), ptr(ptr) {}

    Pointer operator+(i64 offset) const { 
        return Pointer(ctype, level, ptr+offset*unit_size());
    }

    Pointer operator-(i64 offset) const { 
        return Pointer(ctype, level, ptr-offset*unit_size());
    }

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<-1>(type, "__new__", CPP_NOT_IMPLEMENTED());

        vm->bind_method<0>(type, "__repr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            std::stringstream ss;
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

        vm->bind_method<1>(type, "__getattr__", [](VM* vm, Args& args) {
            Pointer& self = CAST(Pointer&, args[0]);
            const Str& name = CAST(Str&, args[1]);
            return VAR_T(Pointer, self._to(vm, name));
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
    T& ref() noexcept { return *reinterpret_cast<T*>(ptr); }

    PyObject* get(VM* vm){
        if(level > 1) return VAR_T(Pointer, ctype, level-1, ref<char*>());
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

    void set(VM* vm, PyObject* val){
        if(level > 1) {
            Pointer& p = CAST(Pointer&, val);
            ref<char*>() = p.ptr;   // We don't check the type, just copy the underlying address
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
            default: FATAL_ERROR();
        }
    }

    Pointer _to(VM* vm, StrName name){
        auto it = ctype->members.find(name);
        if(it == ctype->members.end()){
            vm->AttributeError(fmt("struct '", ctype->name, "' has no member ", name.escape()));
        }
        const MemberInfo& info = it->second;
        return {info.type, level, ptr+info.offset};
    }
};


struct CType{
    PY_CLASS(CType, c, ctype)

    const TypeInfo* type;

    CType() : type(_type_db.get<void>()) {}
    CType(const TypeInfo* type) : type(type) {}

    static void _register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_static_method<1>(type, "__new__", [](VM* vm, Args& args) {
            const Str& name = CAST(Str&, args[0]);
            const TypeInfo* type = _type_db.get(name);
            if(type == nullptr) vm->TypeError("unknown type: " + name.escape());
            return VAR_T(CType, type);
        });
    }
};

inline void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    Pointer::register_class(vm, mod);
    CType::register_class(vm, mod);

    vm->setattr(mod, "nullptr", VAR_T(Pointer));

    vm->bind_func<1>(mod, "malloc", [](VM* vm, Args& args) {
        i64 size = CAST(i64, args[0]);
        return VAR_T(Pointer, _type_db.get<void>(), (char*)malloc(size));
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

    vm->bind_func<2>(mod, "cast", [](VM* vm, Args& args) {
        Pointer& self = CAST(Pointer&, args[0]);
        const Str& name = CAST(Str&, args[1]);
        int level = 0;
        for(int i=name.length()-1; i>=0; i--){
            if(name[i] == '*') level++;
            else break;
        }
        if(level == 0) vm->TypeError("expect a pointer type, such as 'int*'");
        Str type_s = name.substr(0, name.length()-level);
        const TypeInfo* type = _type_db.get(type_s);
        if(type == nullptr) vm->TypeError("unknown type: " + type_s.escape());
        return VAR_T(Pointer, type, level, self.ptr);
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, Args& args) {
        const Str& name = CAST(Str&, args[0]);
        if(name.index("*") != -1) return VAR(sizeof(void*));
        const TypeInfo* type = _type_db.get(name);
        if(type == nullptr) vm->TypeError("unknown type: " + name.escape());
        return VAR(type->size);
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, Args& args) {
        Pointer& dst = CAST(Pointer&, args[0]);
        i64 val = CAST(i64, args[1]);
        i64 size = CAST(i64, args[2]);
        memset(dst.ptr, (int)val, size);
        return vm->None;
    });
}

inline PyObject* py_var(VM* vm, void* p){
    return VAR_T(Pointer, _type_db.get<void>(), (char*)p);
}

inline PyObject* py_var(VM* vm, char* p){
    return VAR_T(Pointer, _type_db.get<char>(), (char*)p);
}

/***********************************************/

template<typename T>
struct _pointer {
    static constexpr int level = 0;
    using baseT = T;
};

template<typename T>
struct _pointer<T*> {
    static constexpr int level = _pointer<T>::level + 1;
    using baseT = typename _pointer<T>::baseT;
};

template<typename T>
struct pointer {
    static constexpr int level = _pointer<std::decay_t<T>>::level;
    using baseT = typename _pointer<std::decay_t<T>>::baseT;
};

template<typename T>
T py_pointer_cast(VM* vm, PyObject* var){
    static_assert(std::is_pointer_v<T>);
    Pointer& p = CAST(Pointer&, var);
    const TypeInfo* type = _type_db.get<typename pointer<T>::baseT>();
    const int level = pointer<T>::level;
    if(p.ctype != type || p.level != level){
        vm->TypeError("invalid pointer cast");
    }
    return reinterpret_cast<T>(p.ptr);
}

template<typename T>
std::enable_if_t<std::is_pointer_v<std::decay_t<T>>, PyObject*>
py_var(VM* vm, T p){
    const TypeInfo* type = _type_db.get<typename pointer<T>::baseT>();
    if(type == nullptr) type = _type_db.get<void>();
    return VAR_T(Pointer, type, pointer<T>::level, (char*)p);
}
}   // namespace pkpy