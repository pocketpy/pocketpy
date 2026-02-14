#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* cur;
    const char* end;
} json_Parser;

static bool json_Parser__match(json_Parser* self, char c) {
    if(self->cur == self->end) return false;
    if(*self->cur == c) {
        self->cur++;
        return true;
    }
    return false;
}

static void json_Parser__skip_whitespace(json_Parser* self) {
    while(self->cur < self->end) {
        char c = *self->cur;
        if(c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            self->cur++;
        } else {
            break;
        }
    }
}

static bool json_Parser__parse_value(json_Parser* self, py_Ref out);

static bool json_Parser__parse_object(json_Parser* self, py_Ref out) {
    if(!json_Parser__match(self, '{')) return false;
    py_newdict(out);
    json_Parser__skip_whitespace(self);
    if(json_Parser__match(self, '}')) return true;

    while(true) {
        json_Parser__skip_whitespace(self);
        py_Ref key = py_pushtmp();
        if(!json_Parser__parse_value(self, key)) return false;
        if(!py_isstr(key)) return ValueError("json: expecting string as key");

        json_Parser__skip_whitespace(self);
        if(!json_Parser__match(self, ':')) return ValueError("json: expecting ':'");

        json_Parser__skip_whitespace(self);
        py_Ref value = py_pushtmp();
        if(!json_Parser__parse_value(self, value)) return false;

        py_dict_setitem(out, key, value);
        py_pop();  // value
        py_pop();  // key

        json_Parser__skip_whitespace(self);
        if(json_Parser__match(self, '}')) return true;
        if(!json_Parser__match(self, ',')) return ValueError("json: expecting ',' or '}'");
    }
}

static bool json_Parser__parse_array(json_Parser* self, py_Ref out) {
    if(!json_Parser__match(self, '[')) return false;
    py_newlist(out);
    json_Parser__skip_whitespace(self);
    if(json_Parser__match(self, ']')) return true;

    while(true) {
        json_Parser__skip_whitespace(self);
        py_Ref value = py_pushtmp();
        if(!json_Parser__parse_value(self, value)) return false;
        py_list_append(out, value);
        py_pop();  // value

        json_Parser__skip_whitespace(self);
        if(json_Parser__match(self, ']')) return true;
        if(!json_Parser__match(self, ',')) return ValueError("json: expecting ',' or ']'");
    }
}

static bool json_Parser__parse_string(json_Parser* self, py_Ref out) {
    if(!json_Parser__match(self, '"')) return false;
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);

    while(self->cur < self->end) {
        char c = *self->cur++;
        if(c == '"') {
            c11_sbuf__py_submit(&buf, out);
            return true;
        }
        if(c == '\\') {
            if(self->cur == self->end) break;
            c = *self->cur++;
            switch(c) {
                case '"': c11_sbuf__write_char(&buf, '"'); break;
                case '\\': c11_sbuf__write_char(&buf, '\\'); break;
                case '/': c11_sbuf__write_char(&buf, '/'); break;
                case 'b': c11_sbuf__write_char(&buf, '\b'); break;
                case 'f': c11_sbuf__write_char(&buf, '\f'); break;
                case 'n': c11_sbuf__write_char(&buf, '\n'); break;
                case 'r': c11_sbuf__write_char(&buf, '\r'); break;
                case 't': c11_sbuf__write_char(&buf, '\t'); break;
                case 'u': {
                    // TODO: support unicode escape \uXXXX
                    // For now we just write \u and continue to avoid crash
                    c11_sbuf__write_char(&buf, '\\');
                    c11_sbuf__write_char(&buf, 'u');
                    break;
                }
                default: c11_sbuf__write_char(&buf, c); break;
            }
        } else {
            c11_sbuf__write_char(&buf, c);
        }
    }
    c11_sbuf__dtor(&buf);
    return ValueError("json: expecting '\"'");
}

static void json_Parser__ctor_true(py_Ref out) { py_newbool(out, true); }

static void json_Parser__ctor_false(py_Ref out) { py_newbool(out, false); }

static bool json_Parser__parse_keyword(json_Parser* self,
                                       const char* keyword,
                                       py_Ref out,
                                       void (*ctor)(py_Ref)) {
    int len = strlen(keyword);
    if(self->end - self->cur >= len && memcmp(self->cur, keyword, len) == 0) {
        self->cur += len;
        ctor(out);
        return true;
    }
    return false;
}

static bool json_Parser__parse_number(json_Parser* self, py_Ref out) {
    const char* start = self->cur;
    if(self->cur < self->end && *self->cur == '-') self->cur++;

    if(self->cur == self->end) return false;

    if(*self->cur == '0') {
        self->cur++;
        // do not allow leading zeros like 0123
    } else if(isdigit(*self->cur)) {
        self->cur++;
        while(self->cur < self->end && isdigit(*self->cur))
            self->cur++;
    } else {
        return false;
    }

    bool is_float = false;

    // fraction
    if(self->cur < self->end && *self->cur == '.') {
        is_float = true;
        self->cur++;
        if(self->cur == self->end || !isdigit(*self->cur)) return false;
        while(self->cur < self->end && isdigit(*self->cur))
            self->cur++;
    }

    // exponent
    if(self->cur < self->end && (*self->cur == 'e' || *self->cur == 'E')) {
        is_float = true;
        self->cur++;
        if(self->cur < self->end && (*self->cur == '+' || *self->cur == '-')) self->cur++;
        if(self->cur == self->end || !isdigit(*self->cur)) return false;
        while(self->cur < self->end && isdigit(*self->cur))
            self->cur++;
    }

    // parse
    char* endptr;
    if(is_float) {
        double val = strtod(start, &endptr);
        if(endptr != self->cur) return false;
        py_newfloat(out, val);
    } else {
        long long val = strtoll(start, &endptr, 10);
        if(endptr != self->cur) return false;
        py_newint(out, val);
    }
    return true;
}

static bool json_Parser__parse_value(json_Parser* self, py_Ref out) {
    json_Parser__skip_whitespace(self);
    if(self->cur == self->end) return ValueError("json: unexpected end of input");

    char c = *self->cur;
    if(c == '{') return json_Parser__parse_object(self, out);
    if(c == '[') return json_Parser__parse_array(self, out);
    if(c == '"') return json_Parser__parse_string(self, out);
    if(c == 't') return json_Parser__parse_keyword(self, "true", out, json_Parser__ctor_true);
    if(c == 'f') return json_Parser__parse_keyword(self, "false", out, json_Parser__ctor_false);
    if(c == 'n') return json_Parser__parse_keyword(self, "null", out, py_newnone);
    if(c == '-' || isdigit(c)) return json_Parser__parse_number(self, out);

    return ValueError("json: unexpected character '%c'", c);
}

bool py_json_loads(const char* source) {
    json_Parser parser;
    parser.cur = source;
    parser.end = source + strlen(source);

    if(!json_Parser__parse_value(&parser, py_retval())) return false;

    json_Parser__skip_whitespace(&parser);
    if(parser.cur != parser.end) return ValueError("json: extra data");
    return true;
}

static bool json_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* source = py_tostr(argv);
    return py_json_loads(source);
}

static bool json_dumps(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_int);
    int indent = py_toint(&argv[1]);
    return py_json_dumps(argv, indent);
}

void pk__add_module_json() {
    py_Ref mod = py_newmodule("json");

    py_setdict(mod, py_name("null"), py_None());
    py_setdict(mod, py_name("true"), py_True());
    py_setdict(mod, py_name("false"), py_False());
    py_TValue tmp;
    py_newfloat(&tmp, NAN);
    py_setdict(mod, py_name("NaN"), &tmp);
    py_newfloat(&tmp, INFINITY);
    py_setdict(mod, py_name("Infinity"), &tmp);

    py_bindfunc(mod, "loads", json_loads);
    py_bind(mod, "dumps(obj, indent=0)", json_dumps);
}

typedef struct {
    c11_sbuf* buf;
    bool first;
    int indent;
    int depth;
} json__write_dict_kv_ctx;

static bool json__write_object(c11_sbuf* buf, py_TValue* obj, int indent, int depth);

static void json__write_indent(c11_sbuf* buf, int n_spaces) {
    for(int i = 0; i < n_spaces; i++) {
        c11_sbuf__write_char(buf, ' ');
    }
}

static bool json__write_array(c11_sbuf* buf, py_TValue* arr, int length, int indent, int depth) {
    c11_sbuf__write_char(buf, '[');
    if(length == 0) {
        c11_sbuf__write_char(buf, ']');
        return true;
    }
    if(indent > 0) c11_sbuf__write_char(buf, '\n');
    int n_spaces = indent * depth;
    const char* sep = indent > 0 ? ",\n" : ", ";
    for(int i = 0; i < length; i++) {
        if(i != 0) c11_sbuf__write_cstr(buf, sep);
        json__write_indent(buf, n_spaces);
        bool ok = json__write_object(buf, arr + i, indent, depth);
        if(!ok) return false;
    }
    if(indent > 0) {
        c11_sbuf__write_char(buf, '\n');
        json__write_indent(buf, n_spaces - indent);
    }
    c11_sbuf__write_char(buf, ']');
    return true;
}

static bool json__write_dict_kv(py_Ref k, py_Ref v, void* ctx_) {
    json__write_dict_kv_ctx* ctx = ctx_;
    int n_spaces = ctx->indent * ctx->depth;
    const char* sep = ctx->indent > 0 ? ",\n" : ", ";
    if(!ctx->first) c11_sbuf__write_cstr(ctx->buf, sep);
    ctx->first = false;
    if(!py_isstr(k)) return TypeError("keys must be strings");
    json__write_indent(ctx->buf, n_spaces);
    c11_sbuf__write_quoted(ctx->buf, py_tosv(k), '"');
    c11_sbuf__write_cstr(ctx->buf, ": ");
    return json__write_object(ctx->buf, v, ctx->indent, ctx->depth);
}

static bool json__write_namedict_kv(py_Name k, py_Ref v, void* ctx_) {
    json__write_dict_kv_ctx* ctx = ctx_;
    int n_spaces = ctx->indent * ctx->depth;
    const char* sep = ctx->indent > 0 ? ",\n" : ", ";
    if(!ctx->first) c11_sbuf__write_cstr(ctx->buf, sep);
    ctx->first = false;
    json__write_indent(ctx->buf, n_spaces);
    c11_sbuf__write_quoted(ctx->buf, py_name2sv(k), '"');
    c11_sbuf__write_cstr(ctx->buf, ": ");
    return json__write_object(ctx->buf, v, ctx->indent, ctx->depth);
}

static bool json__write_object(c11_sbuf* buf, py_TValue* obj, int indent, int depth) {
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
            return json__write_array(buf, py_list_data(obj), py_list_len(obj), indent, depth + 1);
        }
        case tp_tuple: {
            return json__write_array(buf, py_tuple_data(obj), py_tuple_len(obj), indent, depth + 1);
        }
        case tp_dict: {
            c11_sbuf__write_char(buf, '{');
            if(py_dict_len(obj) == 0) {
                c11_sbuf__write_char(buf, '}');
                return true;
            }
            if(indent > 0) c11_sbuf__write_char(buf, '\n');
            json__write_dict_kv_ctx ctx = {.buf = buf,
                                           .first = true,
                                           .indent = indent,
                                           .depth = depth + 1};
            bool ok = py_dict_apply(obj, json__write_dict_kv, &ctx);
            if(!ok) return false;
            if(indent > 0) {
                c11_sbuf__write_char(buf, '\n');
                json__write_indent(buf, indent * depth);
            }
            c11_sbuf__write_char(buf, '}');
            return true;
        }
        case tp_namedict: {
            py_Ref original = py_getslot(obj, 0);
            c11_sbuf__write_char(buf, '{');
            if(PyObject__dict(original->_obj)->length == 0) {
                c11_sbuf__write_char(buf, '}');
                return true;
            }
            if(indent > 0) c11_sbuf__write_char(buf, '\n');
            json__write_dict_kv_ctx ctx = {.buf = buf,
                                           .first = true,
                                           .indent = indent,
                                           .depth = depth + 1};
            bool ok = py_applydict(original, json__write_namedict_kv, &ctx);
            if(!ok) return false;
            if(indent > 0) {
                c11_sbuf__write_char(buf, '\n');
                json__write_indent(buf, indent * depth);
            }
            c11_sbuf__write_char(buf, '}');
            return true;
        }
        default: return TypeError("'%t' object is not JSON serializable", obj->type);
    }
}

bool py_json_dumps(py_Ref val, int indent) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    bool ok = json__write_object(&buf, val, indent, 0);
    if(!ok) {
        c11_sbuf__dtor(&buf);
        return false;
    }
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}
