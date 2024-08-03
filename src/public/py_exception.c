#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/error.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/sstream.h"

typedef struct BaseExceptionFrame {
    pk_SourceData_ src;
    int lineno;
    c11_string* name;
} BaseExceptionFrame;

typedef struct BaseException {
    int lineno_backup;
    const CodeObject* code_backup;
    c11_vector /*T=BaseExceptionFrame*/ stacktrace;
} BaseException;

void py_BaseException__set_lineno(py_Ref self, int lineno, const CodeObject* code) {
    BaseException* ud = py_touserdata(self);
    ud->lineno_backup = lineno;
    ud->code_backup = code;
}

int py_BaseException__get_lineno(py_Ref self, const CodeObject* code) {
    BaseException* ud = py_touserdata(self);
    if(code != ud->code_backup) return -1;
    return ud->lineno_backup;
}

void py_BaseException__stpush(py_Ref self, pk_SourceData_ src, int lineno, const char* func_name) {
    BaseException* ud = py_touserdata(self);
    if(ud->stacktrace.count >= 7) return;
    BaseExceptionFrame* frame = c11_vector__emplace(&ud->stacktrace);
    PK_INCREF(src);
    frame->src = src;
    frame->lineno = lineno;
    frame->name = func_name ? c11_string__new(func_name) : NULL;
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
    BaseException* ud = py_newobject(py_retval(), cls, 1, sizeof(BaseException));
    c11_vector__ctor(&ud->stacktrace, sizeof(BaseExceptionFrame));
    ud->lineno_backup = -1;
    ud->code_backup = NULL;
    return true;
}

static bool _py_BaseException__init__(int argc, py_Ref argv) {
    if(argc == 1 + 0) { return true; }
    if(argc == 1 + 1) {
        py_setslot(py_arg(0), 0, py_arg(1));
        return true;
    }
    return TypeError("__init__() takes at most 2 arguments but %d were given", argc);
}

static bool _py_BaseException__repr__(int argc, py_Ref argv) {
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    pk_sprintf(&ss, "%t(", argv->type);
    py_Ref arg = py_getslot(argv, 0);
    if(!py_isnil(arg)) {
        if(!py_repr(arg)) return false;
        c11_sbuf__write_sv(&ss, py_tosv(py_retval()));
    }
    c11_sbuf__write_char(&ss, ')');
    c11_sbuf__py_submit(&ss, py_retval());
    return true;
}

static bool _py_BaseException__str__(int argc, py_Ref argv) {
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);
    py_Ref arg = py_getslot(argv, 0);
    if(!py_isnil(arg)) {
        if(!py_str(arg)) return false;
        c11_sbuf__write_sv(&ss, py_tosv(py_retval()));
    }
    c11_sbuf__py_submit(&ss, py_retval());
    return true;
}

py_Type pk_BaseException__register() {
    py_Type type = pk_newtype("BaseException", tp_object, NULL, BaseException__dtor, false, false);

    py_bindmagic(type, __new__, _py_BaseException__new__);
    py_bindmagic(type, __init__, _py_BaseException__init__);
    py_bindmagic(type, __repr__, _py_BaseException__repr__);
    py_bindmagic(type, __str__, _py_BaseException__str__);
    return type;
}

py_Type pk_Exception__register() {
    py_Type type = pk_newtype("Exception", tp_BaseException, NULL, NULL, false, true);
    return type;
}

//////////////////////////////////////////////////
bool py_checkexc() {
    pk_VM* vm = pk_current_vm;
    return !py_isnil(&vm->curr_exception);
}

void py_printexc() {
    char* msg = py_formatexc();
    if(!msg) return;
    pk_current_vm->print(msg);
    pk_current_vm->print("\n");
    free(msg);
}

char* py_formatexc() {
    pk_VM* vm = pk_current_vm;
    if(py_isnil(&vm->curr_exception)) return NULL;
    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    if(true) { c11_sbuf__write_cstr(&ss, "Traceback (most recent call last):\n"); }

    BaseException* ud = py_touserdata(&vm->curr_exception);

    for(int i = ud->stacktrace.count - 1; i >= 0; i--) {
        BaseExceptionFrame* frame = c11__at(BaseExceptionFrame, &ud->stacktrace, i);
        pk_SourceData__snapshot(frame->src,
                                &ss,
                                frame->lineno,
                                NULL,
                                frame->name ? frame->name->data : NULL);
        c11_sbuf__write_char(&ss, '\n');
    }

    const char* name = py_tpname(vm->curr_exception.type);
    bool ok = py_str(&vm->curr_exception);
    if(!ok) c11__abort("py_printexc(): failed to convert exception to string");
    const char* message = py_tostr(py_retval());

    c11_sbuf__write_cstr(&ss, name);
    c11_sbuf__write_cstr(&ss, ": ");
    c11_sbuf__write_cstr(&ss, message);

    c11_string* res = c11_sbuf__submit(&ss);
    char* dup = malloc(res->size + 1);
    memcpy(dup, res->data, res->size);
    dup[res->size] = '\0';
    c11_string__delete(res);
    return dup;
}

bool py_exception(const char* name, const char* fmt, ...) {
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    va_list args;
    va_start(args, fmt);
    pk_vsprintf(&buf, fmt, args);
    va_end(args);

    py_Ref message = py_pushtmp();
    c11_sbuf__py_submit(&buf, message);

    py_Ref exc_type = py_getdict(&pk_current_vm->builtins, py_name(name));
    if(exc_type == NULL) c11__abort("py_exception(): '%s' not found", name);
    bool ok = py_call(exc_type, 1, message);
    if(!ok) c11__abort("py_exception(): failed to create exception object");
    py_pop();

    return py_raise(py_retval());
}

bool py_raise(py_Ref exc) {
    assert(py_isinstance(exc, tp_BaseException));
    pk_VM* vm = pk_current_vm;
    vm->curr_exception = *exc;
    return false;
}

bool KeyError(py_Ref key) {
    py_Ref cls = py_getdict(&pk_current_vm->builtins, py_name("KeyError"));
    bool ok = py_call(cls, 1, key);
    if(!ok) return false;
    return py_raise(py_retval());
}