#include "pocketpy/objects/error.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/exception.h"

void py_BaseException__stpush(py_Frame* frame,
                              py_Ref self,
                              SourceData_ src,
                              int lineno,
                              const char* func_name) {
    BaseException* ud = py_touserdata(self);
    int max_frame_dumps = py_debugger_status() == 1 ? 31 : 7;
    if(ud->stacktrace.length >= max_frame_dumps) return;
    BaseExceptionFrame* frame_dump = c11_vector__emplace(&ud->stacktrace);
    PK_INCREF(src);
    frame_dump->src = src;
    frame_dump->lineno = lineno;
    frame_dump->name = func_name ? c11_string__new(func_name) : NULL;

    if(py_debugger_status() == 1) {
        if(frame != NULL) {
            py_Frame_newlocals(frame, &frame_dump->locals);
            py_Frame_newglobals(frame, &frame_dump->globals);
        } else {
            py_newdict(&frame_dump->locals);
            py_newdict(&frame_dump->globals);
        }
    }
}

static void BaseException__dtor(void* ud) {
    BaseException* self = (BaseException*)ud;
    c11__foreach(BaseExceptionFrame, &self->stacktrace, it) {
        PK_DECREF(it->src);
        if(it->name) c11_string__delete(it->name);
    }
    c11_vector__dtor(&self->stacktrace);
}

static bool _py_BaseException__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    BaseException* ud = py_newobject(py_retval(), cls, 0, sizeof(BaseException));
    py_newnil(&ud->args);
    py_newnil(&ud->inner_exc);
    c11_vector__ctor(&ud->stacktrace, sizeof(BaseExceptionFrame));
    return true;
}

static bool _py_BaseException__init__(int argc, py_Ref argv) {
    BaseException* ud = py_touserdata(argv);
    py_newnone(py_retval());
    if(argc == 1 + 0) return true;
    if(argc == 1 + 1) {
        py_assign(&ud->args, &argv[1]);
        return true;
    }
    return TypeError("__init__() takes at most 1 arguments but %d were given", argc - 1);
}

static bool _py_BaseException__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    BaseException* ud = py_touserdata(argv);
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    pk_sprintf(&ss, "%t(", argv->type);
    py_Ref args = &ud->args;
    if(!py_isnil(args)) {
        if(!py_repr(args)) return false;
        c11_sbuf__write_sv(&ss, py_tosv(py_retval()));
    }
    c11_sbuf__write_char(&ss, ')');
    c11_sbuf__py_submit(&ss, py_retval());
    return true;
}

static bool _py_BaseException__str__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    BaseException* ud = py_touserdata(argv);
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    py_Ref args = &ud->args;
    if(!py_isnil(args)) {
        if(argv->type == tp_KeyError) {
            if(!py_repr(args)) return false;
        } else {
            if(!py_str(args)) return false;
        }
        c11_sbuf__write_sv(&ss, py_tosv(py_retval()));
    }
    c11_sbuf__py_submit(&ss, py_retval());
    return true;
}

static bool BaseException_args(int argc, py_Ref argv) {
    BaseException* ud = py_touserdata(argv);
    PY_CHECK_ARGC(1);
    py_Ref args = &ud->args;
    if(!py_isnil(args)) {
        py_Ref p = py_newtuple(py_retval(), 1);
        p[0] = *args;
    } else {
        py_newtuple(py_retval(), 0);
    }
    return true;
}

static bool StopIteration_value(int argc, py_Ref argv) {
    BaseException* ud = py_touserdata(argv);
    PY_CHECK_ARGC(1);
    py_Ref args = &ud->args;
    if(py_isnil(args)) {
        py_newnone(py_retval());
    } else {
        py_assign(py_retval(), args);
    }
    return true;
}

py_Type pk_BaseException__register() {
    py_Type type = pk_newtype("BaseException", tp_object, NULL, BaseException__dtor, false, false);

    py_bindmagic(type, __new__, _py_BaseException__new__);
    py_bindmagic(type, __init__, _py_BaseException__init__);
    py_bindmagic(type, __repr__, _py_BaseException__repr__);
    py_bindmagic(type, __str__, _py_BaseException__str__);
    py_bindproperty(type, "args", BaseException_args, NULL);
    return type;
}

py_Type pk_Exception__register() {
    py_Type type = pk_newtype("Exception", tp_BaseException, NULL, NULL, false, false);
    return type;
}

py_Type pk_StopIteration__register() {
    py_Type type = pk_newtype("StopIteration", tp_Exception, NULL, NULL, false, false);
    py_bindproperty(type, "value", StopIteration_value, NULL);
    return type;
}


static void c11_sbuf__write_exc(c11_sbuf* self, py_Ref exc) {
    c11_sbuf__write_cstr(self, "Traceback (most recent call last):\n");
    BaseException* ud = py_touserdata(exc);

    for(int i = ud->stacktrace.length - 1; i >= 0; i--) {
        BaseExceptionFrame* frame = c11__at(BaseExceptionFrame, &ud->stacktrace, i);
        SourceData__snapshot(frame->src,
                             self,
                             frame->lineno,
                             NULL,
                             frame->name ? frame->name->data : NULL);
        c11_sbuf__write_char(self, '\n');
    }

    const char* name = py_tpname(exc->type);
    char* message = safe_stringify_exception(exc);

    c11_sbuf__write_cstr(self, name);
    c11_sbuf__write_cstr(self, ": ");
    c11_sbuf__write_cstr(self, message);

    PK_FREE(message);
}

char* safe_stringify_exception(py_Ref exc) {
    VM* vm = pk_current_vm;

    const char* message = "<exception str() failed>";

    py_Ref tmp = py_pushtmp();
    py_Ref old_unhandled_exc = py_pushtmp();
    *tmp = *exc;
    *old_unhandled_exc = vm->unhandled_exc;
    py_newnil(&vm->unhandled_exc);

    py_StackRef p0 = py_peek(0);
    bool ok = py_str(tmp);
    if(ok) {
        if(py_isstr(py_retval())) message = py_tostr(py_retval());
    } else {
        py_clearexc(p0);
    }

    vm->unhandled_exc = *old_unhandled_exc;
    py_shrink(2);
    return c11_strdup(message);
}

//////////////////////////////////////////////////
bool py_checkexc() {
    VM* vm = pk_current_vm;
    return !py_isnil(&vm->unhandled_exc);
}

bool py_matchexc(py_Type type) {
    VM* vm = pk_current_vm;
    if(py_isnil(&vm->unhandled_exc)) return false;
    bool ok = py_issubclass(vm->unhandled_exc.type, type);
    if(ok) vm->last_retval = vm->unhandled_exc;
    return ok;
}

void py_clearexc(py_StackRef p0) {
    VM* vm = pk_current_vm;
    py_newnil(&vm->unhandled_exc);
    if(p0) {
        c11__rtassert(p0 >= vm->stack.begin && p0 <= vm->stack.sp);
        vm->stack.sp = p0;
    }
}

void py_printexc() {
    char* msg = py_formatexc();
    if(!msg) return;
    pk_current_vm->callbacks.print(msg);
    pk_current_vm->callbacks.print("\n");
    PK_FREE(msg);
}

char* py_formatexc() {
    VM* vm = pk_current_vm;
    if(py_isnil(&vm->unhandled_exc)) return NULL;
    char* res = formatexc_internal(&vm->unhandled_exc);
    if(py_debugger_status() == 1) py_debugger_exceptionbreakpoint(&vm->unhandled_exc);
    return res;
}

char* formatexc_internal(py_Ref exc) {
    c11__rtassert(exc != NULL);
    c11__rtassert(py_issubclass(exc->type, tp_BaseException));

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    BaseException* ud = py_touserdata(exc);
    py_Ref inner = &ud->inner_exc;
    if(py_isnil(inner)) {
        c11_sbuf__write_exc(&ss, exc);
    } else {
        c11_sbuf__write_exc(&ss, inner);
        c11_sbuf__write_cstr(
            &ss,
            "\n\nDuring handling of the above exception, another exception occurred:\n\n");
        c11_sbuf__write_exc(&ss, exc);
    }

    c11_string* res = c11_sbuf__submit(&ss);
    char* dup = PK_MALLOC(res->size + 1);
    memcpy(dup, res->data, res->size);
    dup[res->size] = '\0';
    c11_string__delete(res);
    return dup;
}

bool py_exception(py_Type type, const char* fmt, ...) {
#ifndef NDEBUG
    if(py_checkexc()) {
        const char* name = py_tpname(pk_current_vm->unhandled_exc.type);
        c11__abort("py_exception(): `%s` was already set!", name);
    }
#endif

    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);

    py_Ref message = py_pushtmp();
    c11_sbuf__py_submit(&buf, message);

    bool ok = py_tpcall(type, 1, message);
    if(!ok) return false;
    py_pop();

    return py_raise(py_retval());
}

bool py_raise(py_Ref exc) {
    assert(py_isinstance(exc, tp_BaseException));
    VM* vm = pk_current_vm;
    if(vm->top_frame) {
        FrameExcInfo* info = Frame__top_exc_info(vm->top_frame);
        if(info && !py_isnil(&info->exc)) {
            BaseException* ud = py_touserdata(exc);
            ud->inner_exc = info->exc;
        }
    }
    assert(py_isnil(&vm->unhandled_exc));
    vm->unhandled_exc = *exc;
    return false;
}

bool KeyError(py_Ref key) {
    bool ok = py_tpcall(tp_KeyError, 1, key);
    if(!ok) return false;
    return py_raise(py_retval());
}

bool StopIteration() {
    bool ok = py_tpcall(tp_StopIteration, 0, NULL);
    if(!ok) return false;
    return py_raise(py_retval());
}
