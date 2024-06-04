#include "pocketpy/interpreter/cffi.hpp"

namespace pkpy {

void VoidP::_register(VM* vm, PyObject* mod, PyObject* type) {
    vm->bind_func(type, __new__, 2, [](VM* vm, ArgsView args) {
        Type cls = PK_OBJ_GET(Type, args[0]);
        i64 addr = CAST(i64, args[1]);
        return vm->new_object<VoidP>(cls, reinterpret_cast<void*>(addr));
    });

    vm->bind__hash__(type->as<Type>(), [](VM* vm, PyVar obj) {
        obj_get_t<VoidP> self = PK_OBJ_GET(VoidP, obj);
        return reinterpret_cast<i64>(self.ptr);
    });

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) -> Str {
        obj_get_t<VoidP> self = PK_OBJ_GET(VoidP, obj);
        return _S("<void* at ", self.hex(), ">");
    });

#define BIND_CMP(name, op)                                                                                             \
    vm->bind##name(type->as<Type>(), [](VM* vm, PyVar lhs, PyVar rhs) {                                                \
        if(!vm->isinstance(rhs, vm->_tp_user<VoidP>())) return vm->NotImplemented;                                     \
        void* _0 = PK_OBJ_GET(VoidP, lhs).ptr;                                                                         \
        void* _1 = PK_OBJ_GET(VoidP, rhs).ptr;                                                                         \
        return VAR(_0 op _1);                                                                                          \
    });

    BIND_CMP(__eq__, ==)
    BIND_CMP(__lt__, <)
    BIND_CMP(__le__, <=)
    BIND_CMP(__gt__, >)
    BIND_CMP(__ge__, >=)

#undef BIND_CMP
}

void Struct::_register(VM* vm, PyObject* mod, PyObject* type) {
    vm->bind_func(type, __new__, 2, [](VM* vm, ArgsView args) {
        Type cls = PK_OBJ_GET(Type, args[0]);
        int size = CAST(int, args[1]);
        return vm->new_object<Struct>(cls, size);
    });

    vm->bind_func(type, "hex", 1, [](VM* vm, ArgsView args) {
        const Struct& self = _CAST(Struct&, args[0]);
        SStream ss;
        for(int i = 0; i < self.size; i++)
            ss.write_hex((unsigned char)self.p[i]);
        return VAR(ss.str());
    });

    // @staticmethod
    vm->bind_func(
        type,
        "fromhex",
        1,
        [](VM* vm, ArgsView args) {
            const Str& s = CAST(Str&, args[0]);
            if(s.size < 2 || s.size % 2 != 0) vm->ValueError("invalid hex string");
            Struct buffer(s.size / 2, false);
            for(int i = 0; i < s.size; i += 2) {
                char c = 0;
                if(s[i] >= '0' && s[i] <= '9')
                    c += s[i] - '0';
                else if(s[i] >= 'A' && s[i] <= 'F')
                    c += s[i] - 'A' + 10;
                else if(s[i] >= 'a' && s[i] <= 'f')
                    c += s[i] - 'a' + 10;
                else
                    vm->ValueError(_S("invalid hex char: '", s[i], "'"));
                c <<= 4;
                if(s[i + 1] >= '0' && s[i + 1] <= '9')
                    c += s[i + 1] - '0';
                else if(s[i + 1] >= 'A' && s[i + 1] <= 'F')
                    c += s[i + 1] - 'A' + 10;
                else if(s[i + 1] >= 'a' && s[i + 1] <= 'f')
                    c += s[i + 1] - 'a' + 10;
                else
                    vm->ValueError(_S("invalid hex char: '", s[i + 1], "'"));
                buffer.p[i / 2] = c;
            }
            return vm->new_user_object<Struct>(std::move(buffer));
        },
        {},
        BindType::STATICMETHOD);

    vm->bind__repr__(type->as<Type>(), [](VM* vm, PyVar obj) {
        Struct& self = _CAST(Struct&, obj);
        SStream ss;
        ss << "<struct object of " << self.size << " bytes>";
        return ss.str();
    });

    vm->bind_func(type, "addr", 1, [](VM* vm, ArgsView args) {
        Struct& self = _CAST(Struct&, args[0]);
        return vm->new_user_object<VoidP>(self.p);
    });

    vm->bind_func(type, "sizeof", 1, [](VM* vm, ArgsView args) {
        Struct& self = _CAST(Struct&, args[0]);
        return VAR(self.size);
    });

    vm->bind_func(type, "copy", 1, [](VM* vm, ArgsView args) {
        const Struct& self = _CAST(Struct&, args[0]);
        return vm->new_object<Struct>(vm->_tp(args[0]), self);
    });

    vm->bind__eq__(type->as<Type>(), [](VM* vm, PyVar lhs, PyVar rhs) {
        Struct& self = _CAST(Struct&, lhs);
        if(!vm->is_user_type<Struct>(rhs)) return vm->NotImplemented;
        Struct& other = _CAST(Struct&, rhs);
        bool ok = self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
        return VAR(ok);
    });

#define BIND_SETGET(T, name)                                                                                           \
    vm->bind(type, "read_" name "(self, offset=0)", [](VM* vm, ArgsView args) {                                        \
        Struct& self = _CAST(Struct&, args[0]);                                                                        \
        i64 offset = CAST(i64, args[1]);                                                                               \
        void* ptr = self.p + offset;                                                                                   \
        return VAR(*(T*)ptr);                                                                                          \
    });                                                                                                                \
    vm->bind(type, "write_" name "(self, value, offset=0)", [](VM* vm, ArgsView args) {                                \
        Struct& self = _CAST(Struct&, args[0]);                                                                        \
        i64 offset = CAST(i64, args[2]);                                                                               \
        void* ptr = self.p + offset;                                                                                   \
        *(T*)ptr = CAST(T, args[1]);                                                                                   \
        return vm->None;                                                                                               \
    });
    BIND_SETGET(char, "char")
    BIND_SETGET(unsigned char, "uchar")
    BIND_SETGET(short, "short")
    BIND_SETGET(unsigned short, "ushort")
    BIND_SETGET(int, "int")
    BIND_SETGET(unsigned int, "uint")
    BIND_SETGET(long, "long")
    BIND_SETGET(unsigned long, "ulong")
    BIND_SETGET(long long, "longlong")
    BIND_SETGET(unsigned long long, "ulonglong")
    BIND_SETGET(float, "float")
    BIND_SETGET(double, "double")
    BIND_SETGET(bool, "bool")
    BIND_SETGET(void*, "void_p")
#undef BIND_SETGET
}

void add_module_c(VM* vm) {
    PyObject* mod = vm->new_module("c");

    vm->bind_func(mod, "malloc", 1, [](VM* vm, ArgsView args) {
        i64 size = CAST(i64, args[0]);
        return VAR(std::malloc(size));
    });

    vm->bind_func(mod, "free", 1, [](VM* vm, ArgsView args) {
        void* p = CAST(void*, args[0]);
        std::free(p);
        return vm->None;
    });

    vm->bind_func(mod, "memset", 3, [](VM* vm, ArgsView args) {
        void* p = CAST(void*, args[0]);
        std::memset(p, CAST(int, args[1]), CAST(size_t, args[2]));
        return vm->None;
    });

    vm->bind_func(mod, "memcpy", 3, [](VM* vm, ArgsView args) {
        void* dst = CAST(void*, args[0]);
        void* src = CAST(void*, args[1]);
        i64 size = CAST(i64, args[2]);
        std::memcpy(dst, src, size);
        return vm->None;
    });

    vm->register_user_class<VoidP>(mod, "void_p", VM::tp_object, true);
    vm->register_user_class<Struct>(mod, "struct", VM::tp_object, true);

    mod->attr().set("NULL", vm->new_user_object<VoidP>(nullptr));

    vm->bind(mod, "p_cast(ptr: 'void_p', cls: type[T]) -> T", [](VM* vm, ArgsView args) {
        VoidP& ptr = CAST(VoidP&, args[0]);
        vm->check_type(args[1], vm->tp_type);
        Type cls = PK_OBJ_GET(Type, args[1]);
        if(!vm->issubclass(cls, vm->_tp_user<VoidP>())) { vm->ValueError("expected a subclass of void_p"); }
        return vm->new_object<VoidP>(cls, ptr.ptr);
    });

    vm->bind(mod, "p_value(ptr: 'void_p') -> int", [](VM* vm, ArgsView args) {
        VoidP& ptr = CAST(VoidP&, args[0]);
        return VAR(reinterpret_cast<i64>(ptr.ptr));
    });

    vm->bind(mod, "pp_deref(ptr: Tp) -> Tp", [](VM* vm, ArgsView args) {
        VoidP& ptr = CAST(VoidP&, args[0]);
        void* value = *reinterpret_cast<void**>(ptr.ptr);
        return vm->new_object<VoidP>(args[0].type, value);
    });

    PyObject* type;
    Type type_t;

#define BIND_PRIMITIVE(T, CNAME)                                                                                       \
    vm->bind_func(mod, CNAME "_", 1, [](VM* vm, ArgsView args) {                                                       \
        T val = CAST(T, args[0]);                                                                                      \
        return vm->new_user_object<Struct>(&val, sizeof(T));                                                           \
    });                                                                                                                \
    type = vm->new_type_object(mod, CNAME "_p", vm->_tp_user<VoidP>(), true);                                          \
    mod->attr().set(CNAME "_p", type);                                                                                 \
    type_t = type->as<Type>();                                                                                         \
    vm->bind_func(type, "read", 1, [](VM* vm, ArgsView args) {                                                         \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, args[0]);                                                           \
        T* target = (T*)voidp.ptr;                                                                                     \
        return VAR(*target);                                                                                           \
    });                                                                                                                \
    vm->bind_func(type, "write", 2, [](VM* vm, ArgsView args) {                                                        \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, args[0]);                                                           \
        T val = CAST(T, args[1]);                                                                                      \
        T* target = (T*)voidp.ptr;                                                                                     \
        *target = val;                                                                                                 \
        return vm->None;                                                                                               \
    });                                                                                                                \
    vm->bind__getitem__(type_t, [](VM* vm, PyVar obj, PyVar index) {                                                   \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, obj);                                                               \
        i64 offset = CAST(i64, index);                                                                                 \
        T* target = (T*)voidp.ptr;                                                                                     \
        return VAR(target[offset]);                                                                                    \
    });                                                                                                                \
    vm->bind__setitem__(type_t, [](VM* vm, PyVar obj, PyVar index, PyVar value) {                                      \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, obj);                                                               \
        i64 offset = CAST(i64, index);                                                                                 \
        T* target = (T*)voidp.ptr;                                                                                     \
        target[offset] = CAST(T, value);                                                                               \
    });                                                                                                                \
    vm->bind__add__(type_t, [](VM* vm, PyVar lhs, PyVar rhs) {                                                         \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, lhs);                                                               \
        i64 offset = CAST(i64, rhs);                                                                                   \
        T* target = (T*)voidp.ptr;                                                                                     \
        return vm->new_object<VoidP>(lhs.type, target + offset);                                                       \
    });                                                                                                                \
    vm->bind__sub__(type_t, [](VM* vm, PyVar lhs, PyVar rhs) {                                                         \
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, lhs);                                                               \
        i64 offset = CAST(i64, rhs);                                                                                   \
        T* target = (T*)voidp.ptr;                                                                                     \
        return vm->new_object<VoidP>(lhs.type, target - offset);                                                       \
    });                                                                                                                \
    vm->bind__repr__(type_t, [](VM* vm, PyVar obj) -> Str {                                                            \
        VoidP& self = _CAST(VoidP&, obj);                                                                              \
        return _S("<", CNAME, "* at ", self.hex(), ">");                                                               \
    });

    BIND_PRIMITIVE(char, "char")
    BIND_PRIMITIVE(unsigned char, "uchar")
    BIND_PRIMITIVE(short, "short")
    BIND_PRIMITIVE(unsigned short, "ushort")
    BIND_PRIMITIVE(int, "int")
    BIND_PRIMITIVE(unsigned int, "uint")
    BIND_PRIMITIVE(long, "long")
    BIND_PRIMITIVE(unsigned long, "ulong")
    BIND_PRIMITIVE(long long, "longlong")
    BIND_PRIMITIVE(unsigned long long, "ulonglong")
    BIND_PRIMITIVE(float, "float")
    BIND_PRIMITIVE(double, "double")
    BIND_PRIMITIVE(bool, "bool")

#undef BIND_PRIMITIVE

    PyObject* char_p_t = mod->attr("char_p").get();
    vm->bind(char_p_t, "read_string(self) -> str", [](VM* vm, ArgsView args) {
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, args[0]);
        const char* target = (const char*)voidp.ptr;
        return VAR(target);
    });

    vm->bind(char_p_t, "write_string(self, value: str)", [](VM* vm, ArgsView args) {
        obj_get_t<VoidP> voidp = PK_OBJ_GET(VoidP, args[0]);
        std::string_view sv = CAST(Str&, args[1]).sv();
        char* target = (char*)voidp.ptr;
        std::memcpy(target, sv.data(), sv.size());
        target[sv.size()] = '\0';
        return vm->None;
    });
}

PyVar from_void_p(VM* vm, void* p) { return vm->new_user_object<VoidP>(p); }

}  // namespace pkpy
