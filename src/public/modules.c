#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"

py_Ref py_getmodule(const char* path) {
    pk_VM* vm = pk_current_vm;
    return pk_NameDict__try_get(&vm->modules, py_name(path));
}

py_Ref py_newmodule(const char* path) {
    pk_ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* obj = pk_ManagedHeap__new(heap, tp_module, -1, 0);

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
    bool exists = pk_NameDict__contains(&pk_current_vm->modules, path_name);
    if(exists) c11__abort("module '%s' already exists", path);
    pk_NameDict__set(&pk_current_vm->modules, path_name, *r0);

    py_shrink(2);
    return py_getmodule(path);
}

bool py_import(const char* path_cstr) {
    pk_VM* vm = pk_current_vm;
    c11_sv path = {path_cstr, strlen(path_cstr)};
    if(path.size == 0) return ValueError("empty module name");

    if(path.data[0] == '.') {
        // try relative import
        py_Ref package = py_getdict(&vm->top_frame->module, __package__);
        c11_sv package_sv = py_tosv(package);
        if(package_sv.size == 0) return ImportError("relative import %q with no known parent package", path);
        c11_string* new_path = c11_string__new3("%v.%v", package_sv, path);
        bool ok = py_import(new_path->data);
        c11_string__delete(new_path);
        return ok;
    }

    assert(path.data[0] != '.' && path.data[path.size - 1] != '.');

    // check existing module
    py_TmpRef ext_mod = py_getmodule(path.data);
    if(ext_mod) {
        py_Ref is_pending = py_getdict(ext_mod, __module_is_pending__);
        if(is_pending) return ImportError("circular import detected");
        py_assign(py_retval(), ext_mod);
        return true;
    }

    // vector<std::string_view> path_cpnts = path.split('.');
    // // check circular import
    // if(__import_context.pending.size() > 128) { ImportError("maximum recursion depth exceeded
    // while importing"); }

    // try import
    c11_string* slashed_path = c11_sv__replace(path, '.', PK_PLATFORM_SEP);

    c11_string* filename = c11_string__new3("site-packages/%s.py", slashed_path->data);
    int size;
    unsigned char* data = py_vfsread(filename->data, &size);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    filename = c11_string__new3("site-packages/%s/__init__.py", slashed_path->data);
    data = py_vfsread(filename->data, &size);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    filename = c11_string__new3("%s.py", slashed_path->data);
    data = vm->import_file(slashed_path->data, &size);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    filename = c11_string__new3("%s/__init__.py", slashed_path->data);
    data = vm->import_file(slashed_path->data, &size);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    return ImportError("module %q not found", path);

__SUCCESS:
    py_push(py_newmodule(path_cstr));
    py_Ref mod = py_peek(-1);
    py_setdict(mod, __module_is_pending__, py_True);
    bool ok = py_exec((const char*)data, filename->data, EXEC_MODE, mod);
    py_deldict(mod, __module_is_pending__);
    py_assign(py_retval(), mod);
    py_pop();

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    free(data);
    return ok;
}

//////////////////////////

static bool _py_builtins__repr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_repr(argv);
}

static bool _py_builtins__exit(int argc, py_Ref argv) {
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

static bool _py_builtins__len(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_len(argv);
}

static bool _py_builtins__reversed(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // convert _0 to list object
    if(!py_tpcall(tp_list, 1, argv)) return false;
    py_list__reverse(py_retval());
    return true;
}

static bool _py_builtins__hex(int argc, py_Ref argv) {
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

static bool _py_builtins__iter(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_iter(argv);
}

static bool _py_builtins__next(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int res = py_next(argv);
    if(res == -1) return false;
    if(res) return true;
    return py_exception("StopIteration", "");
}

static bool _py_builtins__sorted(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    // convert _0 to list object
    if(!py_tpcall(tp_list, 1, py_arg(0))) return false;
    py_push(py_retval());                      // duptop
    py_push(py_retval());                      // [| <list>]
    bool ok = py_pushmethod(py_name("sort"));  // [| list.sort, <list>]
    if(!ok) return false;
    py_push(py_arg(1));        // [| list.sort, <list>, key]
    py_push(py_arg(2));        // [| list.sort, <list>, key, reverse]
    ok = py_vectorcall(2, 0);  // [| ]
    if(!ok) return false;
    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

static bool _py_builtins__hash(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_i64 val;
    if(!py_hash(argv, &val)) return false;
    py_newint(py_retval(), val);
    return true;
}

static bool _py_builtins__abs(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return pk_callmagic(__abs__, 1, argv);
}

static bool _py_builtins__sum(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    int length;
    py_TValue* p = pk_arrayview(argv, &length);
    if(!p) return TypeError("sum() expects a list or tuple");

    py_i64 total_i64 = 0;
    py_f64 total_f64 = 0.0;
    bool is_float = false;
    for(int i = 0; i < length; i++) {
        switch(p[i].type) {
            case tp_int: total_i64 += p[i]._i64; break;
            case tp_float:
                is_float = true;
                total_f64 += p[i]._f64;
                break;
            default: return TypeError("sum() expects a list of numbers");
        }
    }

    if(is_float) {
        py_newfloat(py_retval(), total_f64 + total_i64);
    } else {
        py_newint(py_retval(), total_i64);
    }
    return true;
}

static bool _py_builtins__print(int argc, py_Ref argv) {
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

static bool _py_NoneType__repr__(int argc, py_Ref argv) {
    py_newstr(py_retval(), "None");
    return true;
}

static bool _py_builtins__exec(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    return py_exec(py_tostr(argv), "<exec>", EXEC_MODE, NULL);
}

static bool _py_builtins__eval(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    return py_exec(py_tostr(argv), "<eval>", EVAL_MODE, NULL);
}

py_TValue pk_builtins__register() {
    py_Ref builtins = py_newmodule("builtins");
    py_bindfunc(builtins, "repr", _py_builtins__repr);
    py_bindfunc(builtins, "exit", _py_builtins__exit);
    py_bindfunc(builtins, "len", _py_builtins__len);
    py_bindfunc(builtins, "reversed", _py_builtins__reversed);
    py_bindfunc(builtins, "hex", _py_builtins__hex);
    py_bindfunc(builtins, "iter", _py_builtins__iter);
    py_bindfunc(builtins, "next", _py_builtins__next);
    py_bindfunc(builtins, "hash", _py_builtins__hash);
    py_bindfunc(builtins, "abs", _py_builtins__abs);
    py_bindfunc(builtins, "sum", _py_builtins__sum);

    py_bindfunc(builtins, "exec", _py_builtins__exec);
    py_bindfunc(builtins, "eval", _py_builtins__eval);

    py_bind(builtins, "print(*args, sep=' ', end='\\n')", _py_builtins__print);
    py_bind(builtins, "sorted(iterable, key=None, reverse=False)", _py_builtins__sorted);

    // None __repr__
    py_bindmagic(tp_NoneType, __repr__, _py_NoneType__repr__);
    return *builtins;
}

py_Type pk_function__register() {
    py_Type type =
        pk_newtype("function", tp_object, NULL, (void (*)(void*))Function__dtor, false, true);
    return type;
}

static bool _py_nativefunc__repr(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newstr(py_retval(), "<nativefunc object>");
    return true;
}

py_Type pk_nativefunc__register() {
    py_Type type = pk_newtype("nativefunc", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __repr__, _py_nativefunc__repr);
    return type;
}