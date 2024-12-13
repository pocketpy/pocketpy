#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include <stdint.h>

typedef enum {
    // clang-format off
    PKL_NONE,
    PKL_INT8, PKL_INT16, PKL_INT32, PKL_INT64,
    PKL_FLOAT32, PKL_FLOAT64,
    PKL_TRUE, PKL_FALSE,
    PKL_STRING, PKL_BYTES,
    PKL_BUILD_LIST,
    PKL_BUILD_TUPLE,
    PKL_BUILD_DICT,
    PKL_VEC2, PKL_VEC3,
    PKL_VEC2I, PKL_VEC3I,
    PKL_TYPE,
    PKL_EOF,
    // clang-format on
} PickleOp;

typedef struct {
    c11_vector /*T=char*/ codes;
} PickleObject;

static void PickleObject__ctor(PickleObject* self) { c11_vector__ctor(&self->codes, sizeof(char)); }

static void PickleObject__dtor(PickleObject* self) { c11_vector__dtor(&self->codes); }

static void PickleObject__py_submit(PickleObject* self, py_OutRef out) {
    int size;
    unsigned char* data = c11_vector__submit(&self->codes, &size);
    unsigned char* out_data = py_newbytes(out, size);
    memcpy(out_data, data, size);
}

static void PickleObject__write_bytes(PickleObject* buf, const void* data, int size) {
    c11_vector__extend(char, &buf->codes, data, size);
}

static void pkl__emit_op(PickleObject* buf, PickleOp op) {
    c11_vector__push(char, &buf->codes, op);
}

static void pkl__emit_int(PickleObject* buf, py_i64 val) {
    if(val >= INT8_MIN && val <= INT8_MAX) {
        pkl__emit_op(buf, PKL_INT8);
        PickleObject__write_bytes(buf, &val, 1);
    } else if(val >= INT16_MIN && val <= INT16_MAX) {
        pkl__emit_op(buf, PKL_INT16);
        PickleObject__write_bytes(buf, &val, 2);
    } else if(val >= INT32_MIN && val <= INT32_MAX) {
        pkl__emit_op(buf, PKL_INT32);
        PickleObject__write_bytes(buf, &val, 4);
    } else {
        pkl__emit_op(buf, PKL_INT64);
        PickleObject__write_bytes(buf, &val, 8);
    }
}

#define UNALIGNED_READ(p_val, p_buf)                                                               \
    do {                                                                                           \
        memcpy((p_val), (p_buf), sizeof(*(p_val)));                                                \
        (p_buf) += sizeof(*(p_val));                                                               \
    } while(0)

static py_i64 pkl__read_int(const unsigned char** p) {
    PickleOp op = (PickleOp) * *p;
    (*p)++;
    switch(op) {
        case PKL_INT8: {
            int8_t val;
            UNALIGNED_READ(&val, *p);
            return val;
        }
        case PKL_INT16: {
            int16_t val;
            UNALIGNED_READ(&val, *p);
            return val;
        }
        case PKL_INT32: {
            int32_t val;
            UNALIGNED_READ(&val, *p);
            return val;
        }
        case PKL_INT64: {
            int64_t val;
            UNALIGNED_READ(&val, *p);
            return val;
        }
        default: c11__abort("pkl__read_int(): invalid op: %d", op);
    }
}

const static char* pkl__read_cstr(const unsigned char** p) {
    const char* p_str = (const char*)*p;
    int length = strlen(p_str);
    *p += length + 1;  // include '\0'
    return p_str;
}

static bool pickle_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int size;
    const unsigned char* data = py_tobytes(argv, &size);
    return py_pickle_loads(data, size);
}

static bool pickle_dumps(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_pickle_dumps(argv);
}

void pk__add_module_pickle() {
    py_Ref mod = py_newmodule("pickle");

    int x = 1;
    bool is_little_endian = *(char*)&x == 1;
    if(!is_little_endian) c11__abort("is_little_endian != true");

    py_bindfunc(mod, "loads", pickle_loads);
    py_bindfunc(mod, "dumps", pickle_dumps);
}

static bool pickle__write_object(PickleObject* buf, py_TValue* obj);

static bool pickle__write_array(PickleObject* buf, PickleOp op, py_TValue* arr, int length) {
    for(int i = 0; i < length; i++) {
        bool ok = pickle__write_object(buf, arr + i);
        if(!ok) return false;
    }
    pkl__emit_op(buf, op);
    pkl__emit_int(buf, length);
    return true;
}

static bool pickle__write_dict_kv(py_Ref k, py_Ref v, void* ctx) {
    PickleObject* buf = (PickleObject*)ctx;
    if(!pickle__write_object(buf, k)) return false;
    if(!pickle__write_object(buf, v)) return false;
    return true;
}

static bool pickle__write_object(PickleObject* buf, py_TValue* obj) {
    switch(obj->type) {
        case tp_NoneType: {
            pkl__emit_op(buf, PKL_NONE);
            return true;
        }
        case tp_int: {
            py_i64 val = obj->_i64;
            pkl__emit_int(buf, val);
            return true;
        }
        case tp_float: {
            py_f64 val = obj->_f64;
            float val32 = (float)val;
            if(val == val32) {
                pkl__emit_op(buf, PKL_FLOAT32);
                PickleObject__write_bytes(buf, &val32, 4);
            } else {
                pkl__emit_op(buf, PKL_FLOAT64);
                PickleObject__write_bytes(buf, &val, 8);
            }
            return true;
        }
        case tp_bool: {
            bool val = obj->_bool;
            pkl__emit_op(buf, val ? PKL_TRUE : PKL_FALSE);
            return true;
        }
        case tp_str: {
            pkl__emit_op(buf, PKL_STRING);
            c11_sv sv = py_tosv(obj);
            pkl__emit_int(buf, sv.size);
            PickleObject__write_bytes(buf, sv.data, sv.size);
            return true;
        }
        case tp_bytes: {
            pkl__emit_op(buf, PKL_BYTES);
            int size;
            unsigned char* data = py_tobytes(obj, &size);
            pkl__emit_int(buf, size);
            PickleObject__write_bytes(buf, data, size);
            return true;
        }
        case tp_list: {
            return pickle__write_array(buf, PKL_BUILD_LIST, py_list_data(obj), py_list_len(obj));
        }
        case tp_tuple: {
            return pickle__write_array(buf, PKL_BUILD_TUPLE, py_tuple_data(obj), py_tuple_len(obj));
        }
        case tp_dict: {
            bool ok = py_dict_apply(obj, pickle__write_dict_kv, (void*)buf);
            if(!ok) return false;
            pkl__emit_op(buf, PKL_BUILD_DICT);
            pkl__emit_int(buf, py_dict_len(obj));
            return true;
        }
        case tp_vec2: {
            c11_vec2 val = py_tovec2(obj);
            pkl__emit_op(buf, PKL_VEC2);
            PickleObject__write_bytes(buf, &val, sizeof(c11_vec2));
            return true;
        }
        case tp_vec3: {
            c11_vec3 val = py_tovec3(obj);
            pkl__emit_op(buf, PKL_VEC3);
            PickleObject__write_bytes(buf, &val, sizeof(c11_vec3));
            return true;
        }
        case tp_vec2i: {
            c11_vec2i val = py_tovec2i(obj);
            pkl__emit_op(buf, PKL_VEC2I);
            pkl__emit_int(buf, val.x);
            pkl__emit_int(buf, val.y);
            return true;
        }
        case tp_vec3i: {
            c11_vec3i val = py_tovec3i(obj);
            pkl__emit_op(buf, PKL_VEC3I);
            pkl__emit_int(buf, val.x);
            pkl__emit_int(buf, val.y);
            pkl__emit_int(buf, val.z);
            return true;
        }
        case tp_type: {
            pkl__emit_op(buf, PKL_TYPE);
            py_TypeInfo* ti = pk__type_info(py_totype(obj));
            const char* mod_name = py_tostr(py_getdict(&ti->module, __name__));
            c11_sbuf path_buf;
            c11_sbuf__ctor(&path_buf);
            c11_sbuf__write_cstr(&path_buf, mod_name);
            c11_sbuf__write_cstr(&path_buf, "@");
            c11_sbuf__write_cstr(&path_buf, py_name2str(ti->name));
            c11_string* path = c11_sbuf__submit(&path_buf);
            // include '\0'
            PickleObject__write_bytes(buf, path->data, path->size + 1);
            c11_string__delete(path);
            return true;
        }
        default: return TypeError("'%t' object is not picklable", obj->type);
    }
}

bool py_pickle_dumps(py_Ref val) {
    PickleObject buf;
    PickleObject__ctor(&buf);
    bool ok = pickle__write_object(&buf, val);
    if(!ok) {
        PickleObject__dtor(&buf);
        return false;
    }
    pkl__emit_op(&buf, PKL_EOF);
    PickleObject__py_submit(&buf, py_retval());
    return true;
}

bool py_pickle_loads(const unsigned char* data, int size) {
    py_StackRef p0 = py_peek(0);
    const unsigned char* p = data;
    while(true) {
        PickleOp op = (PickleOp)*p;
        p++;
        switch(op) {
            case PKL_NONE: {
                py_pushnone();
                break;
            }
            case PKL_INT8: {
                int8_t val;
                UNALIGNED_READ(&val, p);
                py_newint(py_pushtmp(), val);
                break;
            }
            case PKL_INT16: {
                int16_t val;
                UNALIGNED_READ(&val, p);
                py_newint(py_pushtmp(), val);
                break;
            }
            case PKL_INT32: {
                int32_t val;
                UNALIGNED_READ(&val, p);
                py_newint(py_pushtmp(), val);
                break;
            }
            case PKL_INT64: {
                int64_t val;
                UNALIGNED_READ(&val, p);
                py_newint(py_pushtmp(), val);
                break;
            }
            case PKL_FLOAT32: {
                float val;
                UNALIGNED_READ(&val, p);
                py_newfloat(py_pushtmp(), val);
                break;
            }
            case PKL_FLOAT64: {
                double val;
                UNALIGNED_READ(&val, p);
                py_newfloat(py_pushtmp(), val);
                break;
            }
            case PKL_TRUE: {
                py_newbool(py_pushtmp(), true);
                break;
            }
            case PKL_FALSE: {
                py_newbool(py_pushtmp(), false);
                break;
            }
            case PKL_STRING: {
                int size = pkl__read_int(&p);
                char* dst = py_newstrn(py_pushtmp(), size);
                memcpy(dst, p, size);
                p += size;
                break;
            }
            case PKL_BYTES: {
                int size = pkl__read_int(&p);
                unsigned char* dst = py_newbytes(py_pushtmp(), size);
                memcpy(dst, p, size);
                p += size;
                break;
            }
            case PKL_BUILD_LIST: {
                int length = pkl__read_int(&p);
                py_OutRef val = py_retval();
                py_newlistn(val, length);
                for(int i = length - 1; i >= 0; i--) {
                    py_StackRef item = py_peek(-1);
                    py_list_setitem(val, i, item);
                    py_pop();
                }
                py_push(val);
                break;
            }
            case PKL_BUILD_TUPLE: {
                int length = pkl__read_int(&p);
                py_OutRef val = py_retval();
                py_newtuple(val, length);
                for(int i = length - 1; i >= 0; i--) {
                    py_StackRef item = py_peek(-1);
                    py_tuple_setitem(val, i, item);
                    py_pop();
                }
                py_push(val);
                break;
            }
            case PKL_BUILD_DICT: {
                int length = pkl__read_int(&p);
                py_OutRef val = py_pushtmp();
                py_newdict(val);
                py_StackRef begin = py_peek(-1) - 2 * length;
                py_StackRef end = py_peek(-1);
                for(py_StackRef i = begin; i < end; i += 2) {
                    py_StackRef k = i;
                    py_StackRef v = i + 1;
                    bool ok = py_dict_setitem(val, k, v);
                    if(!ok) return false;
                }
                py_assign(py_retval(), val);
                py_shrink(2 * length + 1);
                py_push(py_retval());
                break;
            }
            case PKL_VEC2: {
                c11_vec2 val;
                UNALIGNED_READ(&val, p);
                py_newvec2(py_pushtmp(), val);
                break;
            }
            case PKL_VEC3: {
                c11_vec3 val;
                UNALIGNED_READ(&val, p);
                py_newvec3(py_pushtmp(), val);
                break;
            }
            case PKL_VEC2I: {
                c11_vec2i val;
                val.x = pkl__read_int(&p);
                val.y = pkl__read_int(&p);
                py_newvec2i(py_pushtmp(), val);
                break;
            }
            case PKL_VEC3I: {
                c11_vec3i val;
                val.x = pkl__read_int(&p);
                val.y = pkl__read_int(&p);
                val.z = pkl__read_int(&p);
                py_newvec3i(py_pushtmp(), val);
                break;
            }
            case PKL_TYPE: {
                const char* path = pkl__read_cstr(&p);
                char* sep_index = strchr(path, '@');
                assert(sep_index != NULL);
                *sep_index = '\0';
                const char* mod_name = path;
                const char* type_name = sep_index + 1;
                py_Type t = py_gettype(mod_name, py_name(type_name));
                *sep_index = '@';
                if(t == 0) {
                    return ImportError("cannot import '%s' from '%s'", type_name, mod_name);
                }
                py_push(py_tpobject(t));
                break;
            }
            case PKL_EOF: {
                if(py_peek(0) - p0 != 1) { return ValueError("invalid pickle data"); }
                py_assign(py_retval(), p0);
                py_pop();
                return true;
            }
            default: c11__unreachable();
        }
    }
    return true;
}

#undef UNALIGNED_READ