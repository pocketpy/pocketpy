#pragma once

#include "common.h"
#include "vm.h"

namespace pkpy {

#define PY_CLASS(T, mod, name)                  \
    [[deprecated]] static Type _type(VM* vm) { return vm->_cxx_typeid_map[typeid(T)]; }    \
    [[deprecated]] static PyVar register_class(VM* vm, PyVar mod, Type base=Type(0)) {   \
        return vm->register_user_class<T>(mod, #name, base);                \
    }                                                                       

struct VoidP{
    void* ptr;
    VoidP(const void* ptr): ptr(const_cast<void*>(ptr)){}

    bool operator==(const VoidP& other) const {
        return ptr == other.ptr;
    }
    bool operator!=(const VoidP& other) const {
        return ptr != other.ptr;
    }
    bool operator<(const VoidP& other) const { return ptr < other.ptr; }
    bool operator<=(const VoidP& other) const { return ptr <= other.ptr; }
    bool operator>(const VoidP& other) const { return ptr > other.ptr; }
    bool operator>=(const VoidP& other) const { return ptr >= other.ptr; }

    Str hex() const{
        SStream ss;
        ss.write_hex(ptr);
        return ss.str();
    }

    static void _register(VM* vm, PyVar mod, PyVar type);
};

#define POINTER_VAR(Tp, NAME)    \
    inline PyVar py_var(VM* vm, Tp val){    \
        const static std::pair<StrName, StrName> P("c", NAME);      \
        PyVar type = vm->_modules[P.first]->attr(P.second);     \
        return vm->heap.gcnew<VoidP>(PK_OBJ_GET(Type, type), val);  \
    }

POINTER_VAR(char*, "char_p")
// const char* is special, need to rethink about it
POINTER_VAR(const unsigned char*, "uchar_p")
POINTER_VAR(const short*, "short_p")
POINTER_VAR(const unsigned short*, "ushort_p")
POINTER_VAR(const int*, "int_p")
POINTER_VAR(const unsigned int*, "uint_p")
POINTER_VAR(const long*, "long_p")
POINTER_VAR(const unsigned long*, "ulong_p")
POINTER_VAR(const long long*, "longlong_p")
POINTER_VAR(const unsigned long long*, "ulonglong_p")
POINTER_VAR(const float*, "float_p")
POINTER_VAR(const double*, "double_p")
POINTER_VAR(const bool*, "bool_p")

#undef POINTER_VAR


struct Struct{
    static constexpr int INLINE_SIZE = 24;

    char _inlined[INLINE_SIZE];
    char* p;
    int size;

    Struct(int new_size, bool zero_init=true){
        this->size = new_size;
        if(size <= INLINE_SIZE){
            p = _inlined;
        }else{
            p = (char*)malloc(size);
        }
        if(zero_init) memset(p, 0, size);
    }

    Struct(void* p, int size): Struct(size, false){
        if(p != nullptr) memcpy(this->p, p, size);
    }

    Struct(const Struct& other): Struct(other.p, other.size){}
    ~Struct(){ if(p!=_inlined) free(p); }

    static void _register(VM* vm, PyVar mod, PyVar type);
};

static_assert(sizeof(Py_<Struct>) <= 64);
static_assert(sizeof(Py_<Tuple>) <= 64);

/***********************************************/
template<typename Tp>
Tp to_void_p(VM* vm, PyVar var){
    static_assert(std::is_pointer_v<Tp>);
    if(var == vm->None) return nullptr;     // None can be casted to any pointer implicitly
    VoidP& p = CAST(VoidP&, var);
    return reinterpret_cast<Tp>(p.ptr);
}
/*****************************************************************/
void add_module_c(VM* vm);

}   // namespace pkpy