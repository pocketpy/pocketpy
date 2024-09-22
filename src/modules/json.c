#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include <math.h>

static bool json_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* source = py_tostr(argv);
    return py_json_loads(source);
}

static bool json_dumps(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_json_dumps(argv);
}

void pk__add_module_json() {
    py_Ref mod = py_newmodule("json");

    py_setdict(mod, py_name("null"), py_None());
    py_setdict(mod, py_name("true"), py_True());
    py_setdict(mod, py_name("false"), py_False());

    py_bindfunc(mod, "loads", json_loads);
    py_bindfunc(mod, "dumps", json_dumps);
}

static bool json__write_object(c11_sbuf* buf, py_TValue* obj);

static bool json__write_array(c11_sbuf* buf, py_TValue* arr, int length) {
    c11_sbuf__write_char(buf, '[');
    for(int i = 0; i < length; i++) {
        if(i != 0) c11_sbuf__write_cstr(buf, ", ");
        bool ok = json__write_object(buf, arr + i);
        if(!ok) return false;
    }
    c11_sbuf__write_char(buf, ']');
    return true;
}

typedef struct {
    c11_sbuf* buf;
    bool first;
} json__write_dict_kv_ctx;

static bool json__write_dict_kv(py_Ref k, py_Ref v, void* ctx_) {
    json__write_dict_kv_ctx* ctx = ctx_;
    if(!ctx->first) c11_sbuf__write_cstr(ctx->buf, ", ");
    ctx->first = false;
    if(!py_isstr(k)) return TypeError("keys must be strings");
    c11_sbuf__write_quoted(ctx->buf, py_tosv(k), '"');
    c11_sbuf__write_char(ctx->buf, ':');
    return json__write_object(ctx->buf, v);
}

static bool json__write_object(c11_sbuf* buf, py_TValue* obj) {
    switch(obj->type) {
        case tp_NoneType: c11_sbuf__write_cstr(buf, "null"); return true;
        case tp_int: c11_sbuf__write_int(buf, obj->_i64); return true;
        case tp_float: {
            if(isnan(obj->_f64)) {
                c11_sbuf__write_cstr(buf, "NaN");
            } else if(isinf(obj->_f64)) {
                c11_sbuf__write_cstr(buf, obj->_f64 < 0 ? "-Infinity" : "Infinity");
            } else {
                c11_sbuf__write_f64(buf, obj->_f64, -1);
            }
            return true;
        }
        case tp_bool: {
            c11_sbuf__write_cstr(buf, py_tobool(obj) ? "true" : "false");
            return true;
        }
        case tp_str: {
            c11_sbuf__write_quoted(buf, py_tosv(obj), '"');
            return true;
        }
        case tp_list: {
            return json__write_array(buf, py_list_data(obj), py_list_len(obj));
        }
        case tp_tuple: {
            return json__write_array(buf, py_tuple_data(obj), py_tuple_len(obj));
        }
        case tp_dict: {
            c11_sbuf__write_char(buf, '{');
            json__write_dict_kv_ctx ctx = {.buf = buf, .first = true};
            bool ok = py_dict_apply(obj, json__write_dict_kv, &ctx);
            if(!ok) return false;
            c11_sbuf__write_char(buf, '}');
            return true;
        }
        default: return TypeError("'%t' object is not JSON serializable", obj->type);
    }
}

bool py_json_dumps(py_Ref val) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    bool ok = json__write_object(&buf, val);
    if(!ok) {
        c11_sbuf__dtor(&buf);
        return false;
    }
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

bool py_json_loads(const char* source) {
    py_GlobalRef mod = py_getmodule("json");
    return py_exec(source, "<json>", EVAL_MODE, mod);
}

bool py_pusheval(const char* expr, py_GlobalRef module) {
    bool ok = py_exec(expr, "<string>", EVAL_MODE, module);
    if(!ok) return false;
    py_push(py_retval());
    return true;
}
