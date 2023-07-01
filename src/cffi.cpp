#include "pocketpy/cffi.h"

namespace pkpy{

    void VoidP::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_default_constructor<VoidP>(type);

        vm->bind_func<1>(type, "from_hex", [](VM* vm, ArgsView args){
            std::string s = CAST(Str&, args[0]).str();
            size_t size;
            intptr_t ptr = std::stoll(s, &size, 16);
            if(size != s.size()) vm->ValueError("invalid literal for void_p(): " + s);
            return VAR_T(VoidP, (void*)ptr);
        });
        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.hex());
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = _CAST(VoidP&, obj);
            std::stringstream ss;
            ss << "<void* at " << self.hex();
            if(self.base_offset != 1) ss << ", base_offset=" << self.base_offset;
            ss << ">";
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

        vm->bind_method<1>(type, "set_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            if(is_non_tagged_type(args[1], vm->tp_str)){
                const Str& type = _CAST(Str&, args[1]);
                self.base_offset = c99_sizeof(vm, type);
            }else{
                self.base_offset = CAST(int, args[1]);
            }
            return vm->None;
        });

        vm->bind_method<0>(type, "get_base_offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            return VAR(self.base_offset);
        });

        vm->bind_method<1>(type, "offset", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            i64 offset = CAST(i64, args[1]);
            return VAR_T(VoidP, (char*)self.ptr + offset * self.base_offset);
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
        vm->bind_constructor<-1>(type, [](VM* vm, ArgsView args){
            if(args.size() == 1+1){
                if(is_int(args[1])){
                    int size = _CAST(int, args[1]);
                    return VAR_T(C99Struct, size);
                }
                if(is_non_tagged_type(args[1], vm->tp_str)){
                    const Str& s = _CAST(Str&, args[1]);
                    return VAR_T(C99Struct, (void*)s.data, s.size);
                }
                if(is_non_tagged_type(args[1], vm->tp_bytes)){
                    const Bytes& b = _CAST(Bytes&, args[1]);
                    return VAR_T(C99Struct, (void*)b.data(), b.size());
                }
                vm->TypeError("expected int, str or bytes");
                return vm->None;
            }
            if(args.size() == 1+2){
                void* p = CAST(void*, args[1]);
                int size = CAST(int, args[2]);
                return VAR_T(C99Struct, p, size);
            }
            vm->TypeError("expected 1 or 2 arguments");
            return vm->None;
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

        vm->bind_method<0>(type, "to_string", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(Str(self.p, self.size));
        });

        vm->bind_method<0>(type, "to_bytes", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            std::vector<char> buffer(self.size);
            memcpy(buffer.data(), self.p, self.size);
            return VAR(Bytes(std::move(buffer)));
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
            const Str& type = CAST(Str&, args[1]);
            int size = c99_sizeof(vm, type);
            return VAR_T(C99Struct, self.ptr, size);
        });

        vm->bind_method<1>(type, "write_struct", [](VM* vm, ArgsView args){
            VoidP& self = _CAST(VoidP&, args[0]);
            C99Struct& other = CAST(C99Struct&, args[1]);
            memcpy(self.ptr, other.p, other.size);
            return vm->None;
        });
    }

    void C99ReflType::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_notimplemented_constructor<C99ReflType>(type);

        vm->bind_method<0>(type, "__call__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR_T(C99Struct, nullptr, self.size);
        });

        vm->bind_method<0>(type, "__repr__", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR("<ctype '" + Str(self.name) + "'>");
        });

        vm->bind_method<0>(type, "name", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.name);
        });

        vm->bind_method<0>(type, "size", [](VM* vm, ArgsView args){
            C99ReflType& self = _CAST(C99ReflType&, args[0]);
            return VAR(self.size);
        });

        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj, PyObject* key){
            C99ReflType& self = _CAST(C99ReflType&, obj);
            const Str& name = CAST(Str&, key);
            auto it = std::lower_bound(self.fields.begin(), self.fields.end(), name.sv());
            if(it == self.fields.end() || it->name != name.sv()){
                vm->KeyError(key);
                return vm->None;
            }
            return VAR(it->offset);
        });
    }

void add_module_c(VM* vm){
    PyObject* mod = vm->new_module("c");
    
    vm->bind_func<1>(mod, "malloc", [](VM* vm, ArgsView args){
        i64 size = CAST(i64, args[0]);
        return VAR(malloc(size));
    });

    vm->bind_func<1>(mod, "free", [](VM* vm, ArgsView args){
        void* p = CAST(void*, args[0]);
        free(p);
        return vm->None;
    });

    vm->bind_func<1>(mod, "sizeof", [](VM* vm, ArgsView args){
        const Str& type = CAST(Str&, args[0]);
        i64 size = c99_sizeof(vm, type);
        return VAR(size);
    });

    vm->bind_func<1>(mod, "refl", [](VM* vm, ArgsView args){
        const Str& key = CAST(Str&, args[0]);
        auto it = _refl_types.find(key.sv());
        if(it == _refl_types.end()) vm->ValueError("reflection type not found");
        const ReflType& rt = it->second;
        return VAR_T(C99ReflType, rt);
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

    VoidP::register_class(vm, mod);
    C99Struct::register_class(vm, mod);
    C99ReflType::register_class(vm, mod);
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    add_refl_type("char", sizeof(char), {});
    add_refl_type("uchar", sizeof(unsigned char), {});
    add_refl_type("short", sizeof(short), {});
    add_refl_type("ushort", sizeof(unsigned short), {});
    add_refl_type("int", sizeof(int), {});
    add_refl_type("uint", sizeof(unsigned int), {});
    add_refl_type("long", sizeof(long), {});
    add_refl_type("ulong", sizeof(unsigned long), {});
    add_refl_type("longlong", sizeof(long long), {});
    add_refl_type("ulonglong", sizeof(unsigned long long), {});
    add_refl_type("float", sizeof(float), {});
    add_refl_type("double", sizeof(double), {});
    add_refl_type("bool", sizeof(bool), {});
    add_refl_type("void_p", sizeof(void*), {});

    PyObject* void_p_t = mod->attr("void_p");
    for(const char* t: {"char", "uchar", "short", "ushort", "int", "uint", "long", "ulong", "longlong", "ulonglong", "float", "double", "bool"}){
        mod->attr().set(Str(t) + "_", VAR_T(C99ReflType, _refl_types[t]));
        mod->attr().set(Str(t) + "_p", void_p_t);
    }
}

int c99_sizeof(VM* vm, const Str& type){
    auto it = _refl_types.find(type.sv());
    if(it != _refl_types.end()) return it->second.size;
    vm->ValueError("not a valid c99 type");
    return 0;
}

}   // namespace pkpy