#include "pocketpy/common/vector.h"
#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/interpreter/array2d.h"
#include <stdint.h>

typedef enum {
    // clang-format off
    PKL_MEMO_GET,
    PKL_MEMO_SET,
    PKL_NIL, PKL_NONE, PKL_ELLIPSIS,
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
    PKL_IMPORT_PATH,
    PKL_GETATTR,
    PKL_TVALUE,
    PKL_CALL,
    PKL_OBJECT,
    PKL_EOF,
    // clang-format on
} PickleOp;

typedef struct {
    bool* used_types;
    int used_types_length;
    c11_smallmap_p2i memo;
    c11_vector /*T=char*/ codes;
} PickleObject;

static void PickleObject__ctor(PickleObject* self) {
    self->used_types_length = pk_current_vm->types.length;
    self->used_types = PK_MALLOC(self->used_types_length);
    memset(self->used_types, 0, self->used_types_length);
    c11_smallmap_p2i__ctor(&self->memo);
    c11_vector__ctor(&self->codes, sizeof(char));
}

static void PickleObject__dtor(PickleObject* self) {
    PK_FREE(self->used_types);
    c11_smallmap_p2i__dtor(&self->memo);
    c11_vector__dtor(&self->codes);
}

static bool PickleObject__py_submit(PickleObject* self, py_OutRef out);

static void PickleObject__write_bytes(PickleObject* buf, const void* data, int size) {
    c11_vector__extend(char, &buf->codes, data, size);
}

static void c11_sbuf__write_type_path(c11_sbuf* path_buf, py_Type type) {
    py_TypeInfo* ti = pk_typeinfo(type);
    if(py_isnil(ti->module)) {
        c11_sbuf__write_cstr(path_buf, py_name2str(ti->name));
        return;
    }
    py_ModuleInfo* mi = py_touserdata(ti->module);
    c11_sbuf__write_sv(path_buf, c11_string__sv(mi->path));
    c11_sbuf__write_char(path_buf, '.');
    c11_sbuf__write_sv(path_buf, py_name2sv(ti->name));
}

static void pkl__emit_op(PickleObject* buf, PickleOp op) {
    c11_vector__push(char, &buf->codes, op);
}

static void pkl__emit_int(PickleObject* buf, py_i64 val) {
    if(val >= 0 && val <= 15) {
        pkl__emit_op(buf, PKL_INT_0 + val);
        return;
    }
    if(INT8_MIN <= val && val <= INT8_MAX) {
        pkl__emit_op(buf, PKL_INT8);
        PickleObject__write_bytes(buf, &val, 1);
    } else if(INT16_MIN <= val && val <= INT16_MAX) {
        pkl__emit_op(buf, PKL_INT16);
        PickleObject__write_bytes(buf, &val, 2);
    } else if(INT32_MIN <= val && val <= INT32_MAX) {
        pkl__emit_op(buf, PKL_INT32);
        PickleObject__write_bytes(buf, &val, 4);
    } else {
        pkl__emit_op(buf, PKL_INT64);
        PickleObject__write_bytes(buf, &val, 8);
    }
}

static void pkl__emit_cstr(PickleObject* buf, const char* s) {
    PickleObject__write_bytes(buf, s, strlen(s) + 1);
}

const static char* pkl__read_cstr(const unsigned char** p) {
    const char* s = (const char*)*p;
    (*p) += strlen(s) + 1;
    return s;
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
        case PKL_INT_0: return 0; case PKL_INT_1: return 1; case PKL_INT_2: return 2; case PKL_INT_3: return 3;
        case PKL_INT_4: return 4; case PKL_INT_5: return 5; case PKL_INT_6: return 6; case PKL_INT_7: return 7;
        case PKL_INT_8: return 8; case PKL_INT_9: return 9; case PKL_INT_10: return 10; case PKL_INT_11: return 11;
        case PKL_INT_12: return 12; case PKL_INT_13: return 13; case PKL_INT_14: return 14; case PKL_INT_15: return 15;
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

static bool pkl__write_object(PickleObject* buf, py_TValue* obj);

static bool pkl__write_array(PickleObject* buf, PickleOp op, py_TValue* arr, int length) {
    for(int i = 0; i < length; i++) {
        bool ok = pkl__write_object(buf, arr + i);
        if(!ok) return false;
    }
    pkl__emit_op(buf, op);
    pkl__emit_int(buf, length);
    return true;
}

static bool pkl__write_dict_kv(py_Ref k, py_Ref v, void* ctx) {
    PickleObject* buf = (PickleObject*)ctx;
    if(!pkl__write_object(buf, k)) return false;
    if(!pkl__write_object(buf, v)) return false;
    return true;
}

static bool pkl__try_memo(PickleObject* buf, PyObject* memo_key) {
    int index = c11_smallmap_p2i__get(&buf->memo, memo_key, -1);
    if(index != -1) {
        pkl__emit_op(buf, PKL_MEMO_GET);
        pkl__emit_int(buf, index);
        return true;
    }
    return false;
}

static void pkl__store_memo(PickleObject* buf, PyObject* memo_key) {
    int index = buf->memo.length;
    c11_smallmap_p2i__set(&buf->memo, memo_key, index);
    pkl__emit_op(buf, PKL_MEMO_SET);
    pkl__emit_int(buf, index);
}

static bool _check_function(Function* f) {
    if(!f->module) return ValueError("cannot pickle function (!f->module)");
    if(f->closure) return ValueError("cannot pickle function with closure");
    if(f->decl->nested) return ValueError("cannot pickle nested function");
    c11_string* name = f->decl->code.name;
    if(name->size == 0) return ValueError("cannot pickle function with empty name");
    if(name->data[0] == '<') return ValueError("cannot pickle anonymous function");
    return true;
}

static bool pkl__write_object(PickleObject* buf, py_TValue* obj) {
    switch(obj->type) {
        case tp_nil: {
            return ValueError("'nil' object is not picklable");
        }
        case tp_NoneType: {
            pkl__emit_op(buf, PKL_NONE);
            return true;
        }
        case tp_ellipsis: {
            pkl__emit_op(buf, PKL_ELLIPSIS);
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
            if(obj->is_ptr && pkl__try_memo(buf, obj->_obj)) return true;
            pkl__emit_op(buf, PKL_STRING);
            c11_sv sv = py_tosv(obj);
            pkl__emit_int(buf, sv.size);
            PickleObject__write_bytes(buf, sv.data, sv.size);
            if(obj->is_ptr) pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_bytes: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            pkl__emit_op(buf, PKL_BYTES);
            int size;
            unsigned char* data = py_tobytes(obj, &size);
            pkl__emit_int(buf, size);
            PickleObject__write_bytes(buf, data, size);
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_list: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            bool ok = pkl__write_array(buf, PKL_BUILD_LIST, py_list_data(obj), py_list_len(obj));
            if(!ok) return false;
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_tuple: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            bool ok = pkl__write_array(buf, PKL_BUILD_TUPLE, py_tuple_data(obj), py_tuple_len(obj));
            if(!ok) return false;
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_dict: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            bool ok = py_dict_apply(obj, pkl__write_dict_kv, (void*)buf);
            if(!ok) return false;
            pkl__emit_op(buf, PKL_BUILD_DICT);
            pkl__emit_int(buf, py_dict_len(obj));
            pkl__store_memo(buf, obj->_obj);
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
            py_Type type = py_totype(obj);
            buf->used_types[type] = true;
            pkl__emit_int(buf, type);
            return true;
        }
        case tp_module: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            py_ModuleInfo* mi = py_touserdata(obj);
            pkl__emit_op(buf, PKL_IMPORT_PATH);
            pkl__emit_cstr(buf, mi->path->data);
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_function: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            Function* f = py_touserdata(obj);
            if(!_check_function(f)) return false;
            c11_string* name = f->decl->code.name;
            if(f->clazz) {
                // NOTE: copied from logic of `case tp_type:`
                pkl__emit_op(buf, PKL_TYPE);
                py_TypeInfo* ti = PyObject__userdata(f->clazz);
                py_Type type = ti->index;
                buf->used_types[type] = true;
                pkl__emit_int(buf, type);
            } else {
                if(!pkl__write_object(buf, f->module)) return false;
            }
            pkl__emit_op(buf, PKL_GETATTR);
            pkl__emit_cstr(buf, name->data);
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        case tp_boundmethod: {
            py_Ref self = py_getslot(obj, 0);
            if(!py_istype(self, tp_type)) {
                return ValueError("tp_boundmethod: !py_istype(self, tp_type)");
            }
            py_Ref func = py_getslot(obj, 1);
            if(!py_istype(func, tp_function)) {
                return ValueError("tp_boundmethod: !py_istype(func, tp_function)");
            }

            Function* f = py_touserdata(func);
            if(!_check_function(f)) return false;

            c11_string* name = f->decl->code.name;
            // NOTE: copied from logic of `case tp_type:`
            pkl__emit_op(buf, PKL_TYPE);
            py_Type type = py_totype(self);
            buf->used_types[type] = true;
            pkl__emit_int(buf, type);

            pkl__emit_op(buf, PKL_GETATTR);
            pkl__emit_cstr(buf, name->data);
            return true;
        }
        case tp_array2d: {
            if(pkl__try_memo(buf, obj->_obj)) return true;
            c11_array2d* arr = py_touserdata(obj);
            for(int i = 0; i < arr->header.numel; i++) {
                if(arr->data[i].is_ptr)
                    return TypeError(
                        "'array2d' object is not picklable because it contains heap-allocated objects");
                buf->used_types[arr->data[i].type] = true;
            }
            pkl__emit_op(buf, PKL_ARRAY2D);
            pkl__emit_int(buf, arr->header.n_cols);
            pkl__emit_int(buf, arr->header.n_rows);
            PickleObject__write_bytes(buf, arr->data, arr->header.numel * sizeof(py_TValue));
            pkl__store_memo(buf, obj->_obj);
            return true;
        }
        default: {
            if(!obj->is_ptr) {
                pkl__emit_op(buf, PKL_TVALUE);
                PickleObject__write_bytes(buf, obj, sizeof(py_TValue));
                buf->used_types[obj->type] = true;
                return true;
            }
            // try memo for `is_ptr=true` objects
            if(pkl__try_memo(buf, obj->_obj)) return true;

            py_TypeInfo* ti = pk_typeinfo(obj->type);
            py_Ref f_reduce = py_tpfindmagic(obj->type, __reduce__);
            if(f_reduce != NULL) {
                if(!py_call(f_reduce, 1, obj)) return false;
                // expected: (callable, args)
                py_Ref reduced = py_retval();
                if(!py_istuple(reduced)) { return TypeError("__reduce__ must return a tuple"); }
                if(py_tuple_len(reduced) != 2) {
                    return TypeError("__reduce__ must return a tuple of length 2");
                }
                if(!pkl__write_object(buf, py_tuple_getitem(reduced, 0))) return false;
                pkl__emit_op(buf, PKL_NIL);
                py_Ref args_tuple = py_tuple_getitem(reduced, 1);
                int args_length = py_tuple_len(args_tuple);
                for(int i = 0; i < args_length; i++) {
                    if(!pkl__write_object(buf, py_tuple_getitem(args_tuple, i))) return false;
                }
                pkl__emit_op(buf, PKL_CALL);
                pkl__emit_int(buf, args_length);
                // store memo
                pkl__store_memo(buf, obj->_obj);
                return true;
            }
            if(ti->is_python) {
                NameDict* dict = PyObject__dict(obj->_obj);
                for(int i = dict->capacity - 1; i >= 0; i--) {
                    NameDict_KV* kv = &dict->items[i];
                    if(kv->key == NULL) continue;
                    if(!pkl__write_object(buf, &kv->value)) return false;
                }
                pkl__emit_op(buf, PKL_OBJECT);
                pkl__emit_int(buf, obj->type);
                buf->used_types[obj->type] = true;
                pkl__emit_int(buf, dict->length);
                for(int i = 0; i < dict->capacity; i++) {
                    NameDict_KV* kv = &dict->items[i];
                    if(kv->key == NULL) continue;
                    c11_sv field = py_name2sv(kv->key);
                    // include '\0'
                    PickleObject__write_bytes(buf, field.data, field.size + 1);
                }

                // store memo
                pkl__store_memo(buf, obj->_obj);
                return true;
            }
            return TypeError("'%t' object is not picklable", obj->type);
        }
    }
    c11__unreachable();
}

bool py_pickle_dumps(py_Ref val) {
    PickleObject buf;
    PickleObject__ctor(&buf);
    bool ok = pkl__write_object(&buf, val);
    if(!ok) {
        PickleObject__dtor(&buf);
        return false;
    }
    pkl__emit_op(&buf, PKL_EOF);
    return PickleObject__py_submit(&buf, py_retval());
}

static py_Type pkl__header_find_type(c11_sv path) {
    int sep_index = c11_sv__rindex(path, '.');
    if(sep_index == -1) return py_gettype(NULL, py_namev(path));
    c11_sv mod_name = c11_sv__slice2(path, 0, sep_index);
    c11_sv name = c11_sv__slice(path, sep_index + 1);
    char buf[PK_MAX_MODULE_PATH_LEN + 1];
    memcpy(buf, mod_name.data, mod_name.size);
    buf[mod_name.size] = '\0';
    return py_gettype(buf, py_namev(name));
}

static c11_sv pkl__header_read_sv(const unsigned char** p, char sep) {
    c11_sv text;
    text.data = (const char*)*p;
    const char* p_end = strchr(text.data, sep);
    assert(p_end != NULL);
    text.size = p_end - text.data;
    *p = (const unsigned char*)p_end + 1;
    return text;
}

static py_i64 pkl__header_read_int(const unsigned char** p, char sep) {
    c11_sv text = pkl__header_read_sv(p, sep);
    py_i64 out;
    IntParsingResult res = c11__parse_uint(text, &out, 10);
    assert(res == IntParsing_SUCCESS);
    return out;
}

bool py_pickle_loads_body(const unsigned char* p, int memo_length, c11_smallmap_d2d* type_mapping);

bool py_pickle_loads(const unsigned char* data, int size) {
    const unsigned char* p = data;

    // \xf0\x9f\xa5\x95
    if(size < 4 || p[0] != 240 || p[1] != 159 || p[2] != 165 || p[3] != 149)
        return ValueError("invalid pickle data");
    p += 4;

    c11_smallmap_d2d type_mapping;
    c11_smallmap_d2d__ctor(&type_mapping);

    while(true) {
        if(*p == '\n') {
            p++;
            break;
        }
        py_Type type = pkl__header_read_int(&p, '(');
        c11_sv path = pkl__header_read_sv(&p, ')');
        py_Type new_type = pkl__header_find_type(path);
        if(new_type == 0) {
            c11_smallmap_d2d__dtor(&type_mapping);
            return ImportError("cannot find type '%v'", path);
        }
        if(type != new_type) c11_smallmap_d2d__set(&type_mapping, type, new_type);
    }

    int memo_length = pkl__header_read_int(&p, '\n');
    bool ok = py_pickle_loads_body(p, memo_length, &type_mapping);
    c11_smallmap_d2d__dtor(&type_mapping);
    return ok;
}

static py_Type pkl__fix_type(py_Type type, c11_smallmap_d2d* type_mapping) {
    int new_type = c11_smallmap_d2d__get(type_mapping, type, -1);
    if(new_type != -1) return (py_Type)new_type;
    return type;
}

bool py_pickle_loads_body(const unsigned char* p, int memo_length, c11_smallmap_d2d* type_mapping) {
    py_StackRef p0 = py_peek(0);
    py_Ref p_memo = py_newtuple(py_pushtmp(), memo_length);
    while(true) {
        PickleOp op = (PickleOp)*p;
        p++;
        switch(op) {
            case PKL_MEMO_GET: {
                int index = pkl__read_int(&p);
                py_Ref val = &p_memo[index];
                assert(!py_isnil(val));
                py_push(val);
                break;
            }
            case PKL_MEMO_SET: {
                int index = pkl__read_int(&p);
                p_memo[index] = *py_peek(-1);
                break;
            }
            case PKL_NIL: {
                py_pushnil();
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
                py_Ref val = py_retval();
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
                py_Ref val = py_retval();
                py_Ref p = py_newtuple(val, length);
                for(int i = length - 1; i >= 0; i--) {
                    p[i] = *py_peek(-1);
                    py_pop();
                }
                py_push(val);
                break;
            }
            case PKL_BUILD_DICT: {
                int length = pkl__read_int(&p);
                py_Ref val = py_pushtmp();
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
                py_Type type = (py_Type)pkl__read_int(&p);
                type = pkl__fix_type(type, type_mapping);
                py_push(py_tpobject(type));
                break;
            }
            case PKL_ARRAY2D: {
                int n_cols = pkl__read_int(&p);
                int n_rows = pkl__read_int(&p);
                c11_array2d* arr = c11_newarray2d(py_pushtmp(), n_cols, n_rows);
                int total_size = arr->header.numel * sizeof(py_TValue);
                memcpy(arr->data, p, total_size);
                for(int i = 0; i < arr->header.numel; i++) {
                    arr->data[i].type = pkl__fix_type(arr->data[i].type, type_mapping);
                }
                p += total_size;
                break;
            }
            case PKL_IMPORT_PATH: {
                const char* path = pkl__read_cstr(&p);
                int res = py_import(path);
                if(res == -1) return false;
                if(res == 0) return ImportError("No module named '%s'", path);
                py_push(py_retval());
                break;
            }
            case PKL_GETATTR: {
                const char* name = pkl__read_cstr(&p);
                py_Ref obj = py_peek(-1);
                if(!py_getattr(obj, py_name(name))) return false;
                py_pop();
                py_push(py_retval());
                break;
            }
            case PKL_TVALUE: {
                py_TValue* tmp = py_pushtmp();
                memcpy(tmp, p, sizeof(py_TValue));
                tmp->type = pkl__fix_type(tmp->type, type_mapping);
                p += sizeof(py_TValue);
                break;
            }
            case PKL_CALL: {
                int argc = pkl__read_int(&p);
                if(!py_vectorcall(argc, 0)) return false;
                py_push(py_retval());
                break;
            }
            case PKL_OBJECT: {
                py_Type type = (py_Type)pkl__read_int(&p);
                type = pkl__fix_type(type, type_mapping);
                py_newobject(py_retval(), type, -1, 0);
                NameDict* dict = PyObject__dict(py_retval()->_obj);
                int dict_length = pkl__read_int(&p);
                for(int i = 0; i < dict_length; i++) {
                    py_StackRef value = py_peek(-1);
                    c11_sv field = {(const char*)p, strlen((const char*)p)};
                    NameDict__set(dict, py_namev(field), value);
                    py_pop();
                    p += field.size + 1;
                }
                py_push(py_retval());
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
    c11__unreachable();
}

static bool PickleObject__py_submit(PickleObject* self, py_OutRef out) {
    c11_sbuf cleartext;
    c11_sbuf__ctor(&cleartext);
    c11_sbuf__write_cstr(&cleartext, "\xf0\x9f\xa5\x95");
    // line 1: type mapping
    for(py_Type type = 0; type < self->used_types_length; type++) {
        if(self->used_types[type]) {
            c11_sbuf__write_int(&cleartext, type);
            c11_sbuf__write_char(&cleartext, '(');
            c11_sbuf__write_type_path(&cleartext, type);
            c11_sbuf__write_char(&cleartext, ')');
        }
    }
    c11_sbuf__write_char(&cleartext, '\n');
    // line 2: memo length
    c11_sbuf__write_int(&cleartext, self->memo.length);
    c11_sbuf__write_char(&cleartext, '\n');
    // -------------------------------------------------- //
    c11_string* header = c11_sbuf__submit(&cleartext);
    int total_size = header->size + self->codes.length;
    unsigned char* p = py_newbytes(py_retval(), total_size);
    memcpy(p, header->data, header->size);
    memcpy(p + header->size, self->codes.data, self->codes.length);
    c11_string__delete(header);
    PickleObject__dtor(self);
    return true;
}

#undef UNALIGNED_READ
