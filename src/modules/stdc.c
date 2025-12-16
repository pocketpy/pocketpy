#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"
#include <string.h>

#define DEF_BUILTIN_MEMORY_T(Char_, char_, tp_int_, py_newint_, py_toint_, py_i64_)                \
    static bool stdc_##Char_##__new__(int argc, py_Ref argv) {                                     \
        char_* ud = py_newobject(py_retval(), tp_stdc_##Char_, 0, sizeof(char_));                  \
        if(argc == 2) {                                                                            \
            PY_CHECK_ARG_TYPE(1, tp_int_);                                                         \
            *ud = (char_)py_toint_(&argv[1]);                                                      \
        } else if(argc > 2) {                                                                      \
            return TypeError("expected 1 or 2 arguments, got %d", argc);                           \
        }                                                                                          \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__get_value(int argc, py_Ref argv) {                                 \
        PY_CHECK_ARGC(1);                                                                          \
        char_* ud = py_touserdata(argv);                                                           \
        py_newint_(py_retval(), (py_i64_)(*ud));                                                   \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__set_value(int argc, py_Ref argv) {                                 \
        PY_CHECK_ARGC(2);                                                                          \
        char_* ud = py_touserdata(argv);                                                           \
        PY_CHECK_ARG_TYPE(1, tp_int_);                                                             \
        *ud = (char_)py_toint_(&argv[1]);                                                          \
        py_newnone(py_retval());                                                                   \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__read_STATIC(int argc, py_Ref argv) {                               \
        PY_CHECK_ARGC(2);                                                                          \
        PY_CHECK_ARG_TYPE(0, tp_int);                                                              \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        char_* p = (char_*)(intptr_t)py_toint(&argv[0]);                                           \
        int offset = py_toint(&argv[1]);                                                           \
        py_newint_(py_retval(), (py_i64_)(p[offset]));                                             \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__write_STATIC(int argc, py_Ref argv) {                              \
        PY_CHECK_ARGC(3);                                                                          \
        PY_CHECK_ARG_TYPE(0, tp_int);                                                              \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        PY_CHECK_ARG_TYPE(2, tp_int_);                                                             \
        char_* p = (char_*)(intptr_t)py_toint(&argv[0]);                                           \
        int offset = py_toint(&argv[1]);                                                           \
        p[offset] = (char_)py_toint_(&argv[2]);                                                    \
        py_newnone(py_retval());                                                                   \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__array_STATIC(int argc, py_Ref argv) {                              \
        PY_CHECK_ARGC(1);                                                                          \
        PY_CHECK_ARG_TYPE(0, tp_int);                                                              \
        int length = py_toint(argv);                                                               \
        int size = sizeof(char_) * length;                                                         \
        py_newobject(py_retval(), tp_stdc_##Char_, 0, size);                                       \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__getitem__(int argc, py_Ref argv) {                                 \
        PY_CHECK_ARGC(2);                                                                          \
        char_* ud = py_touserdata(argv);                                                           \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        int index = py_toint(&argv[1]);                                                            \
        py_newint_(py_retval(), (py_i64_)(ud[index]));                                             \
        return true;                                                                               \
    }                                                                                              \
    static bool stdc_##Char_##__setitem__(int argc, py_Ref argv) {                                 \
        PY_CHECK_ARGC(3);                                                                          \
        char_* ud = py_touserdata(argv);                                                           \
        PY_CHECK_ARG_TYPE(1, tp_int);                                                              \
        PY_CHECK_ARG_TYPE(2, tp_int_);                                                             \
        int index = py_toint(&argv[1]);                                                            \
        ud[index] = (char_)py_toint_(&argv[2]);                                                    \
        py_newnone(py_retval());                                                                   \
        return true;                                                                               \
    }                                                                                              \
    static void pk__bind_stdc_##Char_(py_Ref mod) {                                                \
        py_Type type = py_newtype(#Char_, tp_stdc_Memory, mod, NULL);                              \
        py_tpsetfinal(type);                                                                       \
        assert(type == tp_stdc_##Char_);                                                           \
        py_bindmagic(type, __new__, stdc_##Char_##__new__);                                        \
        py_bindmagic(type, __getitem__, stdc_##Char_##__getitem__);                                \
        py_bindmagic(type, __setitem__, stdc_##Char_##__setitem__);                                \
        py_bindproperty(type, "value", stdc_##Char_##__get_value, stdc_##Char_##__set_value);      \
        py_bindstaticmethod(type, "read", stdc_##Char_##__read_STATIC);                            \
        py_bindstaticmethod(type, "write", stdc_##Char_##__write_STATIC);                          \
        py_bindstaticmethod(type, "array", stdc_##Char_##__array_STATIC);                          \
        py_newint(py_emplacedict(py_tpobject(type), py_name("size")), sizeof(char_));              \
    }

DEF_BUILTIN_MEMORY_T(Char, char, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(UChar, unsigned char, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(Short, short, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(UShort, unsigned short, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(Int, int, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(UInt, unsigned int, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(Long, long, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(ULong, unsigned long, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(LongLong, long long, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(ULongLong, unsigned long long, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(Float, float, tp_float, py_newfloat, py_tofloat, float)
DEF_BUILTIN_MEMORY_T(Double, double, tp_float, py_newfloat, py_tofloat, double)
DEF_BUILTIN_MEMORY_T(Pointer, void*, tp_int, py_newint, py_toint, py_i64)
DEF_BUILTIN_MEMORY_T(Bool, bool, tp_bool, py_newbool, py_tobool, bool)

#undef DEF_BUILTIN_MEMORY_T

static bool stdc_malloc(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    py_i64 size = py_toint(&argv[0]);
    void* p = py_malloc(size);
    py_newint(py_retval(), (py_i64)(intptr_t)p);
    return true;
}

static bool stdc_free(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    void* p = (void*)(intptr_t)py_toint(&argv[0]);
    py_free(p);
    py_newnone(py_retval());
    return true;
}

static bool stdc_memcpy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    void* dst = (void*)(intptr_t)py_toint(&argv[0]);
    void* src = (void*)(intptr_t)py_toint(&argv[1]);
    py_i64 n = py_toint(&argv[2]);
    memcpy(dst, src, (size_t)n);
    py_newnone(py_retval());
    return true;
}

static bool stdc_memset(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    void* dst = (void*)(intptr_t)py_toint(&argv[0]);
    int value = (int)py_toint(&argv[1]);
    py_i64 n = py_toint(&argv[2]);
    memset(dst, value, (size_t)n);
    py_newnone(py_retval());
    return true;
}

static bool stdc_memcmp(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    void* p1 = (void*)(intptr_t)py_toint(&argv[0]);
    void* p2 = (void*)(intptr_t)py_toint(&argv[1]);
    py_i64 n = py_toint(&argv[2]);
    int res = memcmp(p1, p2, (size_t)n);
    py_newint(py_retval(), (py_i64)res);
    return true;
}

static bool stdc_addressof(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    if(!py_checkinstance(argv, tp_stdc_Memory)) return false;
    void* ud = py_touserdata(argv);
    py_newint(py_retval(), (py_i64)(intptr_t)ud);
    return true;
}

static bool stdc_sizeof(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_type);
    py_Type type = py_totype(&argv[0]);
    if(!py_issubclass(type, tp_stdc_Memory)) {
        return TypeError("expected a type derived from stdc.Memory");
    }
    py_assign(py_retval(), py_getdict(py_tpobject(type), py_name("size")));
    return true;
}

static bool stdc_read_cstr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    char* p = (char*)(intptr_t)py_toint(&argv[0]);
    py_newstr(py_retval(), p);
    return true;
}

static bool stdc_write_cstr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_str);
    char* p = (char*)(intptr_t)py_toint(&argv[0]);
    c11_sv sv = py_tosv(&argv[1]);
    memcpy(p, sv.data, sv.size);
    p[sv.size] = '\0';
    py_newnone(py_retval());
    return true;
}

static bool stdc_read_bytes(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_int);
    unsigned char* p = (unsigned char*)(intptr_t)py_toint(&argv[0]);
    int size = py_toint(&argv[1]);
    unsigned char* dst = py_newbytes(py_retval(), size);
    memcpy(dst, p, size);
    return true;
}

static bool stdc_write_bytes(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_bytes);
    unsigned char* p = (unsigned char*)(intptr_t)py_toint(&argv[0]);
    int size;
    unsigned char* src = py_tobytes(&argv[1], &size);
    memcpy(p, src, size);
    py_newnone(py_retval());
    return true;
}

void pk__add_module_stdc() {
    py_Ref mod = py_newmodule("stdc");

    py_bindfunc(mod, "malloc", stdc_malloc);
    py_bindfunc(mod, "free", stdc_free);
    py_bindfunc(mod, "memcpy", stdc_memcpy);
    py_bindfunc(mod, "memset", stdc_memset);
    py_bindfunc(mod, "memcmp", stdc_memcmp);

    py_bindfunc(mod, "addressof", stdc_addressof);
    py_bindfunc(mod, "sizeof", stdc_sizeof);

    py_bindfunc(mod, "read_cstr", stdc_read_cstr);
    py_bindfunc(mod, "write_cstr", stdc_write_cstr);
    py_bindfunc(mod, "read_bytes", stdc_read_bytes);
    py_bindfunc(mod, "write_bytes", stdc_write_bytes);

    py_Type Memory = py_newtype("Memory", tp_object, mod, NULL);
    assert(Memory == tp_stdc_Memory);

    pk__bind_stdc_Char(mod);
    pk__bind_stdc_UChar(mod);
    pk__bind_stdc_Short(mod);
    pk__bind_stdc_UShort(mod);
    pk__bind_stdc_Int(mod);
    pk__bind_stdc_UInt(mod);
    pk__bind_stdc_Long(mod);
    pk__bind_stdc_ULong(mod);
    pk__bind_stdc_LongLong(mod);
    pk__bind_stdc_ULongLong(mod);

    for(int size = 1; size <= 8; size *= 2) {
        for(py_Type t = tp_stdc_Char; t <= tp_stdc_ULongLong; t += 2) {
            py_Ref size_var = py_getdict(py_tpobject(t), py_name("size"));
            if(py_toint(size_var) == size) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Int%d", size * 8);
                py_setdict(mod, py_name(buf), py_tpobject(t));
                snprintf(buf, sizeof(buf), "UInt%d", size * 8);
                py_setdict(mod, py_name(buf), py_tpobject(t + 1));
                break;
            }
        }
    }
    for(py_Type t = tp_stdc_Char; t <= tp_stdc_ULongLong; t += 2) {
        py_Ref size_var = py_getdict(py_tpobject(t), py_name("size"));
        if(py_toint(size_var) == sizeof(size_t)) {
            py_setdict(mod, py_name("SizeT"), py_tpobject(t + 1));
            break;
        }
    }

    pk__bind_stdc_Float(mod);
    pk__bind_stdc_Double(mod);
    pk__bind_stdc_Pointer(mod);
    pk__bind_stdc_Bool(mod);
}