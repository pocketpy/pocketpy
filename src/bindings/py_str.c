#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include <stdbool.h>

c11_string* pk_tostr(py_Ref self) {
    assert(self->type == tp_str);
    if(!self->is_ptr) {
        return (c11_string*)(&self->extra);
    } else {
        return PyObject__userdata(self->_obj);
    }
}

////////////////////////////////
static bool str__new__(int argc, py_Ref argv) {
    assert(argc >= 1);
    if(argc == 1) {
        py_newstr(py_retval(), "");
        return true;
    }
    if(argc > 2) return TypeError("str() takes at most 1 argument");
    return py_str(py_arg(1));
}

static bool str__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    uint64_t res = c11_sv__hash(py_tosv(argv));
    py_newint(py_retval(), (py_i64)res);
    return true;
}

static bool str__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = pk_tostr(&argv[0]);
    py_newint(py_retval(), c11_sv__u8_length((c11_sv){self->data, self->size}));
    return true;
}

static bool str__add__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    if(py_arg(1)->type != tp_str) {
        py_newnotimplemented(py_retval());
    } else {
        c11_string* other = pk_tostr(&argv[1]);
        char* p = py_newstrn(py_retval(), self->size + other->size);
        memcpy(p, self->data, self->size);
        memcpy(p + self->size, other->data, other->size);
    }
    return true;
}

static bool str__mul__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    if(py_arg(1)->type != tp_int) {
        py_newnotimplemented(py_retval());
    } else {
        py_i64 n = py_toint(py_arg(1));
        if(n <= 0) {
            py_newstr(py_retval(), "");
        } else {
            char* p = py_newstrn(py_retval(), self->size * n);
            for(int i = 0; i < n; i++) {
                memcpy(p + i * self->size, self->data, self->size);
            }
        }
    }
    return true;
}

static bool str__rmul__(int argc, py_Ref argv) { return str__mul__(argc, argv); }

static bool str__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    if(py_arg(1)->type != tp_str) {
        py_newnotimplemented(py_retval());
    } else {
        c11_string* other = pk_tostr(&argv[1]);
        const char* p = strstr(self->data, other->data);
        py_newbool(py_retval(), p != NULL);
    }
    return true;
}

static bool str__str__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    *py_retval() = argv[0];
    return true;
}

static bool str__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_quoted(&buf, py_tosv(&argv[0]), '\'');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool str__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int* ud = py_newobject(py_retval(), tp_str_iterator, 1, sizeof(int));
    *ud = 0;
    py_setslot(py_retval(), 0, argv);  // keep a reference to the string
    return true;
}

static bool str__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_sv self = c11_string__sv(pk_tostr(&argv[0]));
    py_Ref _1 = py_arg(1);
    if(_1->type == tp_int) {
        int index = py_toint(py_arg(1));
        if(!pk__normalize_index(&index, self.size)) return false;
        c11_sv res = c11_sv__u8_getitem(self, index);
        py_newstrv(py_retval(), res);
        return true;
    } else if(_1->type == tp_slice) {
        int start, stop, step;
        bool ok = pk__parse_int_slice(_1, c11_sv__u8_length(self), &start, &stop, &step);
        if(!ok) return false;
        c11_string* res = c11_sv__u8_slice(self, start, stop, step);
        py_newstrv(py_retval(), (c11_sv){res->data, res->size});
        c11_string__delete(res);
        return true;
    } else {
        return TypeError("string indices must be integers");
    }
}

#define DEF_STR_CMP_OP(op, __f, __cond)                                                            \
    static bool str##op(int argc, py_Ref argv) {                                                   \
        PY_CHECK_ARGC(2);                                                                          \
        c11_string* self = pk_tostr(&argv[0]);                                                     \
        if(py_arg(1)->type != tp_str) {                                                            \
            py_newnotimplemented(py_retval());                                                     \
        } else {                                                                                   \
            c11_string* other = pk_tostr(&argv[1]);                                                \
            int res = __f(c11_string__sv(self), c11_string__sv(other));                            \
            py_newbool(py_retval(), __cond);                                                       \
        }                                                                                          \
        return true;                                                                               \
    }

DEF_STR_CMP_OP(__eq__, c11__sveq, res)
DEF_STR_CMP_OP(__ne__, c11__sveq, !res)
DEF_STR_CMP_OP(__lt__, c11_sv__cmp, res < 0)
DEF_STR_CMP_OP(__le__, c11_sv__cmp, res <= 0)
DEF_STR_CMP_OP(__gt__, c11_sv__cmp, res > 0)
DEF_STR_CMP_OP(__ge__, c11_sv__cmp, res >= 0)

#undef DEF_STR_CMP_OP

static bool str_lower(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = pk_tostr(&argv[0]);
    char* p = py_newstrn(py_retval(), self->size);
    for(int i = 0; i < self->size; i++) {
        char c = self->data[i];
        p[i] = c >= 'A' && c <= 'Z' ? c + 32 : c;
    }
    return true;
}

static bool str_upper(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_string* self = pk_tostr(&argv[0]);
    char* p = py_newstrn(py_retval(), self->size);
    for(int i = 0; i < self->size; i++) {
        char c = self->data[i];
        p[i] = c >= 'a' && c <= 'z' ? c - 32 : c;
    }
    return true;
}

static bool str_startswith(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* other = pk_tostr(&argv[1]);
    py_newbool(py_retval(), c11_sv__startswith(c11_string__sv(self), c11_string__sv(other)));
    return true;
}

static bool str_endswith(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* other = pk_tostr(&argv[1]);
    py_newbool(py_retval(), c11_sv__endswith(c11_string__sv(self), c11_string__sv(other)));
    return true;
}

static bool str_join(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_sv self = c11_string__sv(pk_tostr(argv));

    if(!py_iter(py_arg(1))) return false;
    py_push(py_retval());  // iter

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    bool first = true;
    while(true) {
        int res = py_next(py_peek(-1));
        if(res == -1) {
            c11_sbuf__dtor(&buf);
            return false;
        }

        if(res == 0) break;

        if(!first) c11_sbuf__write_sv(&buf, self);
        if(!py_checkstr(py_retval())) {
            c11_sbuf__dtor(&buf);
            return false;
        }
        c11_string* item = pk_tostr(py_retval());
        c11_sbuf__write_cstrn(&buf, item->data, item->size);
        first = false;
    }

    py_pop();  // iter
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool str_replace(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    c11_string* self = pk_tostr(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    PY_CHECK_ARG_TYPE(2, tp_str);
    c11_string* old = pk_tostr(&argv[1]);
    c11_string* new_ = pk_tostr(&argv[2]);
    c11_string* res =
        c11_sv__replace2(c11_string__sv(self), c11_string__sv(old), c11_string__sv(new_));
    py_newstrv(py_retval(), (c11_sv){res->data, res->size});
    c11_string__delete(res);
    return true;
}

static bool str_split(int argc, py_Ref argv) {
    c11_sv self = c11_string__sv(pk_tostr(&argv[0]));
    c11_vector res;
    bool discard_empty = false;
    if(argc > 2) return TypeError("split() takes at most 2 arguments");
    if(argc == 1) {
        // sep = None
        res = c11_sv__splitwhitespace(self);
        discard_empty = true;
    }
    if(argc == 2) {
        // sep = argv[1]
        if(!py_checkstr(&argv[1])) return false;
        c11_sv sep = c11_string__sv(pk_tostr(&argv[1]));
        if(sep.size == 0) return ValueError("empty separator");
        res = c11_sv__split2(self, sep);
    }
    py_newlist(py_retval());
    for(int i = 0; i < res.length; i++) {
        c11_sv part = c11__getitem(c11_sv, &res, i);
        if(discard_empty && part.size == 0) continue;
        py_newstrv(py_list_emplace(py_retval()), part);
    }
    c11_vector__dtor(&res);
    return true;
}

static bool str_count(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_string* self = pk_tostr(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* sub = pk_tostr(&argv[1]);
    int res = c11_sv__count(c11_string__sv(self), c11_string__sv(sub));
    py_newint(py_retval(), res);
    return true;
}

static bool str__strip_impl(bool left, bool right, int argc, py_Ref argv) {
    c11_sv self = c11_string__sv(pk_tostr(&argv[0]));
    c11_sv chars;
    if(argc == 1) {
        chars = (c11_sv){" \t\n\r", 4};
    } else if(argc == 2) {
        if(!py_checkstr(&argv[1])) return false;
        chars = c11_string__sv(pk_tostr(&argv[1]));
    } else {
        return TypeError("strip() takes at most 2 arguments");
    }
    c11_sv res = c11_sv__strip(self, chars, left, right);
    py_newstrv(py_retval(), res);
    return true;
}

static bool str_strip(int argc, py_Ref argv) { return str__strip_impl(true, true, argc, argv); }

static bool str_lstrip(int argc, py_Ref argv) { return str__strip_impl(true, false, argc, argv); }

static bool str_rstrip(int argc, py_Ref argv) { return str__strip_impl(false, true, argc, argv); }

static bool str_zfill(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_sv self = c11_string__sv(pk_tostr(&argv[0]));
    PY_CHECK_ARG_TYPE(1, tp_int);
    int width = py_toint(py_arg(1));
    int delta = width - c11_sv__u8_length(self);
    if(delta <= 0) {
        *py_retval() = argv[0];
        return true;
    }
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    for(int i = 0; i < delta; i++) {
        c11_sbuf__write_char(&buf, '0');
    }
    c11_sbuf__write_sv(&buf, self);
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool str__widthjust_impl(bool left, int argc, py_Ref argv) {
    if(argc > 1 + 2) return TypeError("expected at most 2 arguments");
    char pad;
    if(argc == 1 + 1) {
        pad = ' ';
    } else {
        if(!py_checkstr(&argv[2])) return false;
        c11_string* padstr = pk_tostr(&argv[2]);
        if(padstr->size != 1)
            return TypeError("The fill character must be exactly one character long");
        pad = padstr->data[0];
    }
    c11_sv self = c11_string__sv(pk_tostr(&argv[0]));
    PY_CHECK_ARG_TYPE(1, tp_int);
    int width = py_toint(py_arg(1));
    if(width <= self.size) {
        *py_retval() = argv[0];
        return true;
    }
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    if(left) {
        c11_sbuf__write_sv(&buf, self);
        for(int i = 0; i < width - self.size; i++) {
            c11_sbuf__write_char(&buf, pad);
        }
    } else {
        for(int i = 0; i < width - self.size; i++) {
            c11_sbuf__write_char(&buf, pad);
        }
        c11_sbuf__write_sv(&buf, self);
    }
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool str_ljust(int argc, py_Ref argv) { return str__widthjust_impl(true, argc, argv); }

static bool str_rjust(int argc, py_Ref argv) { return str__widthjust_impl(false, argc, argv); }

static bool str_find(int argc, py_Ref argv) {
    if(argc > 3) return TypeError("find() takes at most 3 arguments");
    int start = 0;
    if(argc == 3) {
        PY_CHECK_ARG_TYPE(2, tp_int);
        start = py_toint(py_arg(2));
    }
    c11_string* self = pk_tostr(&argv[0]);
    PY_CHECK_ARG_TYPE(1, tp_str);
    c11_string* sub = pk_tostr(&argv[1]);
    int res = c11_sv__index2(c11_string__sv(self), c11_string__sv(sub), start);
    py_newint(py_retval(), res);
    return true;
}

static bool str_index(int argc, py_Ref argv) {
    bool ok = str_find(argc, argv);
    if(!ok) return false;
    if(py_toint(py_retval()) == -1) return ValueError("substring not found");
    return true;
}

static bool str_encode(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int size;
    const char* data = py_tostrn(argv, &size);
    unsigned char* p = py_newbytes(py_retval(), size);
    memcpy(p, data, size);
    return true;
}

static bool str_format(int argc, py_Ref argv) {
    c11_sv self = py_tosv(argv);
    py_Ref args = argv + 1;
    int64_t auto_field_index = -1;
    bool manual_field_used = false;
    const char* p_begin = self.data;
    const char* p_end = self.data + self.size;
    const char* p = p_begin;
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    while(p < p_end) {
        if(*p == '{') {
            if((p + 1) < p_end && p[1] == '{') {
                // '{{' -> '{'
                c11_sbuf__write_char(&buf, '{');
                p += 2;
            } else {
                if((p + 1) >= p_end) {
                    c11_sbuf__dtor(&buf);
                    return ValueError("single '{' encountered in format string");
                }
                p++;
                // parse field
                c11_sv field = {p, 0};
                while(p < p_end && *p != '}' && *p != ':') {
                    p++;
                }
                if(p < p_end) field.size = p - field.data;
                // parse spec
                c11_sv spec = {p, 0};
                if(*p == ':') {
                    while(p < p_end && *p != '}') {
                        p++;
                    }
                    if(p < p_end) spec.size = p - spec.data;
                }
                if(p < p_end) {
                    c11__rtassert(*p == '}');
                } else {
                    c11_sbuf__dtor(&buf);
                    return ValueError("expected '}' before end of string");
                }
                // parse auto field
                int64_t arg_index;
                if(field.size > 0) {  // {0}
                    if(auto_field_index >= 0) {
                        c11_sbuf__dtor(&buf);
                        return ValueError(
                            "cannot switch from automatic field numbering to manual field specification");
                    }
                    IntParsingResult res = c11__parse_uint(field, &arg_index, 10);
                    if(res != IntParsing_SUCCESS) {
                        c11_sbuf__dtor(&buf);
                        return ValueError("only integer field name is supported");
                    }
                    manual_field_used = true;
                } else {  // {}
                    if(manual_field_used) {
                        c11_sbuf__dtor(&buf);
                        return ValueError(
                            "cannot switch from manual field specification to automatic field numbering");
                    }
                    auto_field_index++;
                    arg_index = auto_field_index;
                }
                // do format
                if(arg_index < 0 || arg_index >= (argc - 1)) {
                    c11_sbuf__dtor(&buf);
                    return IndexError("replacement index %i out of range for positional args tuple",
                                      arg_index);
                }
                bool ok = pk_format_object(pk_current_vm, &args[arg_index], spec);
                if(!ok) {
                    c11_sbuf__dtor(&buf);
                    return false;
                }
                // append to buf
                c11__rtassert(py_isstr(py_retval()));
                c11_sv formatted = py_tosv(py_retval());
                c11_sbuf__write_sv(&buf, formatted);
                p++;  // skip '}'
            }
        } else if(*p == '}') {
            if((p + 1) < p_end && p[1] == '}') {
                // '}}' -> '}'
                c11_sbuf__write_char(&buf, '}');
                p += 2;
            } else {
                c11_sbuf__dtor(&buf);
                return ValueError("single '}' encountered in format string");
            }
        } else {
            c11_sbuf__write_char(&buf, *p);
            p++;
        }
    }
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

py_Type pk_str__register() {
    py_Type type = pk_newtype("str", tp_object, NULL, NULL, false, true);
    // no need to dtor because the memory is controlled by the object

    py_bindmagic(tp_str, __new__, str__new__);
    py_bindmagic(tp_str, __hash__, str__hash__);
    py_bindmagic(tp_str, __len__, str__len__);
    py_bindmagic(tp_str, __add__, str__add__);
    py_bindmagic(tp_str, __mul__, str__mul__);
    py_bindmagic(tp_str, __rmul__, str__rmul__);
    py_bindmagic(tp_str, __contains__, str__contains__);
    py_bindmagic(tp_str, __str__, str__str__);
    py_bindmagic(tp_str, __repr__, str__repr__);
    py_bindmagic(tp_str, __iter__, str__iter__);
    py_bindmagic(tp_str, __getitem__, str__getitem__);

    py_bindmagic(tp_str, __eq__, str__eq__);
    py_bindmagic(tp_str, __ne__, str__ne__);
    py_bindmagic(tp_str, __lt__, str__lt__);
    py_bindmagic(tp_str, __le__, str__le__);
    py_bindmagic(tp_str, __gt__, str__gt__);
    py_bindmagic(tp_str, __ge__, str__ge__);

    py_bindmethod(tp_str, "lower", str_lower);
    py_bindmethod(tp_str, "upper", str_upper);
    py_bindmethod(tp_str, "startswith", str_startswith);
    py_bindmethod(tp_str, "endswith", str_endswith);
    py_bindmethod(tp_str, "join", str_join);
    py_bindmethod(tp_str, "replace", str_replace);
    py_bindmethod(tp_str, "split", str_split);
    py_bindmethod(tp_str, "count", str_count);
    py_bindmethod(tp_str, "strip", str_strip);
    py_bindmethod(tp_str, "lstrip", str_lstrip);
    py_bindmethod(tp_str, "rstrip", str_rstrip);
    py_bindmethod(tp_str, "zfill", str_zfill);
    py_bindmethod(tp_str, "ljust", str_ljust);
    py_bindmethod(tp_str, "rjust", str_rjust);
    py_bindmethod(tp_str, "find", str_find);
    py_bindmethod(tp_str, "index", str_index);
    py_bindmethod(tp_str, "encode", str_encode);
    py_bindmethod(tp_str, "format", str_format);
    return type;
}

bool str_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int* ud = py_touserdata(&argv[0]);
    int size;
    const char* data = py_tostrn(py_getslot(argv, 0), &size);
    if(*ud == size) return StopIteration();
    int start = *ud;
    int len = c11__u8_header(data[*ud], false);
    *ud += len;
    py_newstrv(py_retval(), (c11_sv){data + start, len});
    return true;
}

py_Type pk_str_iterator__register() {
    py_Type type = pk_newtype("str_iterator", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, str_iterator__next__);
    return type;
}

static bool bytes__new__(int argc, py_Ref argv) {
    if(argc == 1) {
        py_newbytes(py_retval(), 0);
        return true;
    }
    if(argc > 2) return TypeError("bytes() takes at most 1 argument");
    py_TValue* p;
    int length = pk_arrayview(&argv[1], &p);
    if(length == -1) return TypeError("bytes() argument must be a list or tuple");
    unsigned char* data = py_newbytes(py_retval(), length);
    for(int i = 0; i < length; i++) {
        if(!py_checktype(&p[i], tp_int)) return false;
        py_i64 v = py_toint(&p[i]);
        if(v < 0 || v > 255) return ValueError("bytes must be in range(0, 256)");
        data[i] = v;
    }
    return true;
}

static bool bytes__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_bytes* self = py_touserdata(&argv[0]);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, 'b');
    c11_sbuf__write_quoted(&buf, (c11_sv){(const char*)self->data, self->size}, '\'');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool bytes__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    int size;
    unsigned char* data = py_tobytes(&argv[0], &size);
    py_Ref _1 = py_arg(1);
    if(_1->type == tp_int) {
        int index = py_toint(_1);
        if(!pk__normalize_index(&index, size)) return false;
        py_newint(py_retval(), data[index]);
        return true;
    } else if(_1->type == tp_slice) {
        int start, stop, step;
        bool ok = pk__parse_int_slice(_1, size, &start, &stop, &step);
        if(!ok) return false;
        c11_vector res;
        c11_vector__ctor(&res, sizeof(unsigned char));
        for(int i = start; step > 0 ? i < stop : i > stop; i += step) {
            c11_vector__push(unsigned char, &res, data[i]);
        }
        unsigned char* p = py_newbytes(py_retval(), res.length);
        memcpy(p, res.data, res.length);
        c11_vector__dtor(&res);
        return true;
    } else {
        return TypeError("bytes indices must be integers");
    }
}

static bool bytes__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_bytes* self = py_touserdata(&argv[0]);
    if(!py_istype(&argv[1], tp_bytes)) {
        py_newnotimplemented(py_retval());
    } else {
        c11_bytes* other = py_touserdata(&argv[1]);
        py_newbool(py_retval(), c11_bytes__eq(self, other));
    }
    return true;
}

static bool bytes__ne__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_bytes* self = py_touserdata(&argv[0]);
    if(!py_istype(&argv[1], tp_bytes)) {
        py_newnotimplemented(py_retval());
    } else {
        c11_bytes* other = py_touserdata(&argv[1]);
        py_newbool(py_retval(), !c11_bytes__eq(self, other));
    }
    return true;
}

static bool bytes__hash__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_bytes* self = py_touserdata(&argv[0]);
    uint64_t res = 0;
    for(int i = 0; i < self->size; i++) {
        res = res * 31 + self->data[i];
    }
    py_newint(py_retval(), res);
    return true;
}

static bool bytes__add__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    c11_bytes* self = py_touserdata(&argv[0]);
    if(py_arg(1)->type != tp_bytes) {
        py_newnotimplemented(py_retval());
    } else {
        c11_bytes* other = py_touserdata(&argv[1]);
        unsigned char* p = py_newbytes(py_retval(), self->size + other->size);
        memcpy(p, self->data, self->size);
        memcpy(p + self->size, other->data, other->size);
    }
    return true;
}

static bool bytes_decode(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int size;
    unsigned char* data = py_tobytes(&argv[0], &size);
    py_newstrv(py_retval(), (c11_sv){(const char*)data, size});
    return true;
}

static bool bytes__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    c11_bytes* self = py_touserdata(&argv[0]);
    py_newint(py_retval(), self->size);
    return true;
}

py_Type pk_bytes__register() {
    py_Type type = pk_newtype("bytes", tp_object, NULL, NULL, false, true);
    // no need to dtor because the memory is controlled by the object

    py_bindmagic(tp_bytes, __new__, bytes__new__);
    py_bindmagic(tp_bytes, __repr__, bytes__repr__);
    py_bindmagic(tp_bytes, __getitem__, bytes__getitem__);
    py_bindmagic(tp_bytes, __eq__, bytes__eq__);
    py_bindmagic(tp_bytes, __ne__, bytes__ne__);
    py_bindmagic(tp_bytes, __add__, bytes__add__);
    py_bindmagic(tp_bytes, __hash__, bytes__hash__);
    py_bindmagic(tp_bytes, __len__, bytes__len__);

    py_bindmethod(tp_bytes, "decode", bytes_decode);
    return type;
}

#undef DEF_STR_CMP_OP