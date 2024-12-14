#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/array2d.h"
#include <stdint.h>

typedef enum {
    // clang-format off
    PKL_MEMO_GET,
    PKL_MEMO_SET,
    PKL_NONE, PKL_ELLIPSIS,
    PKL_INT_0, PKL_INT_1, PKL_INT_2, PKL_INT_3, PKL_INT_4, PKL_INT_5, PKL_INT_6, PKL_INT_7,
    PKL_INT_8, PKL_INT_9, PKL_INT_10, PKL_INT_11, PKL_INT_12, PKL_INT_13, PKL_INT_14, PKL_INT_15,
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
    PKL_ARRAY2D,
    PKL_EOF,
    // clang-format on
} PickleOp;

typedef struct {
    c11_smallmap_p2i memo;
    c11_vector /*T=char*/ codes;
} PickleObject;

typedef struct {
    uint16_t memo_length;
} PickleObjectHeader;

static void PickleObject__ctor(PickleObject* self) {
    c11_smallmap_p2i__ctor(&self->memo);
    c11_vector__ctor(&self->codes, sizeof(char));
}

static void PickleObject__dtor(PickleObject* self) {
    c11_smallmap_p2i__dtor(&self->memo);
    c11_vector__dtor(&self->codes);
}

static bool PickleObject__py_submit(PickleObject* self, py_OutRef out) {
    unsigned char* data = self->codes.data;
    PickleObjectHeader* p =
        (PickleObjectHeader*)py_newbytes(out, sizeof(PickleObjectHeader) + self->codes.length);
    if(self->memo.length >= UINT16_MAX) c11__abort("PickleObject__py_submit(): memo overflow");
    p->memo_length = (uint16_t)self->memo.length;
    memcpy(p + 1, data, self->codes.length);
    PickleObject__dtor(self);
    return true;
}

static void PickleObject__write_bytes(PickleObject* buf, const void* data, int size) {
    c11_vector__extend(char, &buf->codes, data, size);
}

static void pkl__emit_op(PickleObject* buf, PickleOp op) {
    c11_vector__push(char, &buf->codes, op);
}

static void pkl__emit_int(PickleObject* buf, py_i64 val) {
    if(val >= 0 && val <= 15) {
        pkl__emit_op(buf, PKL_INT_0 + val);
        return;
    }
    if((int8_t)val == val) {
        pkl__emit_op(buf, PKL_INT8);
        PickleObject__write_bytes(buf, &val, 1);
    } else if((int16_t)val == val) {
        pkl__emit_op(buf, PKL_INT16);
        PickleObject__write_bytes(buf, &val, 2);
    } else if((int32_t)val == val) {
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
        // clang-format off
        case PKL_INT_0: return 0;
        case PKL_INT_1: return 1;
        case PKL_INT_2: return 2;
        case PKL_INT_3: return 3;
        case PKL_INT_4: return 4;
        case PKL_INT_5: return 5;
        case PKL_INT_6: return 6;
        case PKL_INT_7: return 7;
        case PKL_INT_8: return 8;
        case PKL_INT_9: return 9;
        case PKL_INT_10: return 10;
        case PKL_INT_11: return 11;
        case PKL_INT_12: return 12;
        case PKL_INT_13: return 13;
        case PKL_INT_14: return 14;
        case PKL_INT_15: return 15;
        // clang-format on
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
    if(obj->is_ptr) {
        void* memo_key = obj->_obj;
        int index = c11_smallmap_p2i__get(&buf->memo, memo_key, -1);
        if(index != -1) {
            pkl__emit_op(buf, PKL_MEMO_GET);
            pkl__emit_int(buf, index);
            return true;
        }
    }
    switch(obj->type) {
        case tp_NoneType: {
            pkl__emit_op(buf, PKL_NONE);
            break;
        }
        case tp_ellipsis: {
            pkl__emit_op(buf, PKL_ELLIPSIS);
            break;
        }
        case tp_int: {
            py_i64 val = obj->_i64;
            pkl__emit_int(buf, val);
            break;
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
            break;
        }
        case tp_bool: {
            bool val = obj->_bool;
            pkl__emit_op(buf, val ? PKL_TRUE : PKL_FALSE);
            break;
        }
        case tp_str: {
            pkl__emit_op(buf, PKL_STRING);
            c11_sv sv = py_tosv(obj);
            pkl__emit_int(buf, sv.size);
            PickleObject__write_bytes(buf, sv.data, sv.size);
            break;
        }
        case tp_bytes: {
            pkl__emit_op(buf, PKL_BYTES);
            int size;
            unsigned char* data = py_tobytes(obj, &size);
            pkl__emit_int(buf, size);
            PickleObject__write_bytes(buf, data, size);
            break;
        }
        case tp_list: {
            bool ok = pickle__write_array(buf, PKL_BUILD_LIST, py_list_data(obj), py_list_len(obj));
            if(!ok) return false;
            break;
        }
        case tp_tuple: {
            bool ok =
                pickle__write_array(buf, PKL_BUILD_TUPLE, py_tuple_data(obj), py_tuple_len(obj));
            if(!ok) return false;
            break;
        }
        case tp_dict: {
            bool ok = py_dict_apply(obj, pickle__write_dict_kv, (void*)buf);
            if(!ok) return false;
            pkl__emit_op(buf, PKL_BUILD_DICT);
            pkl__emit_int(buf, py_dict_len(obj));
            break;
        }
        case tp_vec2: {
            c11_vec2 val = py_tovec2(obj);
            pkl__emit_op(buf, PKL_VEC2);
            PickleObject__write_bytes(buf, &val, sizeof(c11_vec2));
            break;
        }
        case tp_vec3: {
            c11_vec3 val = py_tovec3(obj);
            pkl__emit_op(buf, PKL_VEC3);
            PickleObject__write_bytes(buf, &val, sizeof(c11_vec3));
            break;
        }
        case tp_vec2i: {
            c11_vec2i val = py_tovec2i(obj);
            pkl__emit_op(buf, PKL_VEC2I);
            pkl__emit_int(buf, val.x);
            pkl__emit_int(buf, val.y);
            break;
        }
        case tp_vec3i: {
            c11_vec3i val = py_tovec3i(obj);
            pkl__emit_op(buf, PKL_VEC3I);
            pkl__emit_int(buf, val.x);
            pkl__emit_int(buf, val.y);
            pkl__emit_int(buf, val.z);
            break;
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
            break;
        }
        case tp_array2d: {
            c11_array2d* arr = py_touserdata(obj);
            for(int i = 0; i < arr->numel; i++) {
                if(arr->data[i].is_ptr)
                    return TypeError(
                        "'array2d' object is not picklable because it contains heap-allocated objects");
            }
            pkl__emit_op(buf, PKL_ARRAY2D);
            pkl__emit_int(buf, arr->n_cols);
            pkl__emit_int(buf, arr->n_rows);
            // TODO: fix type index which is not stable
            PickleObject__write_bytes(buf, arr->data, arr->numel * sizeof(py_TValue));
            break;
        }
        default: return TypeError("'%t' object is not picklable", obj->type);
    }
    if(obj->is_ptr) {
        void* memo_key = obj->_obj;
        int index = buf->memo.length;
        c11_smallmap_p2i__set(&buf->memo, memo_key, index);
        pkl__emit_op(buf, PKL_MEMO_SET);
        pkl__emit_int(buf, index);
    }
    return true;
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
    return PickleObject__py_submit(&buf, py_retval());
}

bool py_pickle_loads(const unsigned char* data, int size) {
    PickleObjectHeader* header = (PickleObjectHeader*)data;
    const unsigned char* p = (const unsigned char*)(header + 1);
    py_StackRef p0 = py_peek(0);
    py_StackRef memo = py_pushtmp();
    py_newtuple(memo, header->memo_length);
    while(true) {
        PickleOp op = (PickleOp)*p;
        p++;
        switch(op) {
            case PKL_MEMO_GET: {
                int index = pkl__read_int(&p);
                py_Ref val = py_tuple_getitem(memo, index);
                assert(!py_isnil(val));
                py_push(val);
                break;
            }
            case PKL_MEMO_SET: {
                int index = pkl__read_int(&p);
                py_tuple_setitem(memo, index, py_peek(-1));
                break;
            }
            case PKL_NONE: {
                py_pushnone();
                break;
            }
            case PKL_ELLIPSIS: {
                py_newellipsis(py_pushtmp());
                break;
            }
            // clang-format off
            case PKL_INT_0: case PKL_INT_1: case PKL_INT_2: case PKL_INT_3:
            case PKL_INT_4: case PKL_INT_5: case PKL_INT_6: case PKL_INT_7:
            case PKL_INT_8: case PKL_INT_9: case PKL_INT_10: case PKL_INT_11:
            case PKL_INT_12: case PKL_INT_13: case PKL_INT_14: case PKL_INT_15: {
                py_newint(py_pushtmp(), op - PKL_INT_0);
                break;
            }
            // clang-format on
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
            case PKL_ARRAY2D: {
                int n_cols = pkl__read_int(&p);
                int n_rows = pkl__read_int(&p);
                c11_array2d* arr = py_newarray2d(py_pushtmp(), n_cols, n_rows);
                int total_size = arr->numel * sizeof(py_TValue);
                memcpy(arr->data, p, total_size);
                p += total_size;
                break;
            }
            case PKL_EOF: {
                // [memo, obj]
                if(py_peek(0) - p0 != 2) return ValueError("invalid pickle data");
                py_assign(py_retval(), py_peek(-1));
                py_shrink(2);
                return true;
            }
            default: c11__unreachable();
        }
    }
    return true;
}

#undef UNALIGNED_READ