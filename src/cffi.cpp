#include "pocketpy/cffi.h"

namespace pkpy{

    void VoidP::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            i64 addr = CAST(i64, args[1]);
            return vm->heap.gcnew<VoidP>(cls, reinterpret_cast<void*>(addr));
        });

        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.hex());
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            std::stringstream ss;
            ss << "<void* at " << self.hex() << ">";
            return VAR(ss.str());
        });

#define BIND_CMP(name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){       \
            if(!is_non_tagged_type(rhs, VoidP::_type(vm))) return vm->NotImplemented;       \
            return VAR(_CAST(VoidP&, lhs) op _CAST(VoidP&, rhs));                           \
        });

        BIND_CMP(__eq__, ==)
        BIND_CMP(__lt__, <)
        BIND_CMP(__le__, <=)
        BIND_CMP(__gt__, >)
        BIND_CMP(__ge__, >=)

#undef BIND_CMP

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            return reinterpret_cast<i64>(self.ptr);
        });

        vm->bind__add__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr + offset);
        });

        vm->bind__sub__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            VoidP& self = _CAST(VoidP&, lhs);
            i64 offset = CAST(i64, rhs);
            return VAR_T(VoidP, (char*)self.ptr - offset);
        });

#define BIND_SETGET(T, name) \
        vm->bind_method<0>(type, "read_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            return VAR(*(T*)self.ptr);                              \
        });                                                         \
        vm->bind_method<1>(type, "write_" name, [](VM* vm, ArgsView args){   \
            VoidP& self = _CAST(VoidP&, args[0]);                   \
            *(T*)self.ptr = CAST(T, args[1]);                       \
            return vm->None;                                        \
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

        vm->bind_method<1>(type, "read_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 size = CAST(i64, args[1]);
            std::vector<char> buffer(size);
            memcpy(buffer.data(), self.ptr, size);
            return VAR(Bytes(std::move(buffer)));
        });

        vm->bind_method<1>(type, "write_bytes", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            Bytes& bytes = CAST(Bytes&, args[1]);
            memcpy(self.ptr, bytes.data(), bytes.size());
            return vm->None;
        });

#undef BIND_SETGET
    }

    void C99Struct::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            if(is_int(args[1])){
                int size = _CAST(int, args[1]);
                return vm->heap.gcnew<C99Struct>(cls, size);
            }
            if(is_non_tagged_type(args[1], vm->tp_bytes)){
                const Bytes& b = _CAST(Bytes&, args[1]);
                return vm->heap.gcnew<C99Struct>(cls, (void*)b.data(), b.size());
            }
            vm->TypeError("expected int or bytes");
            return vm->None;
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            C99Struct& self = _CAST(C99Struct&, obj);
            std::stringstream ss;
            ss << "<struct object of " << self.size << " bytes>";
            return VAR(ss.str());
        });

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            C99Struct& self = _CAST(C99Struct&, obj);
            std::string_view view((char*)self.p, self.size);
            return (i64)std::hash<std::string_view>()(view);
        });

        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(VoidP, self.p);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(C99Struct, self);
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            C99Struct& self = _CAST(C99Struct&, lhs);
            if(!is_non_tagged_type(rhs, C99Struct::_type(vm))) return vm->NotImplemented;
            C99Struct& other = _CAST(C99Struct&, rhs);
            bool ok = self.size == other.size && memcmp(self.p, other.p, self.size) == 0;
            return VAR(ok);
        });

#define BIND_SETGET(T, name) \
        vm->bind(type, "read_" name "(self, offset=0)", [](VM* vm, ArgsView args){          \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[1]);    \
            void* ptr = self.p + offset;    \
            return VAR(*(T*)ptr);   \
        }); \
        vm->bind(type, "write_" name "(self, value, offset=0)", [](VM* vm, ArgsView args){  \
            C99Struct& self = _CAST(C99Struct&, args[0]);   \
            i64 offset = CAST(i64, args[2]);    \
            void* ptr = self.p + offset;    \
            *(T*)ptr = CAST(T, args[1]);    \
            return vm->None;    \
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

        // patch VoidP
        type = vm->_t(VoidP::_type(vm));

        vm->bind_method<1>(type, "read_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            int size = CAST(int, args[1]);
            return VAR_T(C99Struct, self.ptr, size);
        });

        vm->bind_method<1>(type, "write_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            C99Struct& other = CAST(C99Struct&, args[1]);
            memcpy(self.ptr, other.p, other.size);
            return vm->None;
        });
    }

void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    
#if PK_ENABLE_OS
    vm->bind_func<1>(mod, "malloc", [](VM* vm, ArgsView args){
        i64 size = CAST(i64, args[0]);
        return VAR(malloc(size));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        free(p);
        return vm->None;
    });

    vm->bind_func<3>(mod, "memset", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        memset(p, CAST(int, args[1]), CAST(size_t, args[2]));
        return vm->None;
    });

    vm->bind_func<3>(mod, "memcpy", [](VM* vm, ArgsView args){
        void* dst = CAST(void*, args[0]);
        void* src = CAST(void*, args[1]);
        i64 size = CAST(i64, args[2]);
        memcpy(dst, src, size);
        return vm->None;
    });
#endif

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, ArgsView args){
        const Str& type = CAST(Str&, args[0]);
        auto it = _refl_types.find(type.sv());
        if(it != _refl_types.end()) return VAR(it->second.size);
        vm->ValueError("not a valid c99 type");
        return vm->None;
    });

    VoidP::register_class(vm, mod);
    C99Struct::register_class(vm, mod);
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    add_refl_type("void_p", sizeof(void*));
    PyObject* void_p_t = mod->attr("void_p");

#define BIND_PRIMITIVE(T, name) \
    vm->bind_func<1>(mod, name "_", [](VM* vm, ArgsView args){      \
        T val = CAST(T, args[0]);                                   \
        return VAR_T(C99Struct, &val, sizeof(T));                   \
    });                                                             \
    add_refl_type(name, sizeof(T));                                 \
    mod->attr().set(name "_p", void_p_t);                           \

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

    // add array type
    CodeObject_ code = vm->compile(kPythonLibs["c"], "c.py", EXEC_MODE);
    vm->_exec(code, mod);
}

}   // namespace pkpy