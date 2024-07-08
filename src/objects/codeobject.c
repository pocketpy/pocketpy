#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/utils.h"
#include <stdint.h>

void Bytecode__set_signed_arg(Bytecode* self, int arg) {
    if(arg < INT16_MIN || arg > INT16_MAX) {
        PK_FATAL_ERROR("set_signed_arg: %d is out of range", arg);
    }
    self->arg = (int16_t)arg;
}

bool Bytecode__is_forward_jump(const Bytecode* self) {
    return self->op >= OP_JUMP_FORWARD && self->op <= OP_LOOP_BREAK;
}

static void FuncDecl__dtor(FuncDecl* self) {
    CodeObject__dtor(&self->code);
    c11_vector__dtor(&self->args);
    c11_vector__dtor(&self->kwargs);
    c11_smallmap_n2i__dtor(&self->kw_to_index);
}

FuncDecl_ FuncDecl__rcnew(pk_SourceData_ src, c11_sv name) {
    FuncDecl* self = malloc(sizeof(FuncDecl));
    self->rc.count = 1;
    self->rc.dtor = (void (*)(void*))FuncDecl__dtor;
    CodeObject__ctor(&self->code, src, name);

    c11_vector__ctor(&self->args, sizeof(int));
    c11_vector__ctor(&self->kwargs, sizeof(FuncDeclKwArg));

    self->starred_arg = -1;
    self->starred_kwarg = -1;
    self->nested = false;

    self->docstring = NULL;
    self->type = FuncType_UNSET;

    c11_smallmap_n2i__ctor(&self->kw_to_index);
    return self;
}

bool FuncDecl__is_duplicated_arg(const FuncDecl* decl, py_Name name) {
    py_Name tmp_name;
    c11__foreach(int, &decl->args, j) {
        tmp_name = c11__getitem(py_Name, &decl->args, *j);
        if(tmp_name == name) return true;
    }
    c11__foreach(FuncDeclKwArg, &decl->kwargs, kv) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, kv->index);
        if(tmp_name == name) return true;
    }
    if(decl->starred_arg != -1) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, decl->starred_arg);
        if(tmp_name == name) return true;
    }
    if(decl->starred_kwarg != -1) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, decl->starred_kwarg);
        if(tmp_name == name) return true;
    }
    return false;
}

void FuncDecl__add_arg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    c11_vector__push(int, &self->args, index);
}

void FuncDecl__add_kwarg(FuncDecl* self, py_Name name, const py_TValue* value) {
    int index = CodeObject__add_varname(&self->code, name);
    c11_smallmap_n2i__set(&self->kw_to_index, name, index);
    FuncDeclKwArg* item = c11_vector__emplace(&self->kwargs);
    item->index = index;
    item->key = name;
    item->value = *value;
}

void FuncDecl__add_starred_arg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    self->starred_arg = index;
}

void FuncDecl__add_starred_kwarg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    self->starred_kwarg = index;
}

FuncDecl_ FuncDecl__build(const char* name,
                          const char** args,
                          int argc,
                          const char* starred_arg,
                          const char** kwargs,
                          int kwargc,
                          py_Ref kwdefaults,  // a tuple contains default values
                          const char* starred_kwarg,
                          const char* docstring) {
    pk_SourceData_ source = pk_SourceData__rcnew("pass", "<bind>", EXEC_MODE, false);
    FuncDecl_ decl = FuncDecl__rcnew(source, (c11_sv){name, strlen(name)});
    for(int i = 0; i < argc; i++) {
        FuncDecl__add_arg(decl, py_name(args[i]));
    }
    if(starred_arg) { FuncDecl__add_starred_arg(decl, py_name(starred_arg)); }
    assert(py_istype(kwdefaults, tp_tuple));
    assert(py_tuple__len(kwdefaults) == kwargc);
    for(int i = 0; i < kwargc; i++) {
        FuncDecl__add_kwarg(decl, py_name(kwargs[i]), py_tuple__getitem(kwdefaults, i));
    }
    if(starred_kwarg) FuncDecl__add_starred_kwarg(decl, py_name(starred_kwarg));
    decl->docstring = docstring;
    PK_DECREF(source);
    return decl;
}

void CodeObject__ctor(CodeObject* self, pk_SourceData_ src, c11_sv name) {
    self->src = src;
    PK_INCREF(src);
    self->name = c11_string__new2(name.data, name.size);

    c11_vector__ctor(&self->codes, sizeof(Bytecode));
    c11_vector__ctor(&self->codes_ex, sizeof(BytecodeEx));

    c11_vector__ctor(&self->consts, sizeof(py_TValue));
    c11_vector__ctor(&self->varnames, sizeof(uint16_t));
    self->nlocals = 0;

    c11_smallmap_n2i__ctor(&self->varnames_inv);
    c11_smallmap_n2i__ctor(&self->labels);

    c11_vector__ctor(&self->blocks, sizeof(CodeBlock));
    c11_vector__ctor(&self->func_decls, sizeof(FuncDecl_));

    self->start_line = -1;
    self->end_line = -1;

    CodeBlock root_block = {CodeBlockType_NO_BLOCK, -1, 0, -1, -1};
    c11_vector__push(CodeBlock, &self->blocks, root_block);
}

void CodeObject__dtor(CodeObject* self) {
    PK_DECREF(self->src);
    c11_string__delete(self->name);

    c11_vector__dtor(&self->codes);
    c11_vector__dtor(&self->codes_ex);

    c11_vector__dtor(&self->consts);
    c11_vector__dtor(&self->varnames);

    c11_smallmap_n2i__dtor(&self->varnames_inv);
    c11_smallmap_n2i__dtor(&self->labels);

    c11_vector__dtor(&self->blocks);

    for(int i = 0; i < self->func_decls.count; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        PK_DECREF(decl);
    }
    c11_vector__dtor(&self->func_decls);
}

void Function__ctor(Function* self, FuncDecl_ decl, PyObject* module) {
    PK_INCREF(decl);
    self->decl = decl;
    self->module = module;
    self->clazz = NULL;
    self->closure = NULL;
    self->cfunc = NULL;
}

int CodeObject__add_varname(CodeObject* self, py_Name name) {
    int index = c11_smallmap_n2i__get(&self->varnames_inv, name, -1);
    if(index >= 0) return index;
    c11_vector__push(uint16_t, &self->varnames, name);
    self->nlocals++;
    index = self->varnames.count - 1;
    c11_smallmap_n2i__set(&self->varnames_inv, name, index);
    return index;
}

void Function__dtor(Function* self) {
    PK_DECREF(self->decl);
    if(self->closure) pk_NameDict__delete(self->closure);
}