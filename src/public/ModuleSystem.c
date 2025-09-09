#include "pocketpy/common/str.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/_generated.h"


py_Ref py_getmodule(const char* path) {
    VM* vm = pk_current_vm;
    return BinTree__try_get(&vm->modules, (void*)path);
}

py_Ref py_newmodule(const char* path) {
    int path_len = strlen(path);
    if(path_len > PK_MAX_MODULE_PATH_LEN) c11__abort("module path too long: %s", path);
    if(path_len == 0) c11__abort("module path cannot be empty");

    py_ModuleInfo* mi = py_newobject(py_retval(), tp_module, -1, sizeof(py_ModuleInfo));

    int last_dot = c11_sv__rindex((c11_sv){path, path_len}, '.');
    if(last_dot == -1) {
        mi->name = c11_string__new(path);
        mi->package = c11_string__new("");
    } else {
        const char* start = path + last_dot + 1;
        mi->name = c11_string__new(start);
        mi->package = c11_string__new2(path, last_dot);
    }

    mi->path = c11_string__new(path);
    path = mi->path->data;

    // we do not allow override in order to avoid memory leak
    // it is because Module objects are not garbage collected
    bool exists = BinTree__contains(&pk_current_vm->modules, (void*)path);
    if(exists) c11__abort("module '%s' already exists", path);

    BinTree__set(&pk_current_vm->modules, (void*)path, py_retval());
    py_GlobalRef retval = py_getmodule(path);
    mi->self = retval;

    // setup __name__
    py_newstrv(py_emplacedict(retval, __name__), c11_string__sv(mi->name));
    // setup __package__
    py_newstrv(py_emplacedict(retval, __package__), c11_string__sv(mi->package));
    // setup __path__
    py_newstrv(py_emplacedict(retval, __path__), c11_string__sv(mi->path));
    return retval;
}

static void py_ModuleInfo__dtor(py_ModuleInfo* mi) {
    c11_string__delete(mi->name);
    c11_string__delete(mi->package);
    c11_string__delete(mi->path);
}

py_Type pk_module__register() {
    py_Type type = pk_newtype("module", tp_object, NULL, (py_Dtor)py_ModuleInfo__dtor, false, true);
    return type;
}

int load_module_from_dll_desktop_only(const char* path) PY_RAISE PY_RETURN;

int py_import(const char* path_cstr) {
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

        py_ModuleInfo* mi = py_touserdata(vm->top_frame->module);
        c11_sv package_sv = c11_string__sv(mi->path);
        if(package_sv.size == 0) {
            return ImportError("attempted relative import with no known parent package");
        }

        c11_vector /* T=c11_sv */ cpnts = c11_sv__split(package_sv, '.');
        for(int i = is_init; i < dot_count; i++) {
            if(cpnts.length == 0)
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
        for(int i = 0; i < cpnts.length; i++) {
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
    py_GlobalRef ext_mod = py_getmodule(path.data);
    if(ext_mod) {
        py_assign(py_retval(), ext_mod);
        return true;
    }

    if(vm->callbacks.lazyimport) {
        py_GlobalRef lazymod = vm->callbacks.lazyimport(path_cstr);
        if(lazymod) {
            c11__rtassert(py_istype(lazymod, tp_module));
            py_assign(py_retval(), lazymod);
            return 1;
        }
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

    data = vm->callbacks.importfile(filename->data);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    filename = c11_string__new3("%s%c__init__.py", slashed_path->data, PK_PLATFORM_SEP);
    data = vm->callbacks.importfile(filename->data);
    if(data != NULL) goto __SUCCESS;

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    // not found
    return load_module_from_dll_desktop_only(path_cstr);

__SUCCESS:
    do {
    } while(0);
    py_GlobalRef mod = py_newmodule(path_cstr);
    bool ok = py_exec((const char*)data, filename->data, EXEC_MODE, mod);
    py_assign(py_retval(), mod);

    c11_string__delete(filename);
    c11_string__delete(slashed_path);
    if(need_free) PK_FREE((void*)data);
    return ok ? 1 : -1;
}

bool py_importlib_reload(py_Ref module) {
    VM* vm = pk_current_vm;
    py_ModuleInfo* mi = py_touserdata(module);
    // We should ensure that the module is its original py_GlobalRef
    module = mi->self;
    c11_sv path = c11_string__sv(mi->path);
    c11_string* slashed_path = c11_sv__replace(path, '.', PK_PLATFORM_SEP);
    c11_string* filename = c11_string__new3("%s.py", slashed_path->data);
    char* data = vm->callbacks.importfile(filename->data);
    if(data == NULL) {
        c11_string__delete(filename);
        filename = c11_string__new3("%s%c__init__.py", slashed_path->data, PK_PLATFORM_SEP);
        data = vm->callbacks.importfile(filename->data);
    }
    c11_string__delete(slashed_path);
    if(data == NULL) return ImportError("module '%v' not found", path);
    // py_cleardict(module); BUG: removing old classes will cause RELOAD_MODE to fail
    bool ok = py_exec(data, filename->data, RELOAD_MODE, module);
    c11_string__delete(filename);
    PK_FREE(data);
    py_assign(py_retval(), module);
    return ok;
}
