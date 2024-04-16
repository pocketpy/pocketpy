#include "pocketpy/cffi.h"

namespace pkpy{

    void VoidP::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_func<2>(type, __new__, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            i64 addr = CAST(i64, args[1]);
            return vm->heap.gcnew<VoidP>(cls, reinterpret_cast<void*>(addr));
        });

        vm->bind__hash__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = PK_OBJ_GET(VoidP, obj);
            return reinterpret_cast<i64>(self.ptr);
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            VoidP& self = PK_OBJ_GET(VoidP, obj);
            return VAR(_S("<void* at ", self.hex(), ">"));
        });

#define BIND_CMP(name, op)  \
        vm->bind##name(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){        \
            if(!vm->isinstance(rhs, VoidP::_type(vm))) return vm->NotImplemented;               \
            void* _0 = PK_OBJ_GET(VoidP, lhs).ptr;                                              \
            void* _1 = PK_OBJ_GET(VoidP, rhs).ptr;                                              \
            return VAR(_0 op _1);                                                               \
        });

        BIND_CMP(__eq__, ==)
        BIND_CMP(__lt__, <)
        BIND_CMP(__le__, <=)
        BIND_CMP(__gt__, >)
        BIND_CMP(__ge__, >=)

#undef BIND_CMP
    }


    void C99Struct::_register(VM* vm, PyObject* mod, PyObject* type){
        vm->bind_constructor<2>(type, [](VM* vm, ArgsView args){
            Type cls = PK_OBJ_GET(Type, args[0]);
            int size = CAST(int, args[1]);
            return vm->heap.gcnew<C99Struct>(cls, size);
        });

        vm->bind_method<0>(type, "hex", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            SStream ss;
            for(int i=0; i<self.size; i++) ss.write_hex((unsigned char)self.p[i]);
            return VAR(ss.str());
        });

        // @staticmethod
        vm->bind_func<1>(type, "fromhex", [](VM* vm, ArgsView args){
            const Str& s = CAST(Str&, args[0]);
            if(s.size<2 || s.size%2!=0) vm->ValueError("invalid hex string");
            C99Struct buffer(s.size/2, false);
            for(int i=0; i<s.size; i+=2){
                char c = 0;
                if(s[i]>='0' && s[i]<='9') c += s[i]-'0';
                else if(s[i]>='A' && s[i]<='F') c += s[i]-'A'+10;
                else if(s[i]>='a' && s[i]<='f') c += s[i]-'a'+10;
                else vm->ValueError(_S("invalid hex char: '", s[i], "'"));
                c <<= 4;
                if(s[i+1]>='0' && s[i+1]<='9') c += s[i+1]-'0';
                else if(s[i+1]>='A' && s[i+1]<='F') c += s[i+1]-'A'+10;
                else if(s[i+1]>='a' && s[i+1]<='f') c += s[i+1]-'a'+10;
                else vm->ValueError(_S("invalid hex char: '", s[i+1], "'"));
                buffer.p[i/2] = c;
            }
            return VAR_T(C99Struct, std::move(buffer));
        }, {}, BindType::STATICMETHOD);

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* obj){
            C99Struct& self = _CAST(C99Struct&, obj);
            SStream ss;
            ss << "<struct object of " << self.size << " bytes>";
            return VAR(ss.str());
        });

        vm->bind_method<0>(type, "addr", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR_T(VoidP, self.p);
        });

        vm->bind_method<0>(type, "sizeof", [](VM* vm, ArgsView args){
            C99Struct& self = _CAST(C99Struct&, args[0]);
            return VAR(self.size);
        });

        vm->bind_method<0>(type, "copy", [](VM* vm, ArgsView args){
            const C99Struct& self = _CAST(C99Struct&, args[0]);
            return vm->heap.gcnew<C99Struct>(vm->_tp(args[0]), self);
        });

        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM* vm, PyObject* lhs, PyObject* rhs){
            C99Struct& self = _CAST(C99Struct&, lhs);
            if(!is_type(rhs, C99Struct::_type(vm))) return vm->NotImplemented;
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
    mod->attr().set("NULL", VAR_T(VoidP, nullptr));

    vm->bind(mod, "p_cast(ptr: 'void_p', cls: type[T]) -> T", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        vm->check_type(args[1], vm->tp_type);
        Type cls = PK_OBJ_GET(Type, args[1]);
        if(!vm->issubclass(cls, VoidP::_type(vm))){
            vm->ValueError("expected a subclass of void_p");
        }
        return vm->heap.gcnew<VoidP>(cls, ptr.ptr);
    });

    vm->bind(mod, "p_value(ptr: 'void_p') -> int", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        return VAR(reinterpret_cast<i64>(ptr.ptr));
    });

    vm->bind(mod, "pp_deref(ptr: Tp) -> Tp", [](VM* vm, ArgsView args){
        VoidP& ptr = CAST(VoidP&, args[0]);
        void* value = *reinterpret_cast<void**>(ptr.ptr);
        return vm->heap.gcnew<VoidP>(args[0]->type, value);
    });

    PyObject* type;
    Type type_t = -1;

#define BIND_PRIMITIVE(T, CNAME) \
    vm->bind_func<1>(mod, CNAME "_", [](VM* vm, ArgsView args){         \
        T val = CAST(T, args[0]);                                       \
        return VAR_T(C99Struct, &val, sizeof(T));                       \
    });                                                                 \
    type = vm->new_type_object(mod, CNAME "_p", VoidP::_type(vm));      \
    mod->attr().set(CNAME "_p", type);                                  \
    type_t = PK_OBJ_GET(Type, type);                                    \
    vm->bind_method<0>(type, "read", [](VM* vm, ArgsView args){         \
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);                      \
        T* target = (T*)voidp.ptr;                                      \
        return VAR(*target);                                            \
    });                                                                 \
    vm->bind_method<1>(type, "write", [](VM* vm, ArgsView args){        \
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);                      \
        T val = CAST(T, args[1]);                                       \
        T* target = (T*)voidp.ptr;                                      \
        *target = val;                                                  \
        return vm->None;                                                \
    });                                                                 \
    vm->bind__getitem__(type_t, [](VM* vm, PyObject* obj, PyObject* index){  \
        VoidP& voidp = PK_OBJ_GET(VoidP, obj);                               \
        i64 offset = CAST(i64, index);                                  \
        T* target = (T*)voidp.ptr;                                      \
        return VAR(target[offset]);                                     \
    });                                                                 \
    vm->bind__setitem__(type_t, [](VM* vm, PyObject* obj, PyObject* index, PyObject* value){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, obj);                          \
        i64 offset = CAST(i64, index);                                  \
        T* target = (T*)voidp.ptr;                                      \
        target[offset] = CAST(T, value);                                \
    });                                                                 \
    vm->bind__add__(type_t, [](VM* vm, PyObject* lhs, PyObject* rhs){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, lhs);                          \
        i64 offset = CAST(i64, rhs);                                    \
        T* target = (T*)voidp.ptr;                                      \
        return vm->heap.gcnew<VoidP>(lhs->type, target + offset);       \
    });                                                                 \
    vm->bind__sub__(type_t, [](VM* vm, PyObject* lhs, PyObject* rhs){   \
        VoidP& voidp = PK_OBJ_GET(VoidP, lhs);                          \
        i64 offset = CAST(i64, rhs);                                    \
        T* target = (T*)voidp.ptr;                                      \
        return vm->heap.gcnew<VoidP>(lhs->type, target - offset);       \
    });                                                                 \
    vm->bind__repr__(type_t, [](VM* vm, PyObject* obj){                 \
        VoidP& self = _CAST(VoidP&, obj);                               \
        return VAR(_S("<", CNAME, "* at ", self.hex(), ">"));         \
    });                                                                 \

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

    PyObject* char_p_t = mod->attr("char_p");
    vm->bind(char_p_t, "read_string(self) -> str", [](VM* vm, ArgsView args){
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);
        const char* target = (const char*)voidp.ptr;
        return VAR(target);
    });

    vm->bind(char_p_t, "write_string(self, value: str)", [](VM* vm, ArgsView args){
        VoidP& voidp = PK_OBJ_GET(VoidP, args[0]);
        std::string_view sv = CAST(Str&, args[1]).sv();
        char* target = (char*)voidp.ptr;
        memcpy(target, sv.data(), sv.size());
        target[sv.size()] = '\0';
        return vm->None;
    });
}

PyObject* from_void_p(VM* vm, void* p){
    return VAR_T(VoidP, p);
}

}   // namespace pkpy