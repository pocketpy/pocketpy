#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/_generated.h"
#include <math.h>

py_Ref py_getmodule(const char* path) {
    VM* vm = pk_current_vm;
    return NameDict__try_get(&vm->modules, py_name(path));
}

py_Ref py_getbuiltin(const char* name){
    return py_getdict(&pk_current_vm->builtins, py_name(name));
}

py_Ref py_getglobal(const char* name){
    return py_getdict(&pk_current_vm->main, py_name(name));
}

void py_setglobal(const char* name, py_Ref val){
    py_setdict(&pk_current_vm->main, py_name(name), val);
}

py_Ref py_newmodule(const char* path) {
    ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = ManagedHeap__new(heap, tp_module, -1, 0);

    py_Ref r0 = py_pushtmp();
    py_Ref r1 = py_pushtmp();

    *r0 = (py_TValue){
        .type = obj->type,
        .is_ptr = true,
        ._obj = obj,
    };

    int last_dot = c11_sv__rindex((c11_sv){path, strlen(path)}, '.');
    if(last_dot == -1) {
        py_newstr(r1, path);
        py_setdict(r0, __name__, r1);
        py_newstr(r1, "");
        py_setdict(r0, __package__, r1);
    } else {
        const char* start = path + last_dot + 1;
        py_newstr(r1, start);
        py_setdict(r0, __name__, r1);
        py_newstrn(r1, path, last_dot);
        py_setdict(r0, __package__, r1);
    }

    py_newstr(r1, path);
    py_setdict(r0, __path__, r1);

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    py_Name path_name = py_name(path);
    bool exists = NameDict__contains(&pk_current_vm->modules, path_name);
    if(exists) c11__abort("module '%s' already exists", path);
    NameDict__set(&pk_current_vm->modules, path_name, *r0);

    py_shrink(2);
    return py_getmodule(path);
}

int py_import(const char* path_cstr) {
    // printf("importing %s\n", path_cstr);

    VM* vm = pk_current_vm;
    c11_sv path = {path_cstr, strlen(path_cstr)};
    if(path.size == 0) return ValueError("empty module name");

    if(path.data[0] == '.') {
        // try relative import
        int dot_count = 1;
        while(dot_count < path.size && path.data[dot_count] == '.')
            dot_count++;

        c11_sv top_filename = c11_string__sv(vm->top_frame->co->src->filename);
        int is_init = c11_sv__endswith(top_filename, (c11_sv){"__init__.py", 11});

        py_Ref package = py_getdict(vm->top_frame->module, __path__);
        c11_sv package_sv = py_tosv(package);
        if(package_sv.size == 0) {
            return ImportError("attempted relative import with no known parent package");
        }

        c11_vector /* T=c11_sv */ cpnts = c11_sv__split(package_sv, '.');
        for(int i = is_init; i < dot_count; i++) {
            if(cpnts.count == 0)
                return ImportError("attempted relative import beyond top-level package");
            c11_vector__pop(&cpnts);
        }

        if(dot_count < path.size) {
            c11_sv last_cpnt = c11_sv__slice(path, dot_count);
            c11_vector__push(c11_sv, &cpnts, last_cpnt);
        }

        // join cpnts
        c11_sbuf buf;
        c11_sbuf__ctor(&buf);
        for(int i = 0; i < cpnts.count; i++) {
            if(i > 0) c11_sbuf__write_char(&buf, '.');
            c11_sbuf__write_sv(&buf, c11__getitem(c11_sv, &cpnts, i));
        }

        c11_vector__dtor(&cpnts);
        c11_string* new_path = c11_sbuf__submit(&buf);
        int res = py_import(new_path->data);
        c11_string__delete(new_path);
        return res;
    }

    assert(path.data[0] != '.' && path.data[path.size - 1] != '.');

    // check existing module
    py_TmpRef ext_mod = py_getmodule(path.data);
    if(ext_mod) {
        py_assign(py_retval(), ext_mod);
        return true;
    }

    // try import
    c11_string* slashed_path = c11_sv__replace(path, '.', PK_PLATFORM_SEP);
    c11_string* filename = c11_string__new3("%s.py", slashed_path->data);

    bool need_free = true;
    const char* data = load_kPythonLib(path_cstr);
    if(data != NULL) {
        need_free = false;
        goto __SUCCESS;
    }

    data = vm->import_file(filename->data);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    filename = c11_string__new3("%s/__init__.py", slashed_path->data);
    data = vm->import_file(filename->data);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    return 0;

__SUCCESS:
    py_push(py_newmodule(path_cstr));
    py_Ref mod = py_peek(-1);
    bool ok = py_exec((const char*)data, filename->data, EXEC_MODE, mod);
    py_assign(py_retval(), mod);
    py_pop();

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    if(need_free) free((void*)data);
    return ok ? 1 : -1;
}

//////////////////////////

static bool builtins_repr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_repr(argv);
}

static bool builtins_exit(int argc, py_Ref argv) {
    int code = 0;
    if(argc > 1) return TypeError("exit() takes at most 1 argument");
    if(argc == 1) {
        PY_CHECK_ARG_TYPE(0, tp_int);
        code = py_toint(argv);
    }
    // return py_exception("SystemExit", "%d", code);
    exit(code);
    return false;
}

static bool builtins_len(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_len(argv);
}

static bool builtins_hex(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);

    py_i64 val = py_toint(argv);

    if(val == 0) {
        py_newstr(py_retval(), "0x0");
        return true;
    }

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    if(val < 0) {
        c11_sbuf__write_char(&ss, '-');
        val = -val;
    }
    c11_sbuf__write_cstr(&ss, "0x");
    bool non_zero = true;
    for(int i = 56; i >= 0; i -= 8) {
        unsigned char cpnt = (val >> i) & 0xff;
        c11_sbuf__write_hex(&ss, cpnt, non_zero);
        if(cpnt != 0) non_zero = false;
    }

    c11_sbuf__py_submit(&ss, py_retval());
    return true;
}

static bool builtins_iter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_iter(argv);
}

static bool builtins_next(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int res = py_next(argv);
    if(res == -1) return false;
    if(res) return true;
    return py_exception(tp_StopIteration, "");
}

static bool builtins_hash(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val;
    if(!py_hash(argv, &val)) return false;
    py_newint(py_retval(), val);
    return true;
}

static bool builtins_abs(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return pk_callmagic(__abs__, 1, argv);
}

static bool builtins_divmod(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    return pk_callmagic(__divmod__, 2, argv);
}

static bool builtins_round(int argc, py_Ref argv) {
    py_i64 ndigits;

    if(argc == 1) {
        ndigits = -1;
    } else if(argc == 2) {
        PY_CHECK_ARG_TYPE(1, tp_int);
        ndigits = py_toint(py_arg(1));
        if(ndigits < 0) return ValueError("ndigits should be non-negative");
    } else {
        return TypeError("round() takes 1 or 2 arguments");
    }

    if(py_isint(py_arg(0))) {
        py_assign(py_retval(), py_arg(0));
        return true;
    }

    PY_CHECK_ARG_TYPE(0, tp_float);
    py_f64 x = py_tofloat(py_arg(0));
    py_f64 offset = x >= 0 ? 0.5 : -0.5;
    if(ndigits == -1) {
        py_newint(py_retval(), (py_i64)(x + offset));
        return true;
    }

    py_f64 factor = pow(10, ndigits);
    py_newfloat(py_retval(), (py_i64)(x * factor + offset) / factor);
    return true;
}

static bool builtins_print(int argc, py_Ref argv) {
    int length;
    py_TValue* args = pk_arrayview(argv, &length);
    assert(args != NULL);
    c11_sv sep = py_tosv(py_arg(1));
    c11_sv end = py_tosv(py_arg(2));
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    for(int i = 0; i < length; i++) {
        if(i > 0) c11_sbuf__write_sv(&buf, sep);
        if(!py_str(&args[i])) return false;
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
    }
    c11_sbuf__write_sv(&buf, end);
    c11_string* res = c11_sbuf__submit(&buf);
    pk_current_vm->print(res->data);
    c11_string__delete(res);
    py_newnone(py_retval());
    return true;
}

static bool builtins_isinstance(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(py_istuple(py_arg(1))) {
        int length = py_tuple_len(py_arg(1));
        for(int i = 0; i < length; i++) {
            py_Ref item = py_tuple_getitem(py_arg(1), i);
            if(!py_checktype(item, tp_type)) return false;
            if(py_isinstance(py_arg(0), py_totype(item))) {
                py_newbool(py_retval(), true);
                return true;
            }
        }
        py_newbool(py_retval(), false);
        return true;
    }

    if(!py_checktype(py_arg(1), tp_type)) return false;
    py_newbool(py_retval(), py_isinstance(py_arg(0), py_totype(py_arg(1))));
    return true;
}

static bool builtins_issubclass(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    if(!py_checktype(py_arg(0), tp_type)) return false;
    if(!py_checktype(py_arg(1), tp_type)) return false;
    py_newbool(py_retval(), py_issubclass(py_totype(py_arg(0)), py_totype(py_arg(1))));
    return true;
}

static bool builtins_getattr(int argc, py_Ref argv) {
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    if(argc == 2) {
        return py_getattr(py_arg(0), name);
    } else if(argc == 3) {
        bool ok = py_getattr(py_arg(0), name);
        if(!ok && py_matchexc(tp_AttributeError)) {
            py_clearexc(NULL);
            py_assign(py_retval(), py_arg(2));
            return true;  // default value
        }
        return ok;
    } else {
        return TypeError("getattr() expected 2 or 3 arguments");
    }
    return true;
}

static bool builtins_setattr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_newnone(py_retval());
    return py_setattr(py_arg(0), name, py_arg(2));
}

static bool builtins_hasattr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    bool ok = py_getattr(py_arg(0), name);
    if(ok) {
        py_newbool(py_retval(), true);
        return true;
    }
    if(py_matchexc(tp_AttributeError)) {
        py_clearexc(NULL);
        py_newbool(py_retval(), false);
        return true;
    }
    return false;
}

static bool builtins_delattr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    py_Name name = py_namev(py_tosv(py_arg(1)));
    py_newnone(py_retval());
    return py_delattr(py_arg(0), name);
}

static bool builtins_chr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_int);
    py_i64 val = py_toint(py_arg(0));
    if(val < 0 || val > 128) { return ValueError("chr() arg not in range(128)"); }
    py_newstrn(py_retval(), (const char*)&val, 1);
    return true;
}

static bool builtins_ord(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    c11_sv sv = py_tosv(py_arg(0));
    if(sv.size != 1) {
        return TypeError("ord() expected a character, but string of length %d found", sv.size);
    }
    py_newint(py_retval(), sv.data[0]);
    return true;
}

static bool builtins_globals(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_newglobals(py_retval());
    return true;
}

static bool builtins_locals(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_newlocals(py_retval());
    return true;
}

void py_newglobals(py_Ref out) {
    Frame* frame = pk_current_vm->top_frame;
    if(frame->is_dynamic) {
        py_assign(out, &frame->p0[0]);
    } else {
        pk_mappingproxy__namedict(out, frame->module);
    }
}

void py_newlocals(py_Ref out) {
    Frame* frame = pk_current_vm->top_frame;
    if(frame->is_dynamic) {
        py_assign(out, &frame->p0[1]);
        return;
    }
    if(frame->has_function) {
        pk_mappingproxy__locals(out, frame);
    } else {
        py_newglobals(out);
    }
}

static bool _builtins_execdyn(const char* title, int argc, py_Ref argv, enum py_CompileMode mode) {
    switch(argc) {
        case 1: {
            py_newglobals(py_pushtmp());
            py_newlocals(py_pushtmp());
            break;
        }
        case 2: {
            if(py_isnone(py_arg(1))) {
                py_newglobals(py_pushtmp());
            } else {
                py_push(py_arg(1));
            }
            py_pushnone();
            break;
        }
        case 3: {
            if(py_isnone(py_arg(1))) {
                py_newglobals(py_pushtmp());
            } else {
                py_push(py_arg(1));
            }
            py_push(py_arg(2));
            break;
        }
        default: return TypeError("%s() takes at most 3 arguments", title);
    }

    py_Ref code;
    if(py_isstr(argv)) {
        bool ok = py_compile(py_tostr(argv), "<string>", mode, true);
        if(!ok) return false;
        code = py_retval();
    } else if(py_istype(argv, tp_code)) {
        code = argv;
    } else {
        return TypeError("%s() expected 'str' or 'code', got '%t'", title, argv->type);
    }

    py_push(code);  // keep it alive

    // [globals, locals, code]
    CodeObject* co = py_touserdata(code);
    if(!co->src->is_dynamic) {
        if(argc != 1)
            return ValueError("code object is not dynamic, so globals and locals must be None");
        py_shrink(3);
    }
    Frame* frame = pk_current_vm->top_frame;
    return pk_exec(co, frame ? frame->module : NULL);
}

static bool builtins_exec(int argc, py_Ref argv) {
    bool ok = _builtins_execdyn("exec", argc, argv, EXEC_MODE);
    py_newnone(py_retval());
    return ok;
}

static bool builtins_eval(int argc, py_Ref argv) {
    return _builtins_execdyn("eval", argc, argv, EVAL_MODE);
}

static bool builtins_compile(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    for(int i = 0; i < 3; i++) {
        if(!py_checktype(py_arg(i), tp_str)) return false;
    }
    const char* source = py_tostr(py_arg(0));
    const char* filename = py_tostr(py_arg(1));
    const char* mode = py_tostr(py_arg(2));

    enum py_CompileMode compile_mode;
    if(strcmp(mode, "exec") == 0) {
        compile_mode = EXEC_MODE;
    } else if(strcmp(mode, "eval") == 0) {
        compile_mode = EVAL_MODE;
    } else if(strcmp(mode, "single") == 0) {
        compile_mode = SINGLE_MODE;
    } else {
        return ValueError("compile() mode must be 'exec', 'eval', or 'single'");
    }
    return py_compile(source, filename, compile_mode, true);
}

static bool NoneType__repr__(int argc, py_Ref argv) {
    py_newstr(py_retval(), "None");
    return true;
}

static bool ellipsis__repr__(int argc, py_Ref argv) {
    py_newstr(py_retval(), "Ellipsis");
    return true;
}

static bool NotImplementedType__repr__(int argc, py_Ref argv) {
    py_newstr(py_retval(), "NotImplemented");
    return true;
}

py_TValue pk_builtins__register() {
    py_Ref builtins = py_newmodule("builtins");
    py_bindfunc(builtins, "repr", builtins_repr);
    py_bindfunc(builtins, "exit", builtins_exit);
    py_bindfunc(builtins, "len", builtins_len);
    py_bindfunc(builtins, "hex", builtins_hex);
    py_bindfunc(builtins, "iter", builtins_iter);
    py_bindfunc(builtins, "next", builtins_next);
    py_bindfunc(builtins, "hash", builtins_hash);
    py_bindfunc(builtins, "abs", builtins_abs);
    py_bindfunc(builtins, "divmod", builtins_divmod);
    py_bindfunc(builtins, "round", builtins_round);

    py_bind(builtins, "print(*args, sep=' ', end='\\n')", builtins_print);

    py_bindfunc(builtins, "isinstance", builtins_isinstance);
    py_bindfunc(builtins, "issubclass", builtins_issubclass);

    py_bindfunc(builtins, "getattr", builtins_getattr);
    py_bindfunc(builtins, "setattr", builtins_setattr);
    py_bindfunc(builtins, "hasattr", builtins_hasattr);
    py_bindfunc(builtins, "delattr", builtins_delattr);

    py_bindfunc(builtins, "chr", builtins_chr);
    py_bindfunc(builtins, "ord", builtins_ord);

    py_bindfunc(builtins, "globals", builtins_globals);
    py_bindfunc(builtins, "locals", builtins_locals);
    py_bindfunc(builtins, "exec", builtins_exec);
    py_bindfunc(builtins, "eval", builtins_eval);
    py_bindfunc(builtins, "compile", builtins_compile);

    // some patches
    py_bindmagic(tp_NoneType, __repr__, NoneType__repr__);
    py_bindmagic(tp_ellipsis, __repr__, ellipsis__repr__);
    py_bindmagic(tp_NotImplementedType, __repr__, NotImplementedType__repr__);
    return *builtins;
}

static bool function__closure__getter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Function* ud = py_touserdata(argv);
    if(!ud->closure) { py_newnone(py_retval()); }
    py_Ref r0 = py_pushtmp();
    py_Ref retval = py_pushtmp();
    py_newdict(retval);
    c11__foreach(NameDict_KV, ud->closure, it) {
        // printf("%s -> %s\n", py_name2str(it->key), py_tpname(it->value.type));
        py_newstr(r0, py_name2str(it->key));
        py_dict_setitem(retval, r0, &it->value);
        if(py_checkexc()) {
            py_shrink(2);
            return false;
        }
    }
    py_assign(py_retval(), retval);
    py_shrink(2);
    return true;
}

static void function__gc_mark(void* ud) {
    Function* func = ud;
    if(func->closure) pk__mark_namedict(func->closure);
    CodeObject__gc_mark(&func->decl->code);
}

py_Type pk_function__register() {
    py_Type type =
        pk_newtype("function", tp_object, NULL, (void (*)(void*))Function__dtor, false, true);

    pk__tp_set_marker(type, function__gc_mark);

    py_bindproperty(type, "__closure__", function__closure__getter, NULL);
    return type;
}

static bool nativefunc__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newstr(py_retval(), "<nativefunc object>");
    return true;
}

py_Type pk_nativefunc__register() {
    py_Type type = pk_newtype("nativefunc", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __repr__, nativefunc__repr__);
    return type;
}

static bool super__new__(int argc, py_Ref argv) {
    py_Type* class_arg = py_newobject(py_retval(), tp_super, 1, sizeof(py_Type));
    Frame* frame = pk_current_vm->top_frame;
    *class_arg = 0;
    py_Ref self_arg = NULL;
    if(argc == 1) {
        // super()
        if(frame->has_function) {
            Function* func = py_touserdata(frame->p0);
            *class_arg = *(py_Type*)PyObject__userdata(func->clazz);
            if(frame->co->nlocals > 0) self_arg = &frame->locals[0];
        }
        if(class_arg == 0 || self_arg == NULL) return RuntimeError("super(): no arguments");
    } else if(argc == 3) {
        // super(type, obj)
        PY_CHECK_ARG_TYPE(1, tp_type);
        *class_arg = py_totype(py_arg(1));
        self_arg = py_arg(2);
        if(!py_isinstance(self_arg, *class_arg)) {
            return TypeError("super(type, obj): obj must be an instance of type");
        }
    } else {
        return TypeError("super() takes 0 or 2 arguments");
    }

    py_TypeInfo* types = pk_current_vm->types.data;
    *class_arg = types[*class_arg].base;
    if(*class_arg == 0) return RuntimeError("super(): base class is invalid");

    py_setslot(py_retval(), 0, self_arg);
    return true;
}

py_Type pk_super__register() {
    py_Type type = pk_newtype("super", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __new__, super__new__);
    return type;
}
